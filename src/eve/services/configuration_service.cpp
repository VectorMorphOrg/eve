#include "eve/services/service_implementations.hpp"

#include <fstream>
#include <map>
#include <sstream>

namespace eve::services {
namespace {

std::string read_file(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

std::string extract_json_string(std::string_view content, std::string_view key) {
    const auto pattern = std::format("\"{}\": \"", key);
    const auto start = content.find(pattern);
    if (start == std::string_view::npos) {
        return {};
    }
    const auto value_start = start + pattern.size();
    const auto value_end = content.find('"', value_start);
    if (value_end == std::string_view::npos) {
        return {};
    }
    return std::string{content.substr(value_start, value_end - value_start)};
}

std::string extract_json_number_string(std::string_view content, std::string_view key) {
    const auto pattern = std::format("\"{}\": ", key);
    const auto start = content.find(pattern);
    if (start == std::string_view::npos) {
        return {};
    }
    const auto value_start = start + pattern.size();
    const auto value_end = content.find_first_of(",}\r\n", value_start);
    if (value_end == std::string_view::npos) {
        return std::string{content.substr(value_start)};
    }
    return std::string{content.substr(value_start, value_end - value_start)};
}

std::size_t extract_json_number(std::string_view content, std::string_view key) {
    const auto pattern = std::format("\"{}\": ", key);
    const auto start = content.find(pattern);
    if (start == std::string_view::npos) {
        return 0;
    }
    const auto value_start = start + pattern.size();
    return static_cast<std::size_t>(std::stoull(std::string{content.substr(value_start)}));
}

}  // namespace

ConfigurationService::ConfigurationService(std::filesystem::path config_path) {
    if (std::filesystem::exists(config_path)) {
        const auto content = read_file(config_path);
        values_["platform_version"] = extract_json_string(content, "platform_version");
        values_["specification_version"] = extract_json_string(content, "specification_version");
        values_["default_repository"] = extract_json_string(content, "default_repository");
        values_["active_ai_provider"] = extract_json_string(content, "active_ai_provider");
        values_["context_limit_chars"] =
            std::to_string(extract_json_number(content, "context_limit_chars"));
        values_["ollama_base_url"] = extract_json_string(content, "ollama_base_url");
        values_["ollama_model"] = extract_json_string(content, "ollama_model");
        values_["ollama_temperature"] = extract_json_number_string(content, "ollama_temperature");
        values_["ollama_top_p"] = extract_json_number_string(content, "ollama_top_p");
        values_["ollama_context_length"] = extract_json_number_string(content, "ollama_context_length");
        values_["ollama_timeout_ms"] = extract_json_number_string(content, "ollama_timeout_ms");

        if (const auto value =
                extract_json_number_string(content, "memory_max_recent_messages");
            !value.empty()) {
            values_["memory_max_recent_messages"] = value;
        }
        if (const auto value =
                extract_json_number_string(content, "memory_max_conversation_chars");
            !value.empty()) {
            values_["memory_max_conversation_chars"] = value;
        }
    }

    if (values_.empty()) {
        values_["platform_version"] = "0.1.0";
        values_["specification_version"] = "1.0.0";
        values_["default_repository"] = "eve";
        values_["active_ai_provider"] = "AI-0000";
        values_["context_limit_chars"] = "32000";
    }

    documentation_paths_.push_back("docs");
}

std::string ConfigurationService::platform_version() const {
    return values_.at("platform_version");
}

std::string ConfigurationService::specification_version() const {
    return values_.at("specification_version");
}

std::string ConfigurationService::default_repository() const {
    return values_.at("default_repository");
}

std::vector<std::filesystem::path> ConfigurationService::documentation_paths() const {
    return documentation_paths_;
}

std::string ConfigurationService::active_ai_provider() const {
    return values_.at("active_ai_provider");
}

std::size_t ConfigurationService::context_limit_chars() const {
    return static_cast<std::size_t>(std::stoull(values_.at("context_limit_chars")));
}

namespace {

std::size_t read_optional_size_t(
    const std::map<std::string, std::string>& values,
    std::string_view key,
    std::size_t fallback) {
    const auto iterator = values.find(std::string{key});
    if (iterator == values.end() || iterator->second.empty()) {
        return fallback;
    }
    try {
        return static_cast<std::size_t>(std::stoull(iterator->second));
    } catch (...) {
        return fallback;
    }
}

}  // namespace

std::size_t ConfigurationService::memory_max_recent_messages() const {
    return read_optional_size_t(
        values_,
        "memory_max_recent_messages",
        ConversationMemoryService::default_max_recent_messages);
}

std::size_t ConfigurationService::memory_max_conversation_chars() const {
    return read_optional_size_t(
        values_,
        "memory_max_conversation_chars",
        ConversationMemoryService::default_max_conversation_chars);
}

std::optional<std::string> ConfigurationService::get(std::string_view key) const {
    const auto iterator = values_.find(std::string{key});
    if (iterator == values_.end()) {
        return std::nullopt;
    }
    return iterator->second;
}

}  // namespace eve::services
