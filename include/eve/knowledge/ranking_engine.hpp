#pragma once

#include "eve/knowledge/knowledge_index.hpp"
#include "eve/knowledge/search_engine.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace eve {

struct RankedSearchResult {
    SearchEngineResult candidate;
    int rank_score{0};
    std::size_t rank_position{0};
};

struct RankedSearchResultSet {
    SearchQuery query;
    SearchMode resolved_mode{SearchMode::Auto};
    std::size_t result_count{0};
    std::vector<RankedSearchResult> results;
};

class RankingEngine {
public:
    explicit RankingEngine(const KnowledgeIndex& index);

    [[nodiscard]] RankedSearchResultSet rank(const SearchEngineResultSet& candidates) const;
    [[nodiscard]] const KnowledgeIndex& index() const noexcept;

private:
    const KnowledgeIndex* index_;
};

}  // namespace eve
