#include "eve/services/service_implementations.hpp"

namespace eve::services {

SearchService::SearchService(
    std::shared_ptr<IKnowledgeStore> store,
    std::shared_ptr<const ReasoningPipeline> reasoning_pipeline)
    : store_(std::move(store)), reasoning_pipeline_(std::move(reasoning_pipeline)) {}

std::expected<std::vector<SearchResult>, PlatformError> SearchService::search(
    std::string_view query,
    std::optional<std::string_view> repository) const {
    if (query.empty()) {
        return std::unexpected(PlatformError{
            .type = "validation_error",
            .description = "Search query must not be empty.",
            .resolution = "Provide a non-empty query parameter.",
        });
    }

    if (!reasoning_pipeline_) {
        const auto results = store_->search(query, repository);
        if (results.empty()) {
            return std::unexpected(PlatformError{
                .type = "not_found",
                .description = std::format("No search results for '{}'.", query),
                .resolution = "Try a different query or verify documentation indexing.",
            });
        }
        return results;
    }

    SearchQuery search_query{
        .text = std::string{query},
        .mode = SearchMode::Auto,
        .exact_match = false,
        .partial_match = true,
    };

    auto search_results = reasoning_pipeline_->search(search_query);
    if (repository) {
        search_results = filter_search_results_by_repository(std::move(search_results), *repository);
    }

    if (search_results.results.empty()) {
        return std::unexpected(PlatformError{
            .type = "not_found",
            .description = std::format("No search results for '{}'.", query),
            .resolution = "Try a different query or verify documentation indexing.",
        });
    }

    const auto ranked_results = reasoning_pipeline_->rank(search_results);
    return to_legacy_search_results(search_results, ranked_results, reasoning_pipeline_->index());
}

}  // namespace eve::services
