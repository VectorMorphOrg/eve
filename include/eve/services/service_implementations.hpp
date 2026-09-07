#pragma once

#include "eve/ai/provider_manager.hpp"
#include "eve/knowledge/reasoning_pipeline.hpp"
#include "eve/services/service_interfaces.hpp"

#include <filesystem>
#include <map>
#include <mutex>
#include <unordered_map>

namespace eve::services {

class ConfigurationService final : public IConfigurationService {
public:
    explicit ConfigurationService(std::filesystem::path config_path);

    [[nodiscard]] std::string platform_version() const override;
    [[nodiscard]] std::string specification_version() const override;
    [[nodiscard]] std::string default_repository() const override;
    [[nodiscard]] std::vector<std::filesystem::path> documentation_paths() const override;
    [[nodiscard]] std::string active_ai_provider() const override;
    [[nodiscard]] std::size_t context_limit_chars() const override;
    [[nodiscard]] std::size_t memory_max_recent_messages() const override;
    [[nodiscard]] std::size_t memory_max_conversation_chars() const override;
    [[nodiscard]] std::optional<std::string> get(std::string_view key) const override;

private:
    std::map<std::string, std::string> values_;
    std::vector<std::filesystem::path> documentation_paths_;
};

class LoggingService final : public ILoggingService {
public:
    void info(std::string_view message) override;
    void warn(std::string_view message) override;
    void error(std::string_view message) override;
};

class DocumentationService final : public IDocumentationService {
public:
    explicit DocumentationService(std::shared_ptr<IKnowledgeStore> store);

    [[nodiscard]] std::expected<std::vector<KnowledgeObject>, PlatformError> lookup(
        std::string_view identifier) const override;
    [[nodiscard]] std::expected<std::vector<KnowledgeObject>, PlatformError> list_repository(
        std::string_view repository) const override;

private:
    std::shared_ptr<IKnowledgeStore> store_;
};

class SearchService final : public ISearchService {
public:
    SearchService(
        std::shared_ptr<IKnowledgeStore> store,
        std::shared_ptr<const ReasoningPipeline> reasoning_pipeline);

    [[nodiscard]] std::expected<std::vector<SearchResult>, PlatformError> search(
        std::string_view query,
        std::optional<std::string_view> repository = std::nullopt) const override;

private:
    std::shared_ptr<IKnowledgeStore> store_;
    std::shared_ptr<const ReasoningPipeline> reasoning_pipeline_;
};

class StatusService final : public IStatusService {
public:
    StatusService(
        const IConfigurationService& configuration,
        const IKnowledgeStore& knowledge_store,
        std::shared_ptr<const ai::ProviderManager> provider_manager);

    [[nodiscard]] PlatformResponse report_status(const PlatformRequest& request) const override;

private:
    const IConfigurationService& configuration_;
    const IKnowledgeStore& knowledge_store_;
    std::shared_ptr<const ai::ProviderManager> provider_manager_;
};

class DiagnosticsService final : public IDiagnosticsService {
public:
    explicit DiagnosticsService(const IKnowledgeStore& knowledge_store);

    [[nodiscard]] std::vector<DiagnosticMessage> collect() const override;

private:
    const IKnowledgeStore& knowledge_store_;
};

class ConversationMemoryService final : public IConversationMemoryService {
public:
    static constexpr std::size_t default_max_recent_messages = 12;
    static constexpr std::size_t default_max_conversation_chars = 8000;

    explicit ConversationMemoryService(
        std::size_t max_recent_messages = default_max_recent_messages,
        std::size_t max_conversation_chars = default_max_conversation_chars);

    [[nodiscard]] context::ConversationContext load(
        std::string_view session_id) const override;

    void append(
        std::string_view session_id,
        std::string_view user_text,
        std::string_view assistant_text) override;

    void reset(std::string_view session_id) override;

private:
    [[nodiscard]] static std::size_t conversation_char_count(
        const context::ConversationContext& conversation);
    void apply_retention(context::ConversationContext& conversation) const;

    std::size_t max_recent_messages_;
    std::size_t max_conversation_chars_;
    mutable std::mutex mutex_;
    std::unordered_map<std::string, context::ConversationContext> sessions_;
};

}  // namespace eve::services
