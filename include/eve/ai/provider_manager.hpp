#pragma once

#include "eve/core/errors.hpp"
#include "eve/context/context_package.hpp"
#include "eve/ai/provider_formatting.hpp"

#include <expected>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace eve::ai {

struct AIResponse {
    std::string generated_text;
    std::vector<context::Citation> citations;
    std::vector<DiagnosticMessage> warnings;
    std::string provider_id;
    std::optional<std::string> model;
    std::optional<std::size_t> prompt_eval_count;
    std::optional<std::size_t> eval_count;
    std::optional<std::int64_t> total_duration_ns;
};

/// Incremental assistant text event for provider streaming.
/// text_delta: provider-supplied text fragment.
/// done: provider indicates stream completion (optional per chunk).
struct StreamChunk {
    std::string text_delta;
    bool done{false};
};

/// Invoked by a provider during streaming generation.
/// Must not be retained after generate_stream() returns.
using StreamConsumer = std::function<void(const StreamChunk&)>;

class IAIProvider {
public:
    virtual ~IAIProvider() = default;

    [[nodiscard]] virtual ProviderId id() const = 0;
    [[nodiscard]] virtual std::string name() const = 0;
    [[nodiscard]] virtual ProviderCapabilities capabilities() const = 0;
    [[nodiscard]] virtual ProviderOptions default_options() const = 0;
    [[nodiscard]] virtual std::expected<AIResponse, ProviderError> generate(
        const ProviderRequest& request) const = 0;

    /// Default: provider does not support streaming.
    /// ProviderManager falls back to generate() when capabilities.supports_streaming is false.
    [[nodiscard]] virtual std::expected<AIResponse, ProviderError> generate_stream(
        const ProviderRequest& request,
        const StreamConsumer& consumer) const;
};

class ProviderManager {
public:
    [[nodiscard]] std::expected<AIResponse, ProviderError> generate(
        const context::ContextPackage& package) const;

    [[nodiscard]] std::expected<AIResponse, ProviderError> generate_stream(
        const context::ContextPackage& package,
        const StreamConsumer& consumer) const;

    void register_provider(std::shared_ptr<IAIProvider> provider);
    void set_active_provider(ProviderId provider_id);
    [[nodiscard]] std::shared_ptr<IAIProvider> active_provider() const;
    [[nodiscard]] const ProviderFormatter& formatter() const noexcept;

private:
    ProviderFormatter formatter_;
    ProviderId active_provider_id_{};
    std::vector<std::shared_ptr<IAIProvider>> providers_;
};

class NullProvider final : public IAIProvider {
public:
    [[nodiscard]] ProviderId id() const override;
    [[nodiscard]] std::string name() const override;
    [[nodiscard]] ProviderCapabilities capabilities() const override;
    [[nodiscard]] ProviderOptions default_options() const override;
    [[nodiscard]] std::expected<AIResponse, ProviderError> generate(
        const ProviderRequest& request) const override;
    [[nodiscard]] std::expected<AIResponse, ProviderError> generate_stream(
        const ProviderRequest& request,
        const StreamConsumer& consumer) const override;
};

}  // namespace eve::ai
