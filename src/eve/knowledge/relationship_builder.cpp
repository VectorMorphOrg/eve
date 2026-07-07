#include "eve/knowledge/relationship_builder.hpp"

#include <algorithm>
#include <cctype>
#include <map>
#include <regex>
#include <set>
#include <sstream>

namespace eve {
namespace {

std::string to_upper(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::toupper(ch));
    });
    return value;
}

std::string trim(std::string value) {
    const auto not_space = [](unsigned char ch) { return !std::isspace(ch); };
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), not_space));
    value.erase(std::find_if(value.rbegin(), value.rend(), not_space).base(), value.end());
    return value;
}

void append_unique(std::vector<std::string>& values, std::string value) {
    value = trim(std::move(value));
    if (value.empty()) {
        return;
    }
    if (std::ranges::find(values, value) == values.end()) {
        values.push_back(std::move(value));
    }
}

void append_unique_sorted(std::vector<std::string>& values) {
    std::sort(values.begin(), values.end());
    values.erase(std::unique(values.begin(), values.end()), values.end());
}

std::string object_key(const KnowledgeObject& object) {
    if (!object.identity.document_identifier.empty()) {
        return to_upper(object.identity.document_identifier);
    }
    return to_upper(object.identity.id.value);
}

std::string canonical_object_id(const KnowledgeObject& object) {
    return object.identity.id.value;
}

std::optional<std::string> parent_directory(const std::filesystem::path& path) {
    if (path.empty()) {
        return std::nullopt;
    }
    const auto parent = path.parent_path();
    if (parent.empty() || parent == path) {
        return std::nullopt;
    }
    return parent.generic_string();
}

std::string path_string(const std::filesystem::path& path) {
    if (path.empty()) {
        return {};
    }
    return path.generic_string();
}

std::string derive_implementation_token(const KnowledgeObject& object) {
    const auto filename = object.source.file_path.stem().string();
    static const std::regex eve_prefix(R"(^eve-\d{4}-(.+)$)", std::regex::icase);
    std::smatch match;
    if (std::regex_match(filename, match, eve_prefix)) {
        auto token = match[1].str();
        static const std::regex suffix(R"(-specification$)", std::regex::icase);
        token = std::regex_replace(token, suffix, "");
        return token;
    }
    return filename;
}

