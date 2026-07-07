#pragma once

#include "eve/knowledge/knowledge_graph.hpp"
#include "eve/knowledge/knowledge_index.hpp"
#include "eve/knowledge/ranking_engine.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace eve {

enum class CitationType {
    PrimarySpecification,
    ImplementationHeader,
    ImplementationSource,
    ValidationTest,
    RelatedDocument,
    SupportingReference,
    Navigation,
};

[[nodiscard]] std::string to_string(CitationType type);

struct Citation {
    std::string object_identifier;
    std::string document_identifier;
    std::string title;
    DocumentType document_type{DocumentType::Other};
    std::string repository;
    std::vector<std::string> supporting_section_titles;
    std::vector<std::string> supporting_headings;
    std::vector<std::string> supporting_source_files;
    std::vector<std::string> supporting_test_files;
    std::vector<std::string> related_references;
    CitationType citation_type{CitationType::PrimarySpecification};
    int citation_confidence{0};
};

struct CitationBundle {
    RankedSearchResult ranked_result;
    Citation primary;
    std::vector<Citation> implementation;
    std::vector<Citation> validation;
    std::vector<Citation> related_documents;
    std::vector<Citation> supporting_references;
    std::vector<Citation> navigation;
};

struct CitationResultSet {
    SearchQuery query;
    SearchMode resolved_mode{SearchMode::Auto};
    std::size_t bundle_count{0};
    std::vector<CitationBundle> bundles;
};

class CitationEngine {
public:
    CitationEngine(const KnowledgeIndex& index, const KnowledgeGraph& graph);

    [[nodiscard]] CitationResultSet cite(const RankedSearchResultSet& ranked_results) const;
    [[nodiscard]] const KnowledgeIndex& index() const noexcept;
    [[nodiscard]] const KnowledgeGraph& graph() const noexcept;

private:
    const KnowledgeIndex* index_;
    const KnowledgeGraph* graph_;
};

}  // namespace eve
