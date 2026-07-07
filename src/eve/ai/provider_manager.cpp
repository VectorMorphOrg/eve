#include "eve/ai/provider_manager.hpp"

namespace eve::ai {

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

std::expected<AIResponse, ProviderError> ProviderManager::generate(
    const context::ContextPackage& package) const {
    const auto provider = active_provider();
    if (!provider) {
        return std::unexpected(ProviderError{
            .provider_id = active_provider_id_.value,
            .message = "No AI provider is registered.",
        });
    }
    return provider->generate(package);
}

}  // namespace eve::ai
