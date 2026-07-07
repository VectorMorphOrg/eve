#include "eve/ai/provider_manager.hpp"

namespace eve::ai {

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
