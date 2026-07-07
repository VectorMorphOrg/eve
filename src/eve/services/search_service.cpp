#include "eve/services/service_implementations.hpp"

namespace eve::services {

SearchService::SearchService(std::shared_ptr<IKnowledgeStore> store)
    : store_(std::move(store)) {}

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

}  // namespace eve::services
