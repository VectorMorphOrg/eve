#include "eve/knowledge/search_engine.hpp"

#include "eve/knowledge/relationship_builder.hpp"

#include <algorithm>
#include <cctype>
#include <map>
#include <optional>
#include <set>

namespace eve {
namespace {

std::string normalize_key(std::string_view value) {
    std::string normalized{value};
    std::transform(normalized.begin(), normalized.end(), normalized.begin(), [](unsigned char ch) {
        return static_cast<char>(std::toupper(ch));
    });
    return normalized;
}

std::vector<std::string> tokenize(std::string_view text) {
    std::vector<std::string> tokens;
    std::string token;
    for (const unsigned char ch : text) {
        if (std::isalnum(ch) != 0) {
            token.push_back(static_cast<char>(ch));
            continue;
        }
        if (!token.empty()) {
            tokens.push_back(token);
            token.clear();
        }
    }
    if (!token.empty()) {
        tokens.push_back(token);
    }
    return tokens;
}

std::vector<std::string> lookup_keys_for_query(const SearchQuery& query) {
    std::vector<std::string> keys;
    if (!query.text.empty()) {
        keys.push_back(query.text);
    }
    if (query.partial_match) {
        for (const auto& token : tokenize(query.text)) {
            keys.push_back(token);
        }
    }
    return keys;
}

bool field_matches(
    std::string_view field_value,
    std::string_view query,
    bool exact_match,
    bool case_sensitive) {
    if (field_value.empty() || query.empty()) {
        return false;
    }
    if (case_sensitive) {
        if (exact_match) {
            return field_value == query;
        }
        return field_value.find(query) != std::string_view::npos;
    }

    const auto normalized_field = normalize_key(field_value);
    const auto normalized_query = normalize_key(query);
    if (exact_match) {
        return normalized_field == normalized_query;
    }
    return normalized_field.find(normalized_query) != std::string::npos;
}

SearchEngineResult make_result(
    const KnowledgeObject& object,
    SearchMatchType match_type,
    std::string matched_field) {
    return SearchEngineResult{
        .object_identifier = object.identity.id.value,
        .document_identifier = object.identity.document_identifier,
        .title = object.identity.title,
        .document_type = object.identity.document_type,
        .repository = object.identity.repository,
        .match_type = match_type,
        .matched_field = std::move(matched_field),
    };
}

bool passes_case_filter(
    const KnowledgeObject& object,
    SearchMatchType match_type,
    std::string_view query,
    bool exact_match,
    bool case_sensitive) {
    if (!case_sensitive) {
        return true;
    }

    switch (match_type) {
        case SearchMatchType::ObjectId:
            return field_matches(object.identity.id.value, query, exact_match, true);
        case SearchMatchType::DocumentIdentifier:
            return field_matches(object.identity.document_identifier, query, exact_match, true);
        case SearchMatchType::Title:
            return field_matches(object.identity.title, query, exact_match, true);
        case SearchMatchType::Keyword:
            return std::ranges::any_of(object.search.keywords, [&](const std::string& keyword) {
                return field_matches(keyword, query, exact_match, true);
            });
        case SearchMatchType::Tag:
            return std::ranges::any_of(object.metadata.tags, [&](const std::string& tag) {
                return field_matches(tag, query, exact_match, true);
            });
        case SearchMatchType::Repository:
            return field_matches(object.identity.repository, query, exact_match, true);
        case SearchMatchType::Series:
            return field_matches(object.metadata.series, query, exact_match, true);
        case SearchMatchType::Part:
            return field_matches(object.metadata.part, query, exact_match, true);
        case SearchMatchType::Status:
            return field_matches(object.metadata.status, query, exact_match, true);
        case SearchMatchType::DocumentType:
            return field_matches(to_string(object.identity.document_type), query, exact_match, true);
        case SearchMatchType::Heading:
            return std::ranges::any_of(object.content.headings, [&](const DocumentHeading& heading) {
                return field_matches(heading.text, query, exact_match, true);
            });
        case SearchMatchType::SectionTitle:
            return std::ranges::any_of(object.content.sections, [&](const KnowledgeSection& section) {
                return field_matches(section.title, query, exact_match, true);
            });
        case SearchMatchType::SearchTextToken:
            return field_matches(object.search.search_text, query, exact_match, true);
        case SearchMatchType::RelationshipTarget:
            return true;
    }
    return true;
}

struct ResultAccumulator {
    std::vector<SearchEngineResult> results;
    std::set<std::string> seen_object_ids;

