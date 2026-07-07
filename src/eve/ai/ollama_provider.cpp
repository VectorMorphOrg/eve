#include "eve/ai/ollama_provider.hpp"

#include <charconv>
#include <format>
#include <sstream>

namespace eve::ai {
namespace {

std::optional<std::string_view> extract_json_string_value(
    std::string_view body,
    std::string_view key) {
    const std::string spaced_pattern = std::format("\"{}\": \"", key);
    const std::string compact_pattern = std::format("\"{}\":\"", key);

    std::size_t start = body.find(spaced_pattern);
    std::size_t value_start = 0;
    if (start != std::string_view::npos) {
        value_start = start + spaced_pattern.size();
    } else {
        start = body.find(compact_pattern);
        if (start == std::string_view::npos) {
            return std::nullopt;
        }
        value_start = start + compact_pattern.size();
    }

    const auto value_end = body.find('"', value_start);
    if (value_end == std::string_view::npos) {
        return std::nullopt;
    }
    return body.substr(value_start, value_end - value_start);
}

std::optional<std::size_t> extract_json_number_value(std::string_view body, std::string_view key) {
    const std::string spaced_pattern = std::format("\"{}\": ", key);
    const std::string compact_pattern = std::format("\"{}\":", key);

    std::size_t start = body.find(spaced_pattern);
    std::size_t value_start = 0;
    if (start != std::string_view::npos) {
        value_start = start + spaced_pattern.size();
    } else {
        start = body.find(compact_pattern);
        if (start == std::string_view::npos) {
            return std::nullopt;
        }
        value_start = start + compact_pattern.size();
    }

    std::size_t value = 0;
    const auto data = body.substr(value_start);
    const auto [pointer, error_code] = std::from_chars(data.data(), data.data() + data.size(), value);
    if (error_code != std::errc{}) {
        return std::nullopt;
    }
    return value;
}

std::optional<std::int64_t> extract_json_int64_value(std::string_view body, std::string_view key) {
    const std::string spaced_pattern = std::format("\"{}\": ", key);
    const std::string compact_pattern = std::format("\"{}\":", key);

    std::size_t start = body.find(spaced_pattern);
    std::size_t value_start = 0;
    if (start != std::string_view::npos) {
        value_start = start + spaced_pattern.size();
    } else {
        start = body.find(compact_pattern);
        if (start == std::string_view::npos) {
            return std::nullopt;
        }
        value_start = start + compact_pattern.size();
    }

    std::int64_t value = 0;
    const auto data = body.substr(value_start);
    const auto [pointer, error_code] = std::from_chars(data.data(), data.data() + data.size(), value);
    if (error_code != std::errc{}) {
        return std::nullopt;
    }
    return value;
}

std::string unescape_json_string(std::string_view value) {
    std::string unescaped;
    unescaped.reserve(value.size());
    for (std::size_t index = 0; index < value.size(); ++index) {
        if (value[index] != '\\' || index + 1 >= value.size()) {
            unescaped.push_back(value[index]);
            continue;
        }

        const char next = value[++index];
        switch (next) {
            case '"':
            case '\\':
            case '/':
                unescaped.push_back(next);
                break;
            case 'n':
                unescaped.push_back('\n');
                break;
            case 'r':
                unescaped.push_back('\r');
                break;
            case 't':
                unescaped.push_back('\t');
                break;
            default:
                unescaped.push_back(next);
                break;
        }
    }
    return unescaped;
}

std::optional<std::string> read_configuration_string(
    const services::IConfigurationService& configuration,
    std::string_view key,
    std::string_view fallback) {
    if (const auto value = configuration.get(key)) {
        return *value;
    }
    return std::string{fallback};
}

double read_configuration_double(
    const services::IConfigurationService& configuration,
    std::string_view key,
    double fallback) {
    if (const auto value = configuration.get(key)) {
        try {
            return std::stod(*value);
        } catch (...) {
            return fallback;
        }
    }
    return fallback;
}

std::size_t read_configuration_size_t(
    const services::IConfigurationService& configuration,
    std::string_view key,
    std::size_t fallback) {
    if (const auto value = configuration.get(key)) {
        try {
            return static_cast<std::size_t>(std::stoull(*value));
        } catch (...) {
            return fallback;
        }
    }
    return fallback;
}

}  // namespace

OllamaProviderConfig OllamaProviderConfig::defaults() {
    return OllamaProviderConfig{};
}

OllamaProviderConfig OllamaProviderConfig::from_configuration(
    const services::IConfigurationService& configuration) {
    OllamaProviderConfig config = defaults();
    config.base_url = read_configuration_string(configuration, "ollama_base_url", config.base_url)
                          .value_or(config.base_url);
    config.model = read_configuration_string(configuration, "ollama_model", config.model).value_or(config.model);
    config.temperature = read_configuration_double(configuration, "ollama_temperature", config.temperature);
    config.top_p = read_configuration_double(configuration, "ollama_top_p", config.top_p);
    config.context_length =
        read_configuration_size_t(configuration, "ollama_context_length", config.context_length);
    config.timeout = std::chrono::milliseconds(
        read_configuration_size_t(configuration, "ollama_timeout_ms", config.timeout.count()));
    return config;
}

ProviderOptions OllamaProviderConfig::default_options() const {
    return ProviderOptions{
        .model = model,
        .temperature = temperature,
        .top_p = top_p,
        .context_length = context_length,
    };
}

std::string escape_json_string(std::string_view value) {
    std::string escaped;
    escaped.reserve(value.size() + 8);
    for (const char ch : value) {
        switch (ch) {
            case '"':
                escaped += "\\\"";
                break;
            case '\\':
                escaped += "\\\\";
                break;
            case '\n':
                escaped += "\\n";
                break;
            case '\r':
                escaped += "\\r";
                break;
            case '\t':
                escaped += "\\t";
                break;
            default:
                escaped.push_back(ch);
                break;
        }
    }
    return escaped;
}

std::string serialize_ollama_chat_request(const ProviderRequest& request) {
    const auto& options = request.options;
    const auto context_length = options.context_length.value_or(32768U);

    std::ostringstream stream;
    stream << '{';
    stream << "\"model\": \"" << escape_json_string(options.model) << "\",";
    stream << "\"stream\": false,";
    stream << "\"messages\": [";
    for (std::size_t index = 0; index < request.messages.size(); ++index) {
        if (index > 0) {
            stream << ',';
        }
        const auto& message = request.messages[index];
        stream << "{\"role\": \"" << escape_json_string(to_string(message.role)) << "\",";
        stream << "\"content\": \"" << escape_json_string(message.content) << "\"}";
    }
    stream << "],";
    stream << "\"options\": {";
    stream << "\"temperature\": " << options.temperature << ',';
    stream << "\"top_p\": " << options.top_p << ',';
    stream << "\"num_ctx\": " << context_length;
    stream << "}}";
    return stream.str();
}

OllamaChatResponse parse_ollama_chat_response(std::string_view body) {
    OllamaChatResponse response;

    if (const auto error = extract_json_string_value(body, "error")) {
        response.error = unescape_json_string(*error);
        return response;
    }

    if (const auto model = extract_json_string_value(body, "model")) {
        response.model = unescape_json_string(*model);
    }

    const auto message_pos = body.find("\"message\"");
    const auto search_from = message_pos == std::string_view::npos ? 0 : message_pos;
    const std::string spaced_content = "\"content\": \"";
    const std::string compact_content = "\"content\":\"";
    auto content_key = body.find(spaced_content, search_from);
    std::size_t value_start = 0;
    if (content_key != std::string_view::npos) {
        value_start = content_key + spaced_content.size();
    } else {
        content_key = body.find(compact_content, search_from);
        if (content_key == std::string_view::npos) {
            return response;
        }
        value_start = content_key + compact_content.size();
    }

    std::string content;
    for (std::size_t index = value_start; index < body.size(); ++index) {
        if (body[index] == '"' && (index == value_start || body[index - 1] != '\\')) {
            response.content = unescape_json_string(content);
            break;
        }
        content.push_back(body[index]);
    }

    response.prompt_eval_count = extract_json_number_value(body, "prompt_eval_count");
    response.eval_count = extract_json_number_value(body, "eval_count");
    response.total_duration_ns = extract_json_int64_value(body, "total_duration");
    return response;
}

std::string build_chat_url(std::string_view base_url) {
    std::string url{base_url};
    while (!url.empty() && url.back() == '/') {
        url.pop_back();
    }
    url += "/api/chat";
    return url;
}

OllamaProvider::OllamaProvider(
    OllamaProviderConfig config,
    std::shared_ptr<IHttpTransport> transport)
    : config_(std::move(config)), transport_(std::move(transport)) {}

ProviderId OllamaProvider::id() const {
    return ProviderId{"AI-0100"};
}

std::string OllamaProvider::name() const {
    return "Ollama";
}

ProviderCapabilities OllamaProvider::capabilities() const {
    return ollama_provider_capabilities();
}

ProviderOptions OllamaProvider::default_options() const {
    return config_.default_options();
}

const OllamaProviderConfig& OllamaProvider::config() const noexcept {
    return config_;
}

std::expected<AIResponse, ProviderError> OllamaProvider::generate_from_chat_response(
    const OllamaChatResponse& chat_response,
    const ProviderRequest& request) const {
    if (chat_response.error.has_value()) {
        return std::unexpected(ProviderError{
            .provider_id = id().value,
            .message = *chat_response.error,
        });
    }

    if (chat_response.content.empty()) {
        return std::unexpected(ProviderError{
            .provider_id = id().value,
            .message = "Ollama returned an empty response.",
        });
    }

    const auto& model = request.options.model;
    AIResponse response{
        .generated_text = chat_response.content,
        .provider_id = id().value,
        .model = chat_response.model.empty() ? std::optional<std::string>{model}
                                             : std::optional<std::string>{chat_response.model},
        .prompt_eval_count = chat_response.prompt_eval_count,
        .eval_count = chat_response.eval_count,
        .total_duration_ns = chat_response.total_duration_ns,
    };
    return response;
}

std::expected<AIResponse, ProviderError> OllamaProvider::generate(
    const ProviderRequest& request) const {
    const auto payload = serialize_ollama_chat_request(request);

    HttpRequest http_request{
        .method = "POST",
        .url = build_chat_url(config_.base_url),
        .body = payload,
        .timeout = config_.timeout,
    };

    const auto http_response = transport_->send(http_request);
    if (!http_response) {
        return std::unexpected(ProviderError{
            .provider_id = id().value,
            .message = http_response.error().message,
        });
    }

    if (http_response->status_code < 200 || http_response->status_code >= 300) {
        const auto parsed_error = parse_ollama_chat_response(http_response->body);
        if (parsed_error.error.has_value()) {
            return std::unexpected(ProviderError{
                .provider_id = id().value,
                .message = *parsed_error.error,
            });
        }
        return std::unexpected(ProviderError{
            .provider_id = id().value,
            .message = std::format(
                "Ollama request failed with HTTP {}.",
                http_response->status_code),
        });
    }

    return generate_from_chat_response(parse_ollama_chat_response(http_response->body), request);
}

std::expected<void, ProviderError> OllamaProvider::health_check() const {
    std::string url{config_.base_url};
    while (!url.empty() && url.back() == '/') {
        url.pop_back();
    }
    url += "/api/tags";

    HttpRequest request{
        .method = "GET",
        .url = url,
        .timeout = std::chrono::seconds{5},
    };

    const auto response = transport_->send(request);
    if (!response) {
        return std::unexpected(ProviderError{
            .provider_id = id().value,
            .message = response.error().message,
        });
    }

    if (response->status_code < 200 || response->status_code >= 300) {
        return std::unexpected(ProviderError{
            .provider_id = id().value,
            .message = std::format("Ollama health check failed with HTTP {}.", response->status_code),
        });
    }

    return {};
}

}  // namespace eve::ai
