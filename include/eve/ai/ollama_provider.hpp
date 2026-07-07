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

struct OllamaProviderConfig {
    std::string base_url{"http://localhost:11434"};
    std::string model{"qwen2.5:14b"};
    double temperature{0.7};
    double top_p{0.9};
    std::size_t context_length{32768};
    std::chrono::milliseconds timeout{std::chrono::seconds{120}};

    [[nodiscard]] static OllamaProviderConfig defaults();
    [[nodiscard]] static OllamaProviderConfig from_configuration(
        const services::IConfigurationService& configuration);
    [[nodiscard]] ProviderOptions default_options() const;
};

struct OllamaChatResponse {
    std::string model;
    std::string content;
    std::optional<std::size_t> prompt_eval_count;
    std::optional<std::size_t> eval_count;
    std::optional<std::int64_t> total_duration_ns;
    std::optional<std::string> error;
};

[[nodiscard]] std::string escape_json_string(std::string_view value);
[[nodiscard]] std::string serialize_ollama_chat_request(const ProviderRequest& request);
[[nodiscard]] OllamaChatResponse parse_ollama_chat_response(std::string_view body);
[[nodiscard]] std::string build_chat_url(std::string_view base_url);

class OllamaProvider final : public IAIProvider {
public:
    OllamaProvider(
        OllamaProviderConfig config,
        std::shared_ptr<IHttpTransport> transport = std::make_shared<SocketHttpTransport>());

    [[nodiscard]] ProviderId id() const override;
    [[nodiscard]] std::string name() const override;
    [[nodiscard]] ProviderCapabilities capabilities() const override;
    [[nodiscard]] ProviderOptions default_options() const override;
    [[nodiscard]] std::expected<AIResponse, ProviderError> generate(
        const ProviderRequest& request) const override;

    [[nodiscard]] std::expected<void, ProviderError> health_check() const;
    [[nodiscard]] const OllamaProviderConfig& config() const noexcept;

private:
    [[nodiscard]] std::expected<AIResponse, ProviderError> generate_from_chat_response(
        const OllamaChatResponse& chat_response,
        const ProviderRequest& request) const;

    OllamaProviderConfig config_;
    std::shared_ptr<IHttpTransport> transport_;
};

}  // namespace eve::ai
