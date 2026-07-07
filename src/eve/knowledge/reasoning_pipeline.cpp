#include "eve/knowledge/reasoning_pipeline.hpp"

#include "eve/knowledge/knowledge_graph.hpp"
#include "eve/knowledge/relationship_builder.hpp"

#include <algorithm>

namespace eve {

ReasoningPipeline::ReasoningPipeline(std::vector<KnowledgeObject> objects)
    : index_(KnowledgeIndexBuilder::build(
          KnowledgeGraphBuilder::build(RelationshipBuilder::build(std::move(objects))))),
      search_engine_(index_),
      ranking_engine_(index_),
      citation_engine_(index_, index_.graph()),
      context_assembler_(index_, index_.graph()) {}

std::shared_ptr<ReasoningPipeline> ReasoningPipeline::build_shared(
    std::vector<KnowledgeObject> objects) {
    return std::shared_ptr<ReasoningPipeline>(new ReasoningPipeline(std::move(objects)));
}

SearchEngineResultSet ReasoningPipeline::search(const SearchQuery& query) const {
    return search_engine_.search(query);
}

RankedSearchResultSet ReasoningPipeline::rank(const SearchEngineResultSet& candidates) const {
    return ranking_engine_.rank(candidates);
}

CitationResultSet ReasoningPipeline::cite(const RankedSearchResultSet& ranked_results) const {
    return citation_engine_.cite(ranked_results);
}

context::ContextAssemblyResult ReasoningPipeline::assemble(
    const context::ContextAssemblyRequest& request) const {
    return context_assembler_.assemble(request);
}

const KnowledgeIndex& ReasoningPipeline::index() const noexcept {
    return index_;
}

const KnowledgeGraph& ReasoningPipeline::graph() const noexcept {
    return index_.graph();
}

const SearchEngine& ReasoningPipeline::search_engine() const noexcept {
    return search_engine_;
}

const RankingEngine& ReasoningPipeline::ranking_engine() const noexcept {
    return ranking_engine_;
}

const CitationEngine& ReasoningPipeline::citation_engine() const noexcept {
    return citation_engine_;
}

const context::ContextAssembler& ReasoningPipeline::context_assembler() const noexcept {
    return context_assembler_;
}

const std::vector<KnowledgeObject>& ReasoningPipeline::objects() const noexcept {
    return index_.graph().objects();
}

SearchEngineResultSet filter_search_results_by_repository(
    SearchEngineResultSet results,
    std::string_view repository) {
    auto end = std::remove_if(
        results.results.begin(),
        results.results.end(),
        [repository](const SearchEngineResult& candidate) {
            return candidate.repository != repository;
        });
    results.results.erase(end, results.results.end());
    results.result_count = results.results.size();
    return results;
}

std::vector<SearchResult> to_legacy_search_results(
    const SearchEngineResultSet& results,
    const RankedSearchResultSet& ranked_results,
    const KnowledgeIndex& index) {
    std::vector<SearchResult> legacy;
    legacy.reserve(results.results.size());

    for (const auto& candidate : results.results) {
        const auto matches = index.lookup_by_object_id(candidate.object_identifier);
        if (matches.empty()) {
            continue;
        }

        double score = 0.0;
        for (const auto& ranked : ranked_results.results) {
            if (ranked.candidate.object_identifier == candidate.object_identifier) {
                score = static_cast<double>(ranked.rank_score);
                break;
            }
        }

        legacy.push_back(SearchResult{.object = *matches.front(), .score = score});
    }

    std::stable_sort(legacy.begin(), legacy.end(), [](const SearchResult& lhs, const SearchResult& rhs) {
        if (lhs.score != rhs.score) {
            return lhs.score > rhs.score;
        }
        return lhs.object.identity.id.value < rhs.object.identity.id.value;
    });

    return legacy;
}

}  // namespace eve
