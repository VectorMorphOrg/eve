#pragma once

#include "eve/context/context_package.hpp"
#include "eve/knowledge/citation_engine.hpp"
#include "eve/knowledge/knowledge_graph.hpp"
#include "eve/knowledge/knowledge_index.hpp"
#include "eve/knowledge/ranking_engine.hpp"
#include "eve/knowledge/search_engine.hpp"

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace eve::context {

enum class ContextSelectionPolicy {
    Full,
    Balanced,
    Minimal,
};

[[nodiscard]] std::string to_string(ContextSelectionPolicy policy);

struct ContextBudget {
    std::size_t max_documents{10};
    std::size_t max_sections{50};
    std::size_t max_headings{50};
    std::size_t max_implementation_files{20};
    std::size_t max_test_files{20};
    std::size_t max_references{50};
    std::size_t max_estimated_tokens{32000};
};

struct ContextBudgetUtilization {
    std::size_t selected_documents{0};
    std::size_t selected_sections{0};
    std::size_t selected_headings{0};
    std::size_t selected_implementation_files{0};
    std::size_t selected_test_files{0};
    std::size_t selected_references{0};
    std::size_t estimated_tokens{0};
    bool budget_truncated{false};
};

struct ContextAssemblyIdentity {
    RequestId request_id;
    CorrelationId correlation_id;
};

struct SelectedSection {
    std::string object_identifier;
    std::string title;
    std::string content;
};

struct SelectedHeading {
    std::string object_identifier;
    std::string text;
};

struct AssembledDocumentMetadata {
    std::string object_identifier;
    std::string repository;
    std::string series;
    std::string part;
    std::string status;
    DocumentType document_type{DocumentType::Other};
};

struct NavigationContextEntry {
    std::string object_identifier;
    std::optional<std::string> previous;
    std::optional<std::string> next;
    std::optional<std::string> parent;
    std::vector<std::string> children;
};

struct ContextAssemblyEvidence {
    ContextAssemblyIdentity identity;
    std::vector<RankedSearchResult> ranked_results;
    std::vector<CitationBundle> citation_bundles;
    std::vector<AssembledDocumentMetadata> document_metadata;
    std::vector<SelectedSection> sections;
    std::vector<SelectedHeading> headings;
    std::vector<std::string> implementation_headers;
    std::vector<std::string> implementation_sources;
    std::vector<std::string> validation_tests;
    std::vector<NavigationContextEntry> navigation;
    std::vector<std::string> related_references;
    ContextBudgetUtilization budget;
};

struct ContextAssemblyRequest {
    PlatformRequest platform_request;
    SearchQuery search_query;
    RankedSearchResultSet ranked_results;
    CitationResultSet citations;
    ContextBudget budget;
    ContextSelectionPolicy policy{ContextSelectionPolicy::Balanced};
    ContextConstraints constraints{
        .documentation_first = true,
        .preserve_citations = true,
        .do_not_invent_information = true,
    };
    SystemInstructions system;
};

struct ContextAssemblyResult {
    ContextPackage package;
    ContextAssemblyEvidence evidence;
    ContextBudget budget;
    ContextSelectionPolicy policy{ContextSelectionPolicy::Balanced};
};

class ContextAssembler {
public:
    ContextAssembler(const KnowledgeIndex& index, const KnowledgeGraph& graph);

    [[nodiscard]] ContextAssemblyResult assemble(const ContextAssemblyRequest& request) const;
    [[nodiscard]] const KnowledgeIndex& index() const noexcept;
    [[nodiscard]] const KnowledgeGraph& graph() const noexcept;

private:
    const KnowledgeIndex* index_;
    const KnowledgeGraph* graph_;
};

[[nodiscard]] ContextBudget effective_budget_for_policy(
    ContextBudget budget,
    ContextSelectionPolicy policy);

[[nodiscard]] std::size_t estimate_tokens(std::string_view text);

}  // namespace eve::context
