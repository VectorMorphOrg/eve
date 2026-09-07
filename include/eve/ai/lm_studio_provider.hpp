#pragma once

#include "eve/ai/http_transport.hpp"
#include "eve/ai/provider_formatting.hpp"
#include "eve/ai/provider_manager.hpp"
#include "eve/services/service_interfaces.hpp"

#include <chrono>
#include <memory>
#include <optional>
#include <string>

namespace eve::ai {

struct LMStudioProviderConfig {
    std::string base_url{"http://localhost:1234"};
    std::string model{};
    double temperature{0.7};
    double top_p{0.9};
    std::chrono::milliseconds timeout{std::chrono::seconds{120}};
    std::optional<std::string> api_key{};

    [[nodiscard]] static LMStudioProviderConfig defaults();
    [[nodiscard]] static LMStudioProviderConfig from_configuration(
        const services::IConfigurationService& configuration);
    [[nodiscard]] ProviderOptions default_options() const;
};

struct LMStudioChatResponse {
    std::string model;
    std::string content;
    std::optional<std::size_t> prompt_tokens;
    std::optional<std::size_t> completion_tokens;
    std::optional<std::string> error;
};

[[nodiscard]] std::string serialize_lm_studio_chat_request(
    const ProviderRequest& request,
    bool stream = false);
[[nodiscard]] LMStudioChatResponse parse_lm_studio_chat_response(std::string_view body);
[[nodiscard]] std::string build_lm_studio_chat_url(std::string_view base_url);

class LMStudioProvider final : public IAIProvider {
public:
    LMStudioProvider(
        LMStudioProviderConfig config,
        std::shared_ptr<IHttpTransport> transport = std::make_shared<SocketHttpTransport>());

    [[nodiscard]] ProviderId id() const override;
    [[nodiscard]] std::string name() const override;
    [[nodiscard]] ProviderCapabilities capabilities() const override;
    [[nodiscard]] ProviderOptions default_options() const override;
    [[nodiscard]] std::expected<AIResponse, ProviderError> generate(
        const ProviderRequest& request) const override;
    [[nodiscard]] std::expected<AIResponse, ProviderError> generate_stream(
        const ProviderRequest& request,
        const StreamConsumer& consumer) const override;

    [[nodiscard]] const LMStudioProviderConfig& config() const noexcept;

private:
    [[nodiscard]] std::expected<AIResponse, ProviderError> generate_from_chat_response(
        const LMStudioChatResponse& chat_response,
        const ProviderRequest& request) const;

    LMStudioProviderConfig config_;
    std::shared_ptr<IHttpTransport> transport_;
};

}  // namespace eve::ai
