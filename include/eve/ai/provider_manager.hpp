#pragma once

#include "eve/core/errors.hpp"
#include "eve/context/context_package.hpp"

#include <expected>
#include <memory>
#include <string>
#include <vector>

namespace eve::ai {

struct AIResponse {
    std::string generated_text;
    std::vector<context::Citation> citations;
    std::vector<DiagnosticMessage> warnings;
    std::string provider_id;
};

class IAIProvider {
public:
    virtual ~IAIProvider() = default;

    [[nodiscard]] virtual ProviderId id() const = 0;
    [[nodiscard]] virtual std::string name() const = 0;
    [[nodiscard]] virtual std::expected<AIResponse, ProviderError> generate(
        const context::ContextPackage& package) const = 0;
};

class ProviderManager {
public:
    void register_provider(std::shared_ptr<IAIProvider> provider);
    void set_active_provider(ProviderId provider_id);
    [[nodiscard]] std::shared_ptr<IAIProvider> active_provider() const;
    [[nodiscard]] std::expected<AIResponse, ProviderError> generate(
        const context::ContextPackage& package) const;

private:
    ProviderId active_provider_id_{};
    std::vector<std::shared_ptr<IAIProvider>> providers_;
};

class NullProvider final : public IAIProvider {
public:
    [[nodiscard]] ProviderId id() const override;
    [[nodiscard]] std::string name() const override;
    [[nodiscard]] std::expected<AIResponse, ProviderError> generate(
        const context::ContextPackage& package) const override;
};

}  // namespace eve::ai
