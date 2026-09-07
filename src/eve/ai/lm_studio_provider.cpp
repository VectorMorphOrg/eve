#include "eve/ai/lm_studio_provider.hpp"

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

    std::string content;
    for (std::size_t index = value_start; index < body.size(); ++index) {
        if (body[index] == '"' && (index == value_start || body[index - 1] != '\\')) {
            return body.substr(value_start, index - value_start);
        }
    }
    return std::nullopt;
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

std::string escape_json_string_local(std::string_view value) {
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

std::optional<std::string> extract_nested_error_message(std::string_view body) {
    const auto error_pos = body.find("\"error\"");
    if (error_pos == std::string_view::npos) {
        return std::nullopt;
    }

    const auto after_error = body.substr(error_pos);
    if (const auto nested = extract_json_string_value(after_error, "message")) {
        return unescape_json_string(*nested);
    }
    if (const auto as_string = extract_json_string_value(body, "error")) {
        return unescape_json_string(*as_string);
    }
    return std::nullopt;
}

std::optional<std::string> extract_choice_message_content(std::string_view body) {
    const auto choices_pos = body.find("\"choices\"");
    if (choices_pos == std::string_view::npos) {
        return std::nullopt;
    }

    const auto search_from = choices_pos;
    const auto message_pos = body.find("\"message\"", search_from);
    if (message_pos == std::string_view::npos) {
        return std::nullopt;
    }

    const auto region = body.substr(message_pos);
    if (const auto content = extract_json_string_value(region, "content")) {
        return unescape_json_string(*content);
    }
    return std::nullopt;
}

std::optional<std::string> extract_choice_delta_content(std::string_view body) {
    const auto choices_pos = body.find("\"choices\"");
    if (choices_pos == std::string_view::npos) {
        return std::nullopt;
    }

    const auto delta_pos = body.find("\"delta\"", choices_pos);
    if (delta_pos == std::string_view::npos) {
        return std::nullopt;
    }

    const auto region = body.substr(delta_pos);
    if (const auto content = extract_json_string_value(region, "content")) {
        return unescape_json_string(*content);
    }
    return std::nullopt;
}

}  // namespace

LMStudioProviderConfig LMStudioProviderConfig::defaults() {
    return LMStudioProviderConfig{};
}

LMStudioProviderConfig LMStudioProviderConfig::from_configuration(
    const services::IConfigurationService& configuration) {
    LMStudioProviderConfig config = defaults();
    config.base_url =
        read_configuration_string(configuration, "lm_studio_base_url", config.base_url)
            .value_or(config.base_url);
    config.model =
        read_configuration_string(configuration, "lm_studio_model", config.model).value_or(config.model);
    config.temperature =
        read_configuration_double(configuration, "lm_studio_temperature", config.temperature);
    config.top_p = read_configuration_double(configuration, "lm_studio_top_p", config.top_p);
    config.timeout = std::chrono::milliseconds(
        read_configuration_size_t(configuration, "lm_studio_timeout_ms", config.timeout.count()));

    if (const auto key = configuration.get("lm_studio_api_key")) {
        if (!key->empty()) {
            config.api_key = *key;
        }
    }
    return config;
}

ProviderOptions LMStudioProviderConfig::default_options() const {
    return ProviderOptions{
        .model = model,
        .temperature = temperature,
        .top_p = top_p,
    };
}

std::string serialize_lm_studio_chat_request(const ProviderRequest& request, bool stream) {
    const auto& options = request.options;

    std::ostringstream out;
    out << '{';
    out << "\"model\": \"" << escape_json_string_local(options.model) << "\",";
    out << "\"stream\": " << (stream ? "true" : "false") << ',';
    out << "\"messages\": [";
    for (std::size_t index = 0; index < request.messages.size(); ++index) {
        if (index > 0) {
            out << ',';
        }
        const auto& message = request.messages[index];
        out << "{\"role\": \"" << escape_json_string_local(to_string(message.role)) << "\",";
        out << "\"content\": \"" << escape_json_string_local(message.content) << "\"}";
    }
    out << "],";
    out << "\"temperature\": " << options.temperature << ',';
    out << "\"top_p\": " << options.top_p;
    if (options.max_tokens.has_value()) {
        out << ",\"max_tokens\": " << *options.max_tokens;
    }
    out << '}';
    return out.str();
}

LMStudioChatResponse parse_lm_studio_chat_response(std::string_view body) {
    LMStudioChatResponse response;

    if (const auto error = extract_nested_error_message(body)) {
        response.error = *error;
        return response;
    }

    if (const auto model = extract_json_string_value(body, "model")) {
        response.model = unescape_json_string(*model);
    }

    if (const auto content = extract_choice_message_content(body)) {
        response.content = *content;
    }

    response.prompt_tokens = extract_json_number_value(body, "prompt_tokens");
    response.completion_tokens = extract_json_number_value(body, "completion_tokens");
    return response;
}

