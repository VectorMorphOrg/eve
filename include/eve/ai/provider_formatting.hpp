#pragma once

#include "eve/context/context_package.hpp"
#include "eve/core/platform_request.hpp"
#include "eve/core/types.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace eve::ai {

enum class ProviderMessageRole {
    System,
    User,
    Assistant,
    Tool,
};

[[nodiscard]] std::string to_string(ProviderMessageRole role);

struct ProviderMessage {
    ProviderMessageRole role{ProviderMessageRole::User};
    std::string content;
};

struct ProviderOptions {
    std::string model;
    double temperature{0.7};
    double top_p{0.9};
    std::optional<std::size_t> max_tokens;
    std::vector<std::string> stop_sequences;
    std::optional<std::size_t> context_length;
    std::optional<std::int64_t> seed;
};

struct ProviderCapabilities {
    bool supports_streaming{false};
    bool supports_system_prompts{true};
    bool supports_tool_calling{false};
    bool supports_json_responses{false};
    bool supports_image_input{false};
    bool supports_reasoning_mode{false};
    bool supports_function_calling{false};
    bool supports_multiple_messages{true};
};

struct ProviderMetadata {
    ProviderId provider_id;
    std::string provider_name;
    std::string formatter_version{"0.1.0"};
    RequestId request_id;
    PackageId package_id;
};

struct ProviderRequest {
    ProviderCapabilities capabilities;
    ProviderOptions options;
    std::vector<ProviderMessage> messages;
    context::SystemInstructions system_instructions;
    PlatformRequest platform_request;
    context::ContextPackage context_package;
    ProviderMetadata metadata;
};

class ProviderFormatter {
public:
    [[nodiscard]] ProviderRequest format(
        const context::ContextPackage& package,
        ProviderCapabilities capabilities,
        ProviderOptions options,
        ProviderMetadata metadata) const;
};

[[nodiscard]] ProviderCapabilities null_provider_capabilities();
[[nodiscard]] ProviderCapabilities ollama_provider_capabilities();

}  // namespace eve::ai
