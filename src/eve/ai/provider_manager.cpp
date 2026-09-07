#include "eve/ai/provider_manager.hpp"

namespace eve::ai {
namespace {

std::expected<AIResponse, ProviderError> unsupported_streaming_error(
    const IAIProvider& provider) {
    return std::unexpected(ProviderError{
        .provider_id = provider.id().value,
        .message = "Streaming is not supported by this provider.",
    });
}

}  // namespace

std::expected<AIResponse, ProviderError> IAIProvider::generate_stream(
    const ProviderRequest& /*request*/,
    const StreamConsumer& /*consumer*/) const {
    return unsupported_streaming_error(*this);
}

std::expected<AIResponse, ProviderError> ProviderManager::generate(
    const context::ContextPackage& package) const {
    const auto provider = active_provider();
    if (!provider) {
        return std::unexpected(ProviderError{
            .provider_id = active_provider_id_.value,
            .message = "No AI provider is registered.",
        });
    }

    ProviderMetadata metadata{
        .provider_id = provider->id(),
        .provider_name = provider->name(),
        .request_id = package.metadata().request_id,
        .package_id = package.metadata().package_id,
    };

    const auto provider_request = formatter_.format(
        package,
        provider->capabilities(),
        provider->default_options(),
        std::move(metadata));

    return provider->generate(provider_request);
}

std::expected<AIResponse, ProviderError> ProviderManager::generate_stream(
    const context::ContextPackage& package,
    const StreamConsumer& consumer) const {
    const auto provider = active_provider();
    if (!provider) {
        return std::unexpected(ProviderError{
            .provider_id = active_provider_id_.value,
            .message = "No AI provider is registered.",
        });
    }

    ProviderMetadata metadata{
        .provider_id = provider->id(),
        .provider_name = provider->name(),
        .request_id = package.metadata().request_id,
        .package_id = package.metadata().package_id,
    };

    const auto provider_request = formatter_.format(
        package,
        provider->capabilities(),
        provider->default_options(),
        std::move(metadata));

    if (!provider->capabilities().supports_streaming) {
        auto response = provider->generate(provider_request);
        if (!response.has_value()) {
            return response;
        }
        if (consumer) {
            consumer(StreamChunk{
                .text_delta = response->generated_text,
                .done = true,
            });
        }
        return response;
    }

    return provider->generate_stream(provider_request, consumer);
}

void ProviderManager::register_provider(std::shared_ptr<IAIProvider> provider) {
    providers_.push_back(std::move(provider));
}

void ProviderManager::set_active_provider(ProviderId provider_id) {
    active_provider_id_ = std::move(provider_id);
}

std::shared_ptr<IAIProvider> ProviderManager::active_provider() const {
    for (const auto& provider : providers_) {
        if (provider->id().value == active_provider_id_.value) {
            return provider;
        }
    }
    if (!providers_.empty()) {
        return providers_.front();
    }
    return nullptr;
}

const ProviderFormatter& ProviderManager::formatter() const noexcept {
    return formatter_;
}

}  // namespace eve::ai
