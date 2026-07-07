#pragma once

#include "eve/ai/provider_manager.hpp"
#include "eve/capability/capability_engine.hpp"
#include "eve/context/context_package.hpp"
#include "eve/core/platform_request.hpp"
#include "eve/core/platform_response.hpp"
#include "eve/dispatcher/command_dispatcher.hpp"
#include "eve/knowledge/in_memory_knowledge_store.hpp"
#include "eve/services/service_interfaces.hpp"

#include "eve/validation/validation_engine.hpp"

#include <chrono>
#include <expected>
#include <filesystem>
#include <memory>

namespace eve {

struct CorePlatformDependencies {
    std::shared_ptr<services::IConfigurationService> configuration;
    std::shared_ptr<services::ILoggingService> logging;
    std::shared_ptr<services::IDocumentationService> documentation;
    std::shared_ptr<services::ISearchService> search;
    std::shared_ptr<services::IStatusService> status;
    std::shared_ptr<services::IDiagnosticsService> diagnostics;
    std::shared_ptr<IKnowledgeStore> knowledge_store;
    context::ContextBuilder context_builder;
    std::shared_ptr<ai::ProviderManager> provider_manager;
    std::shared_ptr<validation::ValidationEngine> validation_engine;
    capability::CapabilityEngine capability_engine;
    dispatcher::CommandDispatcher dispatcher;
};

class CorePlatform {
public:
    explicit CorePlatform(CorePlatformDependencies dependencies);

    [[nodiscard]] std::expected<PlatformResponse, PlatformError> process(
        PlatformRequest request) const;

    [[nodiscard]] const CorePlatformDependencies& dependencies() const noexcept {
        return dependencies_;
    }

private:
    [[nodiscard]] std::vector<KnowledgeObject> gather_knowledge(
        const PlatformRequest& request) const;

    CorePlatformDependencies dependencies_;
};

class PlatformBootstrap {
public:
    [[nodiscard]] static CorePlatform create(
        const std::filesystem::path& project_root,
        const std::filesystem::path& config_path = "config/platform.json");
};

}  // namespace eve