bool path_contains_token(const std::filesystem::path& path, std::string_view token) {
    if (token.empty()) {
        return false;
    }
    auto lower_path = path.generic_string();
    std::string lower_token{token};
    std::transform(lower_path.begin(), lower_path.end(), lower_path.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    std::transform(lower_token.begin(), lower_token.end(), lower_token.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return lower_path.find(lower_token) != std::string::npos;
}

bool is_readme_object(const KnowledgeObject& object) {
    return object.identity.document_type == DocumentType::Readme ||
           object.source.file_path.filename() == "README.md";
}
bool is_documentation_path(const std::filesystem::path& path) {
    const auto generic = path.generic_string();
    return generic.starts_with("docs/") || generic.find("/docs/") != std::string::npos ||
           generic.ends_with(".md");
}

bool is_include_path(std::string_view generic) {
    return generic.starts_with("include/") || generic.find("/include/") != std::string_view::npos;
}

bool is_source_path(std::string_view generic) {
    return generic.starts_with("src/") || generic.find("/src/") != std::string_view::npos;
}

bool is_test_path(std::string_view generic) {
    return generic.starts_with("tests/") || generic.find("/tests/") != std::string_view::npos;
}

void collect_text_sources(const KnowledgeObject& object, std::vector<std::string>& sources) {
    sources.push_back(object.content.raw_markdown);
    sources.push_back(object.search.search_text);
    for (const auto& section : object.content.sections) {
        sources.push_back(section.title);
        sources.push_back(section.content);
        for (const auto& reference : section.references) {
            sources.push_back(reference);
        }
    }
    for (const auto& paragraph : object.content.paragraphs) {
        sources.push_back(paragraph.text);
    }
    for (const auto& link : object.content.links) {
        sources.push_back(link.text);
        sources.push_back(link.destination);
    }
    if (object.relationships.previous) {
        sources.push_back(*object.relationships.previous);
    }
    if (object.relationships.next) {
        sources.push_back(*object.relationships.next);
    }
}

std::optional<std::string> resolve_navigation_target(
    std::string_view value,
    const std::map<std::string, std::size_t>& identifier_index,
    const std::vector<KnowledgeObject>& objects) {
    for (const auto& identifier : RelationshipBuilder::discover_reference_identifiers(value)) {
        const auto normalized = RelationshipBuilder::normalize_reference_identifier(identifier);
        if (identifier_index.contains(normalized)) {
            return canonical_object_id(objects.at(identifier_index.at(normalized)));
        }
    }

    const auto upper = to_upper(trim(std::string{value}));
    if (identifier_index.contains(upper)) {
        return canonical_object_id(objects.at(identifier_index.at(upper)));
    }
    return std::nullopt;
}

struct CorpusIndex {
    std::map<std::string, std::size_t> identifier_index;
    std::map<std::string, std::vector<std::size_t>> by_series;
    std::map<std::string, std::vector<std::size_t>> by_part;
    std::map<std::string, std::vector<std::size_t>> by_repository;
    std::map<std::string, std::vector<std::size_t>> by_directory;
    std::map<std::string, std::size_t> readme_by_directory;
    std::vector<std::string> known_paths;
};

CorpusIndex build_corpus_index(const std::vector<KnowledgeObject>& objects) {
    CorpusIndex index;
    for (std::size_t object_index = 0; object_index < objects.size(); ++object_index) {
        const auto& object = objects[object_index];
        index.identifier_index[object_key(object)] = object_index;

        if (!object.metadata.series.empty()) {
            index.by_series[object.metadata.series].push_back(object_index);
        }
        if (!object.metadata.part.empty()) {
            index.by_part[object.metadata.part].push_back(object_index);
        }
        if (!object.identity.repository.empty()) {
            index.by_repository[object.identity.repository].push_back(object_index);
        }
        if (const auto directory = parent_directory(object.source.file_path)) {
            index.by_directory[*directory].push_back(object_index);
            if (is_readme_object(object)) {
                index.readme_by_directory[*directory] = object_index;
            }
        }
        if (!object.source.file_path.empty()) {
            index.known_paths.push_back(path_string(object.source.file_path));
        }
    }

    for (auto& [_, members] : index.by_series) {
        std::sort(members.begin(), members.end());
    }
    for (auto& [_, members] : index.by_part) {
        std::sort(members.begin(), members.end());
    }
    for (auto& [_, members] : index.by_repository) {
        std::sort(members.begin(), members.end());
    }
    return index;
}

void discover_reference_relationships(
    KnowledgeObject& object,
    const std::string& self_key) {
    std::vector<std::string> sources;
    collect_text_sources(object, sources);

    for (const auto& source : sources) {
        for (const auto& identifier : RelationshipBuilder::discover_reference_identifiers(source)) {
            const auto normalized = RelationshipBuilder::normalize_reference_identifier(identifier);
            if (normalized == self_key) {
                continue;
            }
            append_unique(object.relationships.references, normalized);
        }
    }
    append_unique_sorted(object.relationships.references);
}

void discover_navigation_relationships(
    KnowledgeObject& object,
    std::size_t object_index,
    const CorpusIndex& index,
    const std::vector<KnowledgeObject>& objects) {
    if (object.relationships.previous) {
        if (const auto resolved = resolve_navigation_target(
                *object.relationships.previous,
                index.identifier_index,
                objects)) {
            object.relationships.previous = resolved;
        }
    }
    if (object.relationships.next) {
        if (const auto resolved = resolve_navigation_target(
                *object.relationships.next,
                index.identifier_index,
                objects)) {
            object.relationships.next = resolved;
        }
    }

    if (const auto directory = parent_directory(object.source.file_path)) {
        if (const auto readme = index.readme_by_directory.find(*directory);
            readme != index.readme_by_directory.end() && readme->second != object_index) {
            object.relationships.parent = canonical_object_id(objects.at(readme->second));
        }
    }
}

void discover_repository_relationships(
    KnowledgeObject& object,
    std::size_t object_index,
    const CorpusIndex& index,
    const std::vector<KnowledgeObject>& objects) {
    const auto self_id = canonical_object_id(object);

    if (!object.metadata.series.empty()) {
        for (const auto peer_index : index.by_series.at(object.metadata.series)) {
            if (peer_index == object_index) {
                continue;
            }
            append_unique(object.relationships.same_series, canonical_object_id(objects.at(peer_index)));
        }
    }

    if (!object.metadata.part.empty()) {
        for (const auto peer_index : index.by_part.at(object.metadata.part)) {
            if (peer_index == object_index) {
                continue;
            }
            append_unique(object.relationships.same_part, canonical_object_id(objects.at(peer_index)));
        }
    }

    if (!object.identity.repository.empty()) {
        for (const auto peer_index : index.by_repository.at(object.identity.repository)) {
            if (peer_index == object_index) {
                continue;
            }
            append_unique(
                object.relationships.same_repository,
                canonical_object_id(objects.at(peer_index)));
        }
    }

    append_unique_sorted(object.relationships.same_series);
    append_unique_sorted(object.relationships.same_part);
    append_unique_sorted(object.relationships.same_repository);
    (void)self_id;
}

void discover_implementation_relationships(
    KnowledgeObject& object,
    const CorpusIndex& index) {
    if (!is_documentation_path(object.source.file_path)) {
        return;
    }

    const auto token = derive_implementation_token(object);
    if (token.empty()) {
        return;
    }

    for (const auto& known_path : index.known_paths) {
        const std::filesystem::path path{known_path};
        const auto generic = path.generic_string();
        if (generic == path_string(object.source.file_path)) {
            continue;
        }

        if (is_include_path(generic) && path_contains_token(path, token)) {
            append_unique(object.relationships.related_headers, generic);
        } else if (is_source_path(generic) && path_contains_token(path, token)) {
            append_unique(object.relationships.related_sources, generic);
        } else if (is_test_path(generic) && path_contains_token(path, token)) {
            append_unique(object.relationships.related_tests, generic);
        }
    }

    append_unique_sorted(object.relationships.related_headers);
    append_unique_sorted(object.relationships.related_sources);
    append_unique_sorted(object.relationships.related_tests);
}

void build_reciprocal_references(std::vector<KnowledgeObject>& objects) {
    std::map<std::string, std::set<std::string>> referenced_by_map;

    for (const auto& object : objects) {
        const auto source_id = canonical_object_id(object);
        for (const auto& reference : object.relationships.references) {
            referenced_by_map[reference].insert(source_id);
        }
    }

    for (auto& object : objects) {
        const auto key = object_key(object);
        if (const auto iterator = referenced_by_map.find(key); iterator != referenced_by_map.end()) {
            for (const auto& referencer : iterator->second) {
                append_unique(object.relationships.referenced_by, referencer);
            }
        }
        append_unique_sorted(object.relationships.referenced_by);
    }
}

void build_parent_children(std::vector<KnowledgeObject>& objects) {
    std::map<std::string, std::size_t> id_index;
    for (std::size_t index = 0; index < objects.size(); ++index) {
        id_index[canonical_object_id(objects[index])] = index;
    }

    for (const auto& object : objects) {
        if (!object.relationships.parent) {
            continue;
        }
        const auto parent_id = *object.relationships.parent;
        if (const auto iterator = id_index.find(parent_id); iterator != id_index.end()) {
            append_unique(
                objects[iterator->second].relationships.children,
                canonical_object_id(object));
        }
    }

    for (auto& object : objects) {
        append_unique_sorted(object.relationships.children);
    }
}

}  // namespace

std::vector<std::string> RelationshipBuilder::discover_reference_identifiers(std::string_view text) {
    static const std::regex pattern(
        R"((?:EVE|CAP|SVC|DOC)-\d{4}(?:-[A-Z0-9-]+)?)",
        std::regex::icase);
    std::vector<std::string> identifiers;
    std::string subject{text};
    std::smatch match;
    while (std::regex_search(subject, match, pattern)) {
        append_unique(identifiers, normalize_reference_identifier(match[0].str()));
        subject = match.suffix();
    }
    append_unique_sorted(identifiers);
    return identifiers;
}

std::string RelationshipBuilder::normalize_reference_identifier(std::string reference) {
    reference = trim(std::move(reference));
    static const std::regex pattern(
        R"((?:EVE|CAP|SVC|DOC)-\d{4}(?:-[A-Z0-9-]+)?)",
        std::regex::icase);
    std::smatch match;
    if (std::regex_search(reference, match, pattern)) {
        return to_upper(match[0].str());
    }
    return to_upper(reference);
}

std::vector<KnowledgeObject> RelationshipBuilder::build(std::vector<KnowledgeObject> objects) {
    const auto index = build_corpus_index(objects);

    for (std::size_t object_index = 0; object_index < objects.size(); ++object_index) {
        auto& object = objects[object_index];
        const auto self_key = object_key(object);

        discover_reference_relationships(object, self_key);
        discover_navigation_relationships(object, object_index, index, objects);
        discover_repository_relationships(object, object_index, index, objects);
        discover_implementation_relationships(object, index);
    }

    build_reciprocal_references(objects);
    build_parent_children(objects);
    return objects;
}

}  // namespace eve
