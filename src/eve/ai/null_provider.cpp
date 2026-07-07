#include "eve/ai/provider_manager.hpp"

#include <format>

namespace eve::ai {

ProviderId NullProvider::id() const {
    return ProviderId{"AI-0000"};
}

std::string NullProvider::name() const {
    return "NullProvider";
}

std::expected<AIResponse, ProviderError> NullProvider::generate(
    const context::ContextPackage& package) const {
    return AIResponse{
        .generated_text = std::format(
            "No AI provider is configured. Context package {} contains {} knowledge object(s).",
            package.metadata().package_id.value,
            package.knowledge_objects().size()),
        .citations = {},
        .warnings = {DiagnosticMessage{
            "warning",
            "NullProvider is active. Configure an AI provider for natural language generation.",
        }},
        .provider_id = id().value,
    };
}

}  // namespace eve::ai
