#pragma once

#include "eve/context/context_assembler.hpp"
#include "eve/knowledge/citation_engine.hpp"
#include "eve/knowledge/knowledge_index.hpp"
#include "eve/knowledge/ranking_engine.hpp"
#include "eve/knowledge/search_engine.hpp"

#include <memory>
#include <string_view>
#include <vector>

namespace eve {

/// Integration wrapper that owns immutable reasoning pipeline components built once
/// from indexed knowledge objects. Does not implement retrieval logic itself.
class ReasoningPipeline {
public:
    explicit ReasoningPipeline(std::vector<KnowledgeObject> objects);

    ReasoningPipeline(const ReasoningPipeline&) = delete;
    ReasoningPipeline& operator=(const ReasoningPipeline&) = delete;
    ReasoningPipeline(ReasoningPipeline&&) = delete;
    ReasoningPipeline& operator=(ReasoningPipeline&&) = delete;

    [[nodiscard]] static std::shared_ptr<ReasoningPipeline> build_shared(
        std::vector<KnowledgeObject> objects);

    [[nodiscard]] SearchEngineResultSet search(const SearchQuery& query) const;
    [[nodiscard]] RankedSearchResultSet rank(const SearchEngineResultSet& candidates) const;
    [[nodiscard]] CitationResultSet cite(const RankedSearchResultSet& ranked_results) const;
    [[nodiscard]] context::ContextAssemblyResult assemble(
        const context::ContextAssemblyRequest& request) const;

    [[nodiscard]] const KnowledgeIndex& index() const noexcept;
    [[nodiscard]] const KnowledgeGraph& graph() const noexcept;
    [[nodiscard]] const SearchEngine& search_engine() const noexcept;
    [[nodiscard]] const RankingEngine& ranking_engine() const noexcept;
    [[nodiscard]] const CitationEngine& citation_engine() const noexcept;
    [[nodiscard]] const context::ContextAssembler& context_assembler() const noexcept;
    [[nodiscard]] const std::vector<KnowledgeObject>& objects() const noexcept;

private:
    KnowledgeIndex index_;
    SearchEngine search_engine_;
    RankingEngine ranking_engine_;
    CitationEngine citation_engine_;
    context::ContextAssembler context_assembler_;
};

[[nodiscard]] SearchEngineResultSet filter_search_results_by_repository(
    SearchEngineResultSet results,
    std::string_view repository);

[[nodiscard]] std::vector<SearchResult> to_legacy_search_results(
    const SearchEngineResultSet& results,
    const RankedSearchResultSet& ranked_results,
    const KnowledgeIndex& index);

}  // namespace eve
