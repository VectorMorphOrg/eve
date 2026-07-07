#pragma once

#include "eve/ai/provider_manager.hpp"
#include "eve/capability/capability_engine.hpp"
#include "eve/context/context_package.hpp"
#include "eve/core/platform_request.hpp"
#include "eve/core/platform_response.hpp"
#include "eve/knowledge/reasoning_pipeline.hpp"
#include "eve/services/service_interfaces.hpp"

#include <memory>

namespace eve::capability::handlers {

void register_core_handlers(
    CapabilityRegistry& registry,
    const services::IConfigurationService& configuration,
    const services::IStatusService& status,
    const services::IDiagnosticsService& diagnostics,
    const context::ContextBuilder& context_builder);

void register_documentation_handlers(
    CapabilityRegistry& registry,
    const services::IDocumentationService& documentation,
    const services::IStatusService& status,
    const ReasoningPipeline& reasoning_pipeline,
    std::shared_ptr<const ai::ProviderManager> provider_manager,
    std::size_t context_limit_chars);

}  // namespace eve::capability::handlers
