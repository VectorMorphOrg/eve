#pragma once

#include "eve/knowledge/knowledge_graph.hpp"
#include "eve/knowledge/knowledge_index.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace eve {

enum class SearchMode {
    Auto,
    Identifier,
    Title,
    Keyword,
    Tag,
    Repository,
    Series,
    Part,
    Status,
    DocumentType,
    Heading,
    Section,
    Relationship,
    FullText,
};

enum class SearchMatchType {
    ObjectId,
    DocumentIdentifier,
    Title,
    Keyword,
    Tag,
    Repository,
    Series,
    Part,
    Status,
    DocumentType,
    Heading,
    SectionTitle,
    RelationshipTarget,
    SearchTextToken,
};

[[nodiscard]] std::string to_string(SearchMode mode);
[[nodiscard]] std::string to_string(SearchMatchType match_type);

struct SearchQuery {
    std::string text;
    SearchMode mode{SearchMode::Auto};
    bool exact_match{true};
    bool partial_match{false};
    bool case_sensitive{false};
    std::size_t max_results{0};
    KnowledgeGraphEdgeType relationship_type{KnowledgeGraphEdgeType::References};
};

struct SearchEngineResult {
    std::string object_identifier;
    std::string document_identifier;
    std::string title;
    DocumentType document_type{DocumentType::Other};
    std::string repository;
    SearchMatchType match_type{SearchMatchType::ObjectId};
    std::string matched_field;
};

struct SearchEngineResultSet {
    SearchQuery query;
    SearchMode resolved_mode{SearchMode::Auto};
    std::size_t result_count{0};
    std::vector<SearchEngineResult> results;
};

class SearchEngine {
public:
    explicit SearchEngine(const KnowledgeIndex& index);

    [[nodiscard]] SearchEngineResultSet search(const SearchQuery& query) const;
    [[nodiscard]] const KnowledgeIndex& index() const noexcept;

private:
    const KnowledgeIndex* index_;
};

}  // namespace eve