    void add(const KnowledgeObject& object, SearchMatchType match_type, std::string matched_field) {
        if (seen_object_ids.contains(object.identity.id.value)) {
            return;
        }
        seen_object_ids.insert(object.identity.id.value);
        results.push_back(make_result(object, match_type, std::move(matched_field)));
    }

    void add_from_lookup(
        const std::vector<const KnowledgeObject*>& objects,
        SearchMatchType match_type,
        std::string_view query,
        std::string matched_field,
        bool exact_match,
        bool case_sensitive) {
        for (const auto* object : objects) {
            if (!passes_case_filter(*object, match_type, query, exact_match, case_sensitive)) {
                continue;
            }
            add(*object, match_type, matched_field);
        }
    }
};

std::optional<DocumentType> parse_document_type(std::string_view text) {
    const auto normalized = normalize_key(text);
    static const std::map<std::string, DocumentType> lookup{
        {"SPECIFICATION", DocumentType::Specification},
        {"REFERENCECATALOG", DocumentType::ReferenceCatalog},
        {"REFERENCE CATALOG", DocumentType::ReferenceCatalog},
        {"RFC", DocumentType::Rfc},
        {"WHITEPAPER", DocumentType::Whitepaper},
        {"README", DocumentType::Readme},
        {"GUIDE", DocumentType::Guide},
        {"CHANGELOG", DocumentType::Changelog},
        {"OTHER", DocumentType::Other},
    };

    if (const auto iterator = lookup.find(normalized); iterator != lookup.end()) {
        return iterator->second;
    }
    return std::nullopt;
}

void collect_identifier_lookups(
    const KnowledgeIndex& index,
    const SearchQuery& query,
    ResultAccumulator& accumulator) {
    const auto identifiers = RelationshipBuilder::discover_reference_identifiers(query.text);
    const std::vector<std::string> keys =
        identifiers.empty() ? lookup_keys_for_query(query) : identifiers;

    for (const auto& key : keys) {
        accumulator.add_from_lookup(
            index.lookup_by_object_id(key),
            SearchMatchType::ObjectId,
            key,
            "identity.id",
            query.exact_match,
            query.case_sensitive);
        accumulator.add_from_lookup(
            index.lookup_by_document_identifier(key),
            SearchMatchType::DocumentIdentifier,
            key,
            "identity.document_identifier",
            query.exact_match,
            query.case_sensitive);
    }
}

void collect_title_lookups(
    const KnowledgeIndex& index,
    const SearchQuery& query,
    ResultAccumulator& accumulator) {
    for (const auto& key : lookup_keys_for_query(query)) {
        accumulator.add_from_lookup(
            index.lookup_by_title(key),
            SearchMatchType::Title,
            key,
            "identity.title",
            query.exact_match,
            query.case_sensitive);
    }
}

void collect_keyword_lookups(
    const KnowledgeIndex& index,
    const SearchQuery& query,
    ResultAccumulator& accumulator) {
    for (const auto& key : lookup_keys_for_query(query)) {
        accumulator.add_from_lookup(
            index.lookup_by_keyword(key),
            SearchMatchType::Keyword,
            key,
            "search.keywords",
            query.exact_match,
            query.case_sensitive);
    }
}

void collect_tag_lookups(
    const KnowledgeIndex& index,
    const SearchQuery& query,
    ResultAccumulator& accumulator) {
    for (const auto& key : lookup_keys_for_query(query)) {
        accumulator.add_from_lookup(
            index.lookup_by_tag(key),
            SearchMatchType::Tag,
            key,
            "metadata.tags",
            query.exact_match,
            query.case_sensitive);
    }
}

void collect_repository_lookups(
    const KnowledgeIndex& index,
    const SearchQuery& query,
    ResultAccumulator& accumulator) {
    for (const auto& key : lookup_keys_for_query(query)) {
        accumulator.add_from_lookup(
            index.lookup_by_repository(key),
            SearchMatchType::Repository,
            key,
            "identity.repository",
            query.exact_match,
            query.case_sensitive);
    }
}

void collect_series_lookups(
    const KnowledgeIndex& index,
    const SearchQuery& query,
    ResultAccumulator& accumulator) {
    for (const auto& key : lookup_keys_for_query(query)) {
        accumulator.add_from_lookup(
            index.lookup_by_series(key),
            SearchMatchType::Series,
            key,
            "metadata.series",
            query.exact_match,
            query.case_sensitive);
    }
}

void collect_part_lookups(
    const KnowledgeIndex& index,
    const SearchQuery& query,
    ResultAccumulator& accumulator) {
    for (const auto& key : lookup_keys_for_query(query)) {
        accumulator.add_from_lookup(
            index.lookup_by_part(key),
            SearchMatchType::Part,
            key,
            "metadata.part",
            query.exact_match,
            query.case_sensitive);
    }
}

void collect_status_lookups(
    const KnowledgeIndex& index,
    const SearchQuery& query,
    ResultAccumulator& accumulator) {
    for (const auto& key : lookup_keys_for_query(query)) {
        accumulator.add_from_lookup(
            index.lookup_by_status(key),
            SearchMatchType::Status,
            key,
            "metadata.status",
            query.exact_match,
            query.case_sensitive);
    }
}

void collect_document_type_lookups(
    const KnowledgeIndex& index,
    const SearchQuery& query,
    ResultAccumulator& accumulator) {
    if (const auto document_type = parse_document_type(query.text)) {
        for (const auto* object : index.lookup_by_document_type(*document_type)) {
            if (!passes_case_filter(
                    *object,
                    SearchMatchType::DocumentType,
                    query.text,
                    query.exact_match,
                    query.case_sensitive)) {
                continue;
            }
            accumulator.add(
                *object,
                SearchMatchType::DocumentType,
                "identity.document_type");
        }
    }
}

void collect_heading_lookups(
    const KnowledgeIndex& index,
    const SearchQuery& query,
    ResultAccumulator& accumulator) {
    for (const auto& key : lookup_keys_for_query(query)) {
        accumulator.add_from_lookup(
            index.lookup_by_heading(key),
            SearchMatchType::Heading,
            key,
            "content.headings",
            query.exact_match,
            query.case_sensitive);
    }
}

void collect_section_lookups(
    const KnowledgeIndex& index,
    const SearchQuery& query,
    ResultAccumulator& accumulator) {
    for (const auto& key : lookup_keys_for_query(query)) {
        accumulator.add_from_lookup(
            index.lookup_by_section_title(key),
            SearchMatchType::SectionTitle,
            key,
            "content.sections.title",
            query.exact_match,
            query.case_sensitive);
    }
}

void collect_full_text_lookups(
    const KnowledgeIndex& index,
    const SearchQuery& query,
    ResultAccumulator& accumulator) {
    for (const auto& key : lookup_keys_for_query(query)) {
        accumulator.add_from_lookup(
            index.lookup_by_search_text_token(key),
            SearchMatchType::SearchTextToken,
            key,
            "search.search_text",
            query.exact_match,
            query.case_sensitive);
    }
}

void collect_relationship_lookups(
    const KnowledgeIndex& index,
    const SearchQuery& query,
    ResultAccumulator& accumulator) {
    for (const auto& key : lookup_keys_for_query(query)) {
        const auto normalized = RelationshipBuilder::normalize_reference_identifier(key);
        const auto lookup_key = normalized.empty() ? key : normalized;
        accumulator.add_from_lookup(
            index.lookup_by_relationship(query.relationship_type, lookup_key),
            SearchMatchType::RelationshipTarget,
            lookup_key,
            "relationships",
            query.exact_match,
            query.case_sensitive);
    }
}

void collect_auto_lookups(
    const KnowledgeIndex& index,
    const SearchQuery& query,
    ResultAccumulator& accumulator) {
    if (!RelationshipBuilder::discover_reference_identifiers(query.text).empty()) {
        collect_identifier_lookups(index, query, accumulator);
    }

    collect_title_lookups(index, query, accumulator);
    collect_keyword_lookups(index, query, accumulator);
    collect_tag_lookups(index, query, accumulator);
    collect_repository_lookups(index, query, accumulator);
    collect_series_lookups(index, query, accumulator);
    collect_part_lookups(index, query, accumulator);
    collect_status_lookups(index, query, accumulator);
    collect_document_type_lookups(index, query, accumulator);
    collect_heading_lookups(index, query, accumulator);
    collect_section_lookups(index, query, accumulator);
    collect_full_text_lookups(index, query, accumulator);

    SearchQuery relationship_query = query;
    relationship_query.relationship_type = KnowledgeGraphEdgeType::References;
    collect_relationship_lookups(index, relationship_query, accumulator);
}

void apply_max_results(SearchEngineResultSet& result_set, std::size_t max_results) {
    if (max_results == 0 || result_set.results.size() <= max_results) {
        result_set.result_count = result_set.results.size();
        return;
    }
    result_set.results.resize(max_results);
    result_set.result_count = result_set.results.size();
}

void dispatch_mode(
    const KnowledgeIndex& index,
    SearchMode mode,
    const SearchQuery& query,
    ResultAccumulator& accumulator) {
    switch (mode) {
        case SearchMode::Auto:
            collect_auto_lookups(index, query, accumulator);
            break;
        case SearchMode::Identifier:
            collect_identifier_lookups(index, query, accumulator);
            break;
        case SearchMode::Title:
            collect_title_lookups(index, query, accumulator);
            break;
        case SearchMode::Keyword:
            collect_keyword_lookups(index, query, accumulator);
            break;
        case SearchMode::Tag:
            collect_tag_lookups(index, query, accumulator);
            break;
        case SearchMode::Repository:
            collect_repository_lookups(index, query, accumulator);
            break;
        case SearchMode::Series:
            collect_series_lookups(index, query, accumulator);
            break;
        case SearchMode::Part:
            collect_part_lookups(index, query, accumulator);
            break;
        case SearchMode::Status:
            collect_status_lookups(index, query, accumulator);
            break;
        case SearchMode::DocumentType:
            collect_document_type_lookups(index, query, accumulator);
            break;
        case SearchMode::Heading:
            collect_heading_lookups(index, query, accumulator);
            break;
        case SearchMode::Section:
            collect_section_lookups(index, query, accumulator);
            break;
        case SearchMode::Relationship:
            collect_relationship_lookups(index, query, accumulator);
            break;
        case SearchMode::FullText:
            collect_full_text_lookups(index, query, accumulator);
            break;
    }
}

}  // namespace

std::string to_string(SearchMode mode) {
    switch (mode) {
        case SearchMode::Auto:
            return "Auto";
        case SearchMode::Identifier:
            return "Identifier";
        case SearchMode::Title:
            return "Title";
        case SearchMode::Keyword:
            return "Keyword";
        case SearchMode::Tag:
            return "Tag";
        case SearchMode::Repository:
            return "Repository";
        case SearchMode::Series:
            return "Series";
        case SearchMode::Part:
            return "Part";
        case SearchMode::Status:
            return "Status";
        case SearchMode::DocumentType:
            return "DocumentType";
        case SearchMode::Heading:
            return "Heading";
        case SearchMode::Section:
            return "Section";
        case SearchMode::Relationship:
            return "Relationship";
        case SearchMode::FullText:
            return "FullText";
    }
    return "Unknown";
}

std::string to_string(SearchMatchType match_type) {
    switch (match_type) {
        case SearchMatchType::ObjectId:
            return "ObjectId";
        case SearchMatchType::DocumentIdentifier:
            return "DocumentIdentifier";
        case SearchMatchType::Title:
            return "Title";
        case SearchMatchType::Keyword:
            return "Keyword";
        case SearchMatchType::Tag:
            return "Tag";
        case SearchMatchType::Repository:
            return "Repository";
        case SearchMatchType::Series:
            return "Series";
        case SearchMatchType::Part:
            return "Part";
        case SearchMatchType::Status:
            return "Status";
        case SearchMatchType::DocumentType:
            return "DocumentType";
        case SearchMatchType::Heading:
            return "Heading";
        case SearchMatchType::SectionTitle:
            return "SectionTitle";
        case SearchMatchType::RelationshipTarget:
            return "RelationshipTarget";
        case SearchMatchType::SearchTextToken:
            return "SearchTextToken";
    }
    return "Unknown";
}

SearchEngine::SearchEngine(const KnowledgeIndex& index) : index_(&index) {}

const KnowledgeIndex& SearchEngine::index() const noexcept {
    return *index_;
}

SearchEngineResultSet SearchEngine::search(const SearchQuery& query) const {
    SearchEngineResultSet result_set{
        .query = query,
        .resolved_mode = query.mode,
    };

    if (query.text.empty()) {
        result_set.result_count = 0;
        return result_set;
    }

    ResultAccumulator accumulator;
    dispatch_mode(*index_, query.mode, query, accumulator);

    result_set.results = std::move(accumulator.results);
    apply_max_results(result_set, query.max_results);
    return result_set;
}

}  // namespace eve
