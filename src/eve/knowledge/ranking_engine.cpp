#include "eve/knowledge/ranking_engine.hpp"

#include <algorithm>
#include <cctype>
#include <chrono>

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

bool contains_token(std::string_view haystack, std::string_view token) {
    if (token.empty()) {
        return false;
    }
    return normalize_key(haystack).find(normalize_key(token)) != std::string::npos;
}

int match_type_base_score(SearchMatchType match_type) {
    switch (match_type) {
        case SearchMatchType::ObjectId:
            return 1000;
        case SearchMatchType::DocumentIdentifier:
            return 950;
        case SearchMatchType::Title:
            return 900;
        case SearchMatchType::Heading:
            return 850;
        case SearchMatchType::SectionTitle:
            return 800;
        case SearchMatchType::Keyword:
            return 750;
        case SearchMatchType::SearchTextToken:
            return 700;
        case SearchMatchType::RelationshipTarget:
            return 650;
        case SearchMatchType::Tag:
            return 600;
        case SearchMatchType::Repository:
        case SearchMatchType::Series:
        case SearchMatchType::Part:
        case SearchMatchType::Status:
        case SearchMatchType::DocumentType:
            return 500;
    }
    return 0;
}

int exact_match_bonus(
    const SearchQuery& query,
    const SearchEngineResult& result,
    const KnowledgeObject* object) {
    const auto normalized_query = normalize_key(query.text);
    if (normalized_query.empty()) {
        return 0;
    }

    if (result.match_type == SearchMatchType::ObjectId &&
        normalize_key(result.object_identifier) == normalized_query) {
        return 500;
    }
    if (result.match_type == SearchMatchType::DocumentIdentifier &&
        normalize_key(result.document_identifier) == normalized_query) {
        return 500;
    }
    if (result.match_type == SearchMatchType::Title &&
        normalize_key(result.title) == normalized_query) {
        return 400;
    }
    if (object != nullptr) {
        if (result.match_type == SearchMatchType::Heading) {
            for (const auto& heading : object->content.headings) {
                if (normalize_key(heading.text) == normalized_query) {
                    return 300;
                }
            }
        }
        if (result.match_type == SearchMatchType::SectionTitle) {
            for (const auto& section : object->content.sections) {
                if (normalize_key(section.title) == normalized_query) {
                    return 300;
                }
            }
        }
        if (result.match_type == SearchMatchType::Keyword) {
            for (const auto& keyword : object->search.keywords) {
                if (normalize_key(keyword) == normalized_query) {
                    return 250;
                }
            }
        }
    }
    return 0;
}

int specification_priority_score(const KnowledgeObject& object) {
    const auto priority = object.search.priority_rank > 0
        ? object.search.priority_rank
        : priority_rank_for_document_type(object.identity.document_type);
    return std::max(0, 200 - priority);
}

int freshness_score(const KnowledgeObject& object) {
    if (object.source.last_modified_time != TimePoint{}) {
        return static_cast<int>(
            std::chrono::duration_cast<std::chrono::seconds>(
                object.source.last_modified_time.time_since_epoch())
                .count() %
            1000000);
    }
    if (!object.source.last_modified.empty()) {
        return static_cast<int>(normalize_key(object.source.last_modified).size());
    }
    return 0;
}

int full_text_token_score(const SearchQuery& query, const KnowledgeObject& object) {
    int score = 0;
    for (const auto& token : tokenize(query.text)) {
        if (contains_token(object.search.search_text, token)) {
            score += 10;
        }
    }
    return score;
}

int relationship_score(SearchMatchType match_type) {
    if (match_type == SearchMatchType::RelationshipTarget) {
        return 100;
    }
    return 0;
}

int compute_rank_score(
    const SearchQuery& query,
    const SearchEngineResult& result,
    const KnowledgeObject* object) {
    int score = match_type_base_score(result.match_type);
    score += exact_match_bonus(query, result, object);
    score += relationship_score(result.match_type);

    if (object != nullptr) {
        score += specification_priority_score(*object);
        score += freshness_score(*object);
        score += full_text_token_score(query, *object);
    } else {
        score += std::max(0, 200 - priority_rank_for_document_type(result.document_type));
    }

    return score;
}

const KnowledgeObject* resolve_object(
    const KnowledgeIndex& index,
    const SearchEngineResult& result) {
    const auto by_id = index.lookup_by_object_id(result.object_identifier);
    if (!by_id.empty()) {
        return by_id.front();
    }
    const auto by_identifier = index.lookup_by_document_identifier(result.document_identifier);
    if (!by_identifier.empty()) {
        return by_identifier.front();
    }
    return nullptr;
}

}  // namespace

RankingEngine::RankingEngine(const KnowledgeIndex& index) : index_(&index) {}

const KnowledgeIndex& RankingEngine::index() const noexcept {
    return *index_;
}

RankedSearchResultSet RankingEngine::rank(const SearchEngineResultSet& candidates) const {
    RankedSearchResultSet ranked{
        .query = candidates.query,
        .resolved_mode = candidates.resolved_mode,
    };

    ranked.results.reserve(candidates.results.size());
    for (const auto& candidate : candidates.results) {
        const auto* object = resolve_object(*index_, candidate);
        ranked.results.push_back(RankedSearchResult{
            .candidate = candidate,
            .rank_score = compute_rank_score(candidates.query, candidate, object),
        });
    }

    std::stable_sort(
        ranked.results.begin(),
        ranked.results.end(),
        [](const RankedSearchResult& lhs, const RankedSearchResult& rhs) {
            if (lhs.rank_score != rhs.rank_score) {
                return lhs.rank_score > rhs.rank_score;
            }
            return lhs.candidate.object_identifier < rhs.candidate.object_identifier;
        });

    for (std::size_t index = 0; index < ranked.results.size(); ++index) {
        ranked.results[index].rank_position = index + 1U;
    }

    ranked.result_count = ranked.results.size();
    return ranked;
}

}  // namespace eve