std::string build_lm_studio_chat_url(std::string_view base_url) {
    std::string url{base_url};
    while (!url.empty() && url.back() == '/') {
        url.pop_back();
    }
    url += "/v1/chat/completions";
    return url;
}

LMStudioProvider::LMStudioProvider(
    LMStudioProviderConfig config,
    std::shared_ptr<IHttpTransport> transport)
    : config_(std::move(config)), transport_(std::move(transport)) {}

ProviderId LMStudioProvider::id() const {
    return ProviderId{"AI-0102"};
}

std::string LMStudioProvider::name() const {
    return "LM Studio";
}

ProviderCapabilities LMStudioProvider::capabilities() const {
    return lm_studio_provider_capabilities();
}

ProviderOptions LMStudioProvider::default_options() const {
    return config_.default_options();
}

const LMStudioProviderConfig& LMStudioProvider::config() const noexcept {
    return config_;
}

std::expected<AIResponse, ProviderError> LMStudioProvider::generate_from_chat_response(
    const LMStudioChatResponse& chat_response,
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
            .message = "LM Studio returned an empty response.",
        });
    }

    const auto& model = request.options.model;
    return AIResponse{
        .generated_text = chat_response.content,
        .provider_id = id().value,
        .model = chat_response.model.empty() ? std::optional<std::string>{model}
                                             : std::optional<std::string>{chat_response.model},
        .prompt_eval_count = chat_response.prompt_tokens,
        .eval_count = chat_response.completion_tokens,
    };
}

std::expected<AIResponse, ProviderError> LMStudioProvider::generate(
    const ProviderRequest& request) const {
    const auto payload = serialize_lm_studio_chat_request(request, false);

    HttpRequest http_request{
        .method = "POST",
        .url = build_lm_studio_chat_url(config_.base_url),
        .body = payload,
        .timeout = config_.timeout,
    };
    if (config_.api_key.has_value() && !config_.api_key->empty()) {
        http_request.headers["Authorization"] = std::format("Bearer {}", *config_.api_key);
    }

    const auto http_response = transport_->send(http_request);
    if (!http_response) {
        return std::unexpected(ProviderError{
            .provider_id = id().value,
            .message = http_response.error().message,
        });
    }

    if (http_response->status_code < 200 || http_response->status_code >= 300) {
        const auto parsed_error = parse_lm_studio_chat_response(http_response->body);
        if (parsed_error.error.has_value()) {
            return std::unexpected(ProviderError{
                .provider_id = id().value,
                .message = *parsed_error.error,
            });
        }
        return std::unexpected(ProviderError{
            .provider_id = id().value,
            .message = std::format(
                "LM Studio request failed with HTTP {}.",
                http_response->status_code),
        });
    }

    const auto parsed = parse_lm_studio_chat_response(http_response->body);
    if (!parsed.error.has_value() && parsed.content.empty() &&
        http_response->body.find("\"choices\"") == std::string::npos) {
        return std::unexpected(ProviderError{
            .provider_id = id().value,
            .message = "Malformed LM Studio response.",
        });
    }
    if (!parsed.error.has_value() && parsed.content.empty() &&
        http_response->body.find("\"choices\"") != std::string::npos) {
        return std::unexpected(ProviderError{
            .provider_id = id().value,
            .message = "LM Studio response is missing message content.",
        });
    }

    return generate_from_chat_response(parsed, request);
}

