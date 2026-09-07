#include "eve/ai/provider_manager.hpp"

#include <algorithm>
#include <format>

namespace eve::ai {
namespace {

constexpr std::size_t kNullStreamChunkSize = 16;

}  // namespace

ProviderId NullProvider::id() const {
    return ProviderId{"AI-0000"};
}

std::string NullProvider::name() const {
    return "NullProvider";
}

ProviderCapabilities NullProvider::capabilities() const {
    return null_provider_capabilities();
}

ProviderOptions NullProvider::default_options() const {
    return ProviderOptions{};
}

std::expected<AIResponse, ProviderError> NullProvider::generate(
    const ProviderRequest& request) const {
    const auto& package = request.context_package;
    return AIResponse{
        .generated_text = std::format(
            "No AI provider is configured. Context package {} contains {} knowledge object(s).",
            package.metadata().package_id.value,
            package.knowledge_objects().size()),
        .warnings = {DiagnosticMessage{
            "warning",
            "NullProvider is active. Configure an AI provider for natural language generation.",
        }},
        .provider_id = id().value,
    };
}

std::expected<AIResponse, ProviderError> NullProvider::generate_stream(
    const ProviderRequest& request,
    const StreamConsumer& consumer) const {
    auto response = generate(request);
    if (!response.has_value()) {
        return response;
    }

    if (!consumer) {
        return response;
    }

    const std::string& text = response->generated_text;
    if (text.empty()) {
        consumer(StreamChunk{.text_delta = {}, .done = true});
        return response;
    }

    for (std::size_t offset = 0; offset < text.size(); offset += kNullStreamChunkSize) {
        const auto size = std::min(kNullStreamChunkSize, text.size() - offset);
        const bool is_last = offset + size >= text.size();
        consumer(StreamChunk{
            .text_delta = text.substr(offset, size),
            .done = is_last,
        });
    }

    return response;
}

}  // namespace eve::ai
