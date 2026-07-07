#include "eve/services/service_implementations.hpp"

namespace eve::services {

DocumentationService::DocumentationService(std::shared_ptr<IKnowledgeStore> store)
    : store_(std::move(store)) {}

std::expected<std::vector<KnowledgeObject>, PlatformError> DocumentationService::lookup(
    std::string_view identifier) const {
    if (const auto object = store_->get_by_id(KnowledgeObjectId{std::string{identifier}})) {
        return std::vector<KnowledgeObject>{*object};
    }

    const auto results = store_->search(identifier);
    if (results.empty()) {
        return std::unexpected(PlatformError{
            .type = "not_found",
            .description = std::format("No documentation found for '{}'.", identifier),
            .resolution = "Verify the identifier or search query.",
        });
    }

    std::vector<KnowledgeObject> objects;
    objects.reserve(results.size());
    for (const auto& result : results) {
        objects.push_back(result.object);
    }
    return objects;
}

std::expected<std::vector<KnowledgeObject>, PlatformError> DocumentationService::list_repository(
    std::string_view repository) const {
    auto objects = store_->list_by_repository(repository);
    if (objects.empty()) {
        return std::unexpected(PlatformError{
            .type = "not_found",
            .description = std::format("No documents indexed for repository '{}'.", repository),
            .resolution = "Verify repository configuration and indexing paths.",
        });
    }
    return objects;
}

}  // namespace eve::services