std::expected<AIResponse, ProviderError> LMStudioProvider::generate_stream(
    const ProviderRequest& request,
    const StreamConsumer& consumer) const {
    const auto payload = serialize_lm_studio_chat_request(request, true);

    HttpRequest http_request{
        .method = "POST",
        .url = build_lm_studio_chat_url(config_.base_url),
        .body = payload,
        .timeout = config_.timeout,
    };
    if (config_.api_key.has_value() && !config_.api_key->empty()) {
        http_request.headers["Authorization"] = std::format("Bearer {}", *config_.api_key);
    }

    std::string line_buffer;
    std::string data_payload;
    std::string accumulated;
    std::string model;
    std::optional<std::size_t> prompt_tokens;
    std::optional<std::size_t> completion_tokens;
    bool saw_done = false;
    std::optional<ProviderError> stream_error;

    auto fail = [&](std::string message) {
        stream_error = ProviderError{
            .provider_id = id().value,
            .message = std::move(message),
        };
    };

    auto process_data_payload = [&](std::string_view payload) {
        if (stream_error.has_value() || saw_done) {
            return;
        }

        while (!payload.empty() &&
               (payload.front() == ' ' || payload.front() == '\t')) {
            payload.remove_prefix(1);
        }
        while (!payload.empty() &&
               (payload.back() == ' ' || payload.back() == '\t' || payload.back() == '\r')) {
            payload.remove_suffix(1);
        }
        if (payload.empty()) {
            return;
        }

        if (payload == "[DONE]") {
            saw_done = true;
            if (consumer) {
                consumer(StreamChunk{.text_delta = {}, .done = true});
            }
            return;
        }

        if (payload.front() != '{') {
            fail("Malformed LM Studio SSE data payload.");
            return;
        }
        if (payload.back() != '}') {
            fail("Malformed LM Studio SSE data payload.");
            return;
        }

        if (const auto error = extract_nested_error_message(payload)) {
            fail(*error);
            return;
        }

        if (const auto event_model = extract_json_string_value(payload, "model")) {
            model = unescape_json_string(*event_model);
        }
        if (const auto tokens = extract_json_number_value(payload, "prompt_tokens")) {
            prompt_tokens = *tokens;
        }
        if (const auto tokens = extract_json_number_value(payload, "completion_tokens")) {
            completion_tokens = *tokens;
        }

        if (const auto delta = extract_choice_delta_content(payload)) {
            if (!delta->empty()) {
                accumulated += *delta;
                if (consumer) {
                    consumer(StreamChunk{
                        .text_delta = *delta,
                        .done = false,
                    });
                }
            }
        }
    };

    auto process_line = [&](std::string_view line) {
        if (stream_error.has_value() || saw_done) {
            return;
        }

        while (!line.empty() && line.back() == '\r') {
            line.remove_suffix(1);
        }

        if (line.empty()) {
            if (!data_payload.empty()) {
                process_data_payload(data_payload);
                data_payload.clear();
            }
            return;
        }

        if (line.starts_with(':')) {
            return;
        }

        if (line.starts_with("data:")) {
            auto value = line.substr(5);
            if (!value.empty() && value.front() == ' ') {
                value.remove_prefix(1);
            }
            if (data_payload.empty()) {
                data_payload = std::string{value};
            } else {
                data_payload.push_back('\n');
                data_payload.append(value);
            }
            return;
        }
    };

    auto feed = [&](std::string_view data) {
        if (stream_error.has_value()) {
            return;
        }
        line_buffer.append(data);
        while (true) {
            const auto newline = line_buffer.find('\n');
            if (newline == std::string::npos) {
                break;
            }
            process_line(std::string_view{line_buffer}.substr(0, newline));
            line_buffer.erase(0, newline + 1);
            if (stream_error.has_value() || saw_done) {
                return;
            }
        }
    };

    const auto http_response = transport_->send_stream(http_request, feed);
    if (!http_response) {
        return std::unexpected(ProviderError{
            .provider_id = id().value,
            .message = http_response.error().message,
        });
    }

    if (stream_error.has_value()) {
        return std::unexpected(*stream_error);
    }

    if (!line_buffer.empty()) {
        process_line(line_buffer);
        line_buffer.clear();
        if (stream_error.has_value()) {
            return std::unexpected(*stream_error);
        }
    }
    if (!data_payload.empty() && !saw_done && !stream_error.has_value()) {
        process_data_payload(data_payload);
        data_payload.clear();
    }

    if (stream_error.has_value()) {
        return std::unexpected(*stream_error);
    }

    if (http_response->status_code < 200 || http_response->status_code >= 300) {
        const auto parsed_error = parse_lm_studio_chat_response(http_response->body);
        if (parsed_error.error.has_value()) {
            return std::unexpected(ProviderError{
                .provider_id = id().value,
                .message = *parsed_error.error,
            });
        }
        return std::unexpected(ProviderError{
            .provider_id = id().value,
            .message = std::format(
                "LM Studio request failed with HTTP {}.",
                http_response->status_code),
        });
    }

    if (!saw_done) {
        return std::unexpected(ProviderError{
            .provider_id = id().value,
            .message = "LM Studio stream ended without a terminal [DONE] event.",
        });
    }

    const auto& fallback_model = request.options.model;
    return AIResponse{
        .generated_text = accumulated,
        .provider_id = id().value,
        .model = model.empty() ? std::optional<std::string>{fallback_model}
                               : std::optional<std::string>{model},
        .prompt_eval_count = prompt_tokens,
        .eval_count = completion_tokens,
    };
}

}  // namespace eve::ai
