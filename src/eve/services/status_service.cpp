#include "eve/services/service_implementations.hpp"

#include <format>

namespace eve::services {

StatusService::StatusService(
    const IConfigurationService& configuration,
    const IKnowledgeStore& knowledge_store,
    std::shared_ptr<const ai::ProviderManager> provider_manager)
    : configuration_(configuration),
      knowledge_store_(knowledge_store),
      provider_manager_(std::move(provider_manager)) {}

PlatformResponse StatusService::report_status(const PlatformRequest& request) const {
    const auto provider = provider_manager_->active_provider();
    const auto provider_name = provider ? provider->name() : "none";

    ResponseContent content{
        .primary = std::format(
            "E.V.E. platform version {} (specification {}). Indexed documents: {}. Active AI "
            "provider: {}.",
            configuration_.platform_version(),
            configuration_.specification_version(),
            knowledge_store_.document_count(),
            provider_name),
    };
    content.structured["platform_version"] = configuration_.platform_version();
    content.structured["specification_version"] = configuration_.specification_version();
    content.structured["indexed_documents"] = std::to_string(knowledge_store_.document_count());
    content.structured["active_ai_provider"] = provider_name;

    return PlatformResponse::success(request, std::move(content));
}

}  // namespace eve::services
