#include "eve/ai/provider_manager.hpp"
#include "eve/capability/handlers/capability_handlers.hpp"
#include "eve/core/core_platform.hpp"
#include "eve/core/platform_request.hpp"
#include "eve/knowledge/in_memory_knowledge_store.hpp"
#include "eve/knowledge/knowledge_object.hpp"
#include "eve/knowledge/reasoning_pipeline.hpp"
#include "eve/services/service_implementations.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <memory>

#ifndef EVE_PROJECT_ROOT
#define EVE_PROJECT_ROOT "."
#endif

namespace eve {
namespace {

bool trace_contains_component(const PlatformResponse& response, std::string_view component) {
    return std::ranges::any_of(response.trace().entries, [&](const TraceEntry& entry) {
        return entry.component == component;
    });
}

bool services_executed_contains(const PlatformResponse& response, std::string_view service) {
    return std::ranges::any_of(response.trace().services_executed, [&](const std::string& value) {
        return value == service;
    });
}

class ConversationCaptureProvider final : public ai::IAIProvider {
public:
    [[nodiscard]] ProviderId id() const override { return ProviderId{"AI-CAPTURE"}; }
    [[nodiscard]] std::string name() const override { return "ConversationCaptureProvider"; }
    [[nodiscard]] ai::ProviderCapabilities capabilities() const override {
        return ai::null_provider_capabilities();
    }
    [[nodiscard]] ai::ProviderOptions default_options() const override { return {}; }

    [[nodiscard]] std::expected<ai::AIResponse, ProviderError> generate(
        const ai::ProviderRequest& request) const override {
        ++generate_calls;
        conversation_present = request.context_package.conversation().has_value();
        conversation_messages.clear();
        knowledge_object_count = request.context_package.knowledge_objects().size();
        if (request.context_package.conversation().has_value()) {
            conversation_messages = request.context_package.conversation()->recent_messages;
        }
        if (fail) {
            return std::unexpected(ProviderError{.message = "forced provider failure"});
        }
        ai::AIResponse response{
            .generated_text = generated_text,
            .provider_id = id().value,
        };
        if (emit_warning) {
            response.warnings.push_back(DiagnosticMessage{
                .level = "warning",
                .message = "forced partial success",
            });
        }
        return response;
    }

    mutable bool conversation_present{false};
    mutable std::vector<context::ConversationTurn> conversation_messages{};
    mutable std::size_t knowledge_object_count{0};
    mutable std::size_t generate_calls{0};
    bool fail{false};
    bool emit_warning{false};
    std::string generated_text{"captured"};
};

struct Cap0102MemoryFixture {
    std::shared_ptr<services::ConversationMemoryService> memory;
    std::shared_ptr<ai::ProviderManager> provider_manager = std::make_shared<ai::ProviderManager>();
    std::shared_ptr<ConversationCaptureProvider> capture =
        std::make_shared<ConversationCaptureProvider>();
    std::shared_ptr<const ReasoningPipeline> reasoning_pipeline;
    std::shared_ptr<InMemoryKnowledgeStore> knowledge_store =
        std::make_shared<InMemoryKnowledgeStore>();
    std::shared_ptr<services::DocumentationService> documentation;
    std::shared_ptr<services::ConfigurationService> configuration;
    std::shared_ptr<services::StatusService> status;
    std::shared_ptr<capability::CapabilityRegistry> registry =
        std::make_shared<capability::CapabilityRegistry>();
    std::unique_ptr<capability::CapabilityEngine> engine;

    explicit Cap0102MemoryFixture(
        std::size_t max_recent_messages =
            services::ConversationMemoryService::default_max_recent_messages,
        std::size_t max_conversation_chars =
            services::ConversationMemoryService::default_max_conversation_chars)
        : memory(std::make_shared<services::ConversationMemoryService>(
              max_recent_messages,
              max_conversation_chars)) {
        configuration = std::make_shared<services::ConfigurationService>(
            std::filesystem::path(EVE_PROJECT_ROOT) / "config/platform.json");

        std::vector<KnowledgeObject> indexed_objects;
        for (const auto& docs_path : configuration->documentation_paths()) {
            auto objects = DocumentIndexer::index_directory(
                std::filesystem::path(EVE_PROJECT_ROOT) / docs_path,
                configuration->default_repository());
            indexed_objects.insert(
                indexed_objects.end(),
                std::make_move_iterator(objects.begin()),
                std::make_move_iterator(objects.end()));
        }
        reasoning_pipeline = ReasoningPipeline::build_shared(std::move(indexed_objects));
        knowledge_store->load(reasoning_pipeline->objects());
        documentation = std::make_shared<services::DocumentationService>(knowledge_store);

        provider_manager->register_provider(capture);
        provider_manager->set_active_provider(capture->id());

        status = std::make_shared<services::StatusService>(
            *configuration,
            *knowledge_store,
            provider_manager);

        capability::handlers::register_documentation_handlers(
            *registry,
            *documentation,
            *status,
            *reasoning_pipeline,
            provider_manager,
            memory,
            configuration->context_limit_chars());
        engine = std::make_unique<capability::CapabilityEngine>(registry);
    }
};

TEST(IntegrationTest, DocumentationSearchEndToEnd) {
    CorePlatform platform = PlatformBootstrap::create(EVE_PROJECT_ROOT);
    const auto request = PlatformRequest::create(
        CapabilityId{"CAP-0102"},
        InterfaceType::Internal,
        {{"query", "Platform Request"}});

    const auto response = platform.process(std::move(request));
    ASSERT_TRUE(response.has_value());
    EXPECT_TRUE(
        response->status() == ResponseStatus::Success ||
        response->status() == ResponseStatus::PartialSuccess);
    EXPECT_NE(response->content().primary.find("Platform Request"), std::string::npos);
    EXPECT_FALSE(response->references().empty());
}

TEST(IntegrationTest, DocumentationSearchUsesReasoningPipeline) {
    CorePlatform platform = PlatformBootstrap::create(EVE_PROJECT_ROOT);
    const auto request = PlatformRequest::create(
        CapabilityId{"CAP-0102"},
        InterfaceType::Internal,
        {{"query", "Platform Request"}});

    const auto response = platform.process(std::move(request));
    ASSERT_TRUE(response.has_value());

    EXPECT_EQ(response->content().structured.at("reasoning_pipeline"), "4.1");
    EXPECT_FALSE(response->content().structured.at("context_package_id").empty());
    EXPECT_GT(std::stoul(response->content().structured.at("search_result_count")), 0U);
    EXPECT_GT(std::stoul(response->content().structured.at("ranked_result_count")), 0U);
    EXPECT_GT(std::stoul(response->content().structured.at("citation_bundle_count")), 0U);

    EXPECT_TRUE(trace_contains_component(*response, "SearchEngine"));
    EXPECT_TRUE(trace_contains_component(*response, "RankingEngine"));
    EXPECT_TRUE(trace_contains_component(*response, "CitationEngine"));
    EXPECT_TRUE(trace_contains_component(*response, "ContextAssembler"));
    EXPECT_TRUE(trace_contains_component(*response, "NullProvider"));

    EXPECT_TRUE(services_executed_contains(*response, "SearchEngine"));
    EXPECT_TRUE(services_executed_contains(*response, "RankingEngine"));
    EXPECT_TRUE(services_executed_contains(*response, "CitationEngine"));
    EXPECT_TRUE(services_executed_contains(*response, "ContextAssembler"));
    EXPECT_TRUE(services_executed_contains(*response, "NullProvider"));
    EXPECT_TRUE(services_executed_contains(*response, "CommandDispatcher"));
    EXPECT_TRUE(services_executed_contains(*response, "CapabilityEngine"));
}

TEST(IntegrationTest, ReasoningPipelineExecutesBeforeNullProvider) {
    CorePlatform platform = PlatformBootstrap::create(EVE_PROJECT_ROOT);
    const auto request = PlatformRequest::create(
        CapabilityId{"search"},
        InterfaceType::Internal,
        {{"query", "Context Package"}});

    const auto response = platform.process(std::move(request));
    ASSERT_TRUE(response.has_value());
    EXPECT_NE(response->content().primary.find("Context package"), std::string::npos);
    EXPECT_TRUE(trace_contains_component(*response, "ContextAssembler"));
    EXPECT_TRUE(trace_contains_component(*response, "NullProvider"));
}

TEST(IntegrationTest, StatusCapabilityViaAlias) {
    CorePlatform platform = PlatformBootstrap::create(EVE_PROJECT_ROOT);
    const auto request = PlatformRequest::create(
        CapabilityId{"status"},
        InterfaceType::Cli);

    const auto response = platform.process(std::move(request));
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->status(), ResponseStatus::Success);
    EXPECT_NE(response->content().primary.find("0.1.0"), std::string::npos);
}

TEST(IntegrationTest, Cap0102WithoutSessionIdLeavesConversationAbsent) {
    Cap0102MemoryFixture fixture;
    const auto request = PlatformRequest::create(
        CapabilityId{"CAP-0102"},
        InterfaceType::Internal,
        {{"query", "Platform Request"}});

    const auto response = fixture.engine->execute(request);
    ASSERT_TRUE(response.has_value());
    EXPECT_FALSE(fixture.capture->conversation_present);
    EXPECT_GT(fixture.capture->knowledge_object_count, 0U);
    EXPECT_TRUE(fixture.memory->load("").recent_messages.empty());
    EXPECT_TRUE(fixture.memory->load("any-session").recent_messages.empty());
}

TEST(IntegrationTest, Cap0102UnknownSessionAttachesEmptyConversation) {
    Cap0102MemoryFixture fixture;
    const auto request = PlatformRequest::create(
        CapabilityId{"CAP-0102"},
        InterfaceType::Internal,
        {{"query", "Platform Request"}},
        {},
        UserInfo{.session_id = "unknown-session"});

    const auto response = fixture.engine->execute(request);
    ASSERT_TRUE(response.has_value());
    EXPECT_TRUE(fixture.capture->conversation_present);
    EXPECT_TRUE(fixture.capture->conversation_messages.empty());

    const auto after = fixture.memory->load("unknown-session");
    ASSERT_EQ(after.recent_messages.size(), 2U);
    EXPECT_EQ(after.recent_messages[0].content, "Platform Request");
    EXPECT_EQ(after.recent_messages[1].content, "captured");
}

TEST(IntegrationTest, Cap0102KnownSessionAttachesLoadedConversationThenPersists) {
    Cap0102MemoryFixture fixture;
    fixture.memory->append("known-session", "prior-user", "prior-assistant");

    const auto request = PlatformRequest::create(
        CapabilityId{"CAP-0102"},
        InterfaceType::Internal,
        {{"query", "Platform Request"}},
        {},
        UserInfo{.session_id = "known-session"});

    const auto response = fixture.engine->execute(request);
    ASSERT_TRUE(response.has_value());
    ASSERT_TRUE(fixture.capture->conversation_present);
    ASSERT_EQ(fixture.capture->conversation_messages.size(), 2U);
    EXPECT_EQ(fixture.capture->conversation_messages[0].role, context::ConversationRole::User);
    EXPECT_EQ(fixture.capture->conversation_messages[0].content, "prior-user");
    EXPECT_EQ(
        fixture.capture->conversation_messages[1].role,
        context::ConversationRole::Assistant);
    EXPECT_EQ(fixture.capture->conversation_messages[1].content, "prior-assistant");
    EXPECT_GT(fixture.capture->knowledge_object_count, 0U);

    const auto after = fixture.memory->load("known-session");
    ASSERT_EQ(after.recent_messages.size(), 4U);
    EXPECT_EQ(after.recent_messages[0].content, "prior-user");
    EXPECT_EQ(after.recent_messages[1].content, "prior-assistant");
    EXPECT_EQ(after.recent_messages[2].content, "Platform Request");
    EXPECT_EQ(after.recent_messages[3].content, "captured");
}

TEST(IntegrationTest, Cap0102EmptySessionIdDisablesMemory) {
    Cap0102MemoryFixture fixture;
    fixture.memory->append("should-not-load", "user", "assistant");

    const auto request = PlatformRequest::create(
        CapabilityId{"CAP-0102"},
        InterfaceType::Internal,
        {{"query", "Platform Request"}},
        {},
        UserInfo{.session_id = ""});

    const auto response = fixture.engine->execute(request);
    ASSERT_TRUE(response.has_value());
    EXPECT_FALSE(fixture.capture->conversation_present);
    ASSERT_EQ(fixture.memory->load("should-not-load").recent_messages.size(), 2U);
}

TEST(IntegrationTest, Cap0102SuccessfulRequestPersistsQueryAndGeneratedText) {
    Cap0102MemoryFixture fixture;
    fixture.capture->generated_text = "assistant-answer";

    const auto request = PlatformRequest::create(
        CapabilityId{"CAP-0102"},
        InterfaceType::Internal,
        {{"query", "Platform Request"}},
        {},
        UserInfo{.session_id = "persist-session"});

    const auto response = fixture.engine->execute(request);
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->status(), ResponseStatus::Success);
    EXPECT_EQ(fixture.capture->generate_calls, 1U);

    const auto stored = fixture.memory->load("persist-session");
    ASSERT_EQ(stored.recent_messages.size(), 2U);
    EXPECT_EQ(stored.recent_messages[0].role, context::ConversationRole::User);
    EXPECT_EQ(stored.recent_messages[0].content, "Platform Request");
    EXPECT_EQ(stored.recent_messages[1].role, context::ConversationRole::Assistant);
    EXPECT_EQ(stored.recent_messages[1].content, "assistant-answer");
}

TEST(IntegrationTest, Cap0102SubsequentRequestReceivesPersistedHistory) {
    Cap0102MemoryFixture fixture;
    fixture.capture->generated_text = "first-answer";

    const auto first = PlatformRequest::create(
        CapabilityId{"CAP-0102"},
        InterfaceType::Internal,
        {{"query", "Platform Request"}},
        {},
        UserInfo{.session_id = "multi-turn"});
    ASSERT_TRUE(fixture.engine->execute(first).has_value());

    fixture.capture->generated_text = "second-answer";
    const auto second = PlatformRequest::create(
        CapabilityId{"CAP-0102"},
        InterfaceType::Internal,
        {{"query", "Context Package"}},
        {},
        UserInfo{.session_id = "multi-turn"});
    ASSERT_TRUE(fixture.engine->execute(second).has_value());

    ASSERT_EQ(fixture.capture->conversation_messages.size(), 2U);
    EXPECT_EQ(fixture.capture->conversation_messages[0].content, "Platform Request");
    EXPECT_EQ(fixture.capture->conversation_messages[1].content, "first-answer");

    const auto stored = fixture.memory->load("multi-turn");
    ASSERT_EQ(stored.recent_messages.size(), 4U);
    EXPECT_EQ(stored.recent_messages[2].content, "Context Package");
    EXPECT_EQ(stored.recent_messages[3].content, "second-answer");
}

TEST(IntegrationTest, Cap0102MissingSessionIdDoesNotPersist) {
    Cap0102MemoryFixture fixture;
    const auto request = PlatformRequest::create(
        CapabilityId{"CAP-0102"},
        InterfaceType::Internal,
        {{"query", "Platform Request"}});

    ASSERT_TRUE(fixture.engine->execute(request).has_value());
    EXPECT_TRUE(fixture.memory->load("").recent_messages.empty());
    EXPECT_TRUE(fixture.memory->load("any").recent_messages.empty());
}

TEST(IntegrationTest, Cap0102ProviderFailureDoesNotPersist) {
    Cap0102MemoryFixture fixture;
    fixture.capture->fail = true;

    const auto request = PlatformRequest::create(
        CapabilityId{"CAP-0102"},
        InterfaceType::Internal,
        {{"query", "Platform Request"}},
        {},
        UserInfo{.session_id = "fail-session"});

    const auto response = fixture.engine->execute(request);
    EXPECT_FALSE(response.has_value());
    EXPECT_EQ(fixture.capture->generate_calls, 1U);
    EXPECT_TRUE(fixture.memory->load("fail-session").recent_messages.empty());
}

TEST(IntegrationTest, Cap0102PartialSuccessPersistsTurn) {
    Cap0102MemoryFixture fixture;
    fixture.capture->emit_warning = true;
    fixture.capture->generated_text = "partial-answer";

    const auto request = PlatformRequest::create(
        CapabilityId{"CAP-0102"},
        InterfaceType::Internal,
        {{"query", "Platform Request"}},
        {},
        UserInfo{.session_id = "partial-session"});

    const auto response = fixture.engine->execute(request);
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->status(), ResponseStatus::PartialSuccess);

    const auto stored = fixture.memory->load("partial-session");
    ASSERT_EQ(stored.recent_messages.size(), 2U);
    EXPECT_EQ(stored.recent_messages[0].content, "Platform Request");
    EXPECT_EQ(stored.recent_messages[1].content, "partial-answer");
}

TEST(IntegrationTest, Cap0102PersistsExactlyOncePerSuccessfulRequest) {
    Cap0102MemoryFixture fixture;
    fixture.capture->generated_text = "once";

    const auto request = PlatformRequest::create(
        CapabilityId{"CAP-0102"},
        InterfaceType::Internal,
        {{"query", "Platform Request"}},
        {},
        UserInfo{.session_id = "once-session"});

    ASSERT_TRUE(fixture.engine->execute(request).has_value());
    EXPECT_EQ(fixture.capture->generate_calls, 1U);
    EXPECT_EQ(fixture.memory->load("once-session").recent_messages.size(), 2U);
}

TEST(IntegrationTest, Cap0102CrossSessionIsolation) {
    Cap0102MemoryFixture fixture;
    fixture.capture->generated_text = "alpha-answer";
    ASSERT_TRUE(fixture.engine
                    ->execute(PlatformRequest::create(
                        CapabilityId{"CAP-0102"},
                        InterfaceType::Internal,
                        {{"query", "Platform Request"}},
                        {},
                        UserInfo{.session_id = "session-alpha"}))
                    .has_value());

    fixture.capture->generated_text = "beta-answer";
    ASSERT_TRUE(fixture.engine
                    ->execute(PlatformRequest::create(
                        CapabilityId{"CAP-0102"},
                        InterfaceType::Internal,
                        {{"query", "Context Package"}},
                        {},
                        UserInfo{.session_id = "session-beta"}))
                    .has_value());

    const auto alpha = fixture.memory->load("session-alpha");
    const auto beta = fixture.memory->load("session-beta");
    ASSERT_EQ(alpha.recent_messages.size(), 2U);
    ASSERT_EQ(beta.recent_messages.size(), 2U);
    EXPECT_EQ(alpha.recent_messages[0].content, "Platform Request");
    EXPECT_EQ(alpha.recent_messages[1].content, "alpha-answer");
    EXPECT_EQ(beta.recent_messages[0].content, "Context Package");
    EXPECT_EQ(beta.recent_messages[1].content, "beta-answer");

    fixture.capture->generated_text = "alpha-second";
    ASSERT_TRUE(fixture.engine
                    ->execute(PlatformRequest::create(
                        CapabilityId{"CAP-0102"},
                        InterfaceType::Internal,
                        {{"query", "Platform Request"}},
                        {},
                        UserInfo{.session_id = "session-alpha"}))
                    .has_value());

    ASSERT_EQ(fixture.capture->conversation_messages.size(), 2U);
    EXPECT_EQ(fixture.capture->conversation_messages[0].content, "Platform Request");
    EXPECT_EQ(fixture.capture->conversation_messages[1].content, "alpha-answer");
    EXPECT_EQ(fixture.memory->load("session-beta").recent_messages.size(), 2U);
    EXPECT_EQ(fixture.memory->load("session-beta").recent_messages[1].content, "beta-answer");
}

TEST(IntegrationTest, Cap0102ResetClearsHistoryForNextRequest) {
    Cap0102MemoryFixture fixture;
    fixture.capture->generated_text = "before-reset";
    ASSERT_TRUE(fixture.engine
                    ->execute(PlatformRequest::create(
                        CapabilityId{"CAP-0102"},
                        InterfaceType::Internal,
                        {{"query", "Platform Request"}},
                        {},
                        UserInfo{.session_id = "reset-session"}))
                    .has_value());
    ASSERT_EQ(fixture.memory->load("reset-session").recent_messages.size(), 2U);

    fixture.memory->reset("reset-session");
    EXPECT_TRUE(fixture.memory->load("reset-session").recent_messages.empty());

    fixture.capture->generated_text = "after-reset";
    ASSERT_TRUE(fixture.engine
                    ->execute(PlatformRequest::create(
                        CapabilityId{"CAP-0102"},
                        InterfaceType::Internal,
                        {{"query", "Context Package"}},
                        {},
                        UserInfo{.session_id = "reset-session"}))
                    .has_value());

    EXPECT_TRUE(fixture.capture->conversation_present);
    EXPECT_TRUE(fixture.capture->conversation_messages.empty());
    const auto after = fixture.memory->load("reset-session");
    ASSERT_EQ(after.recent_messages.size(), 2U);
    EXPECT_EQ(after.recent_messages[0].content, "Context Package");
    EXPECT_EQ(after.recent_messages[1].content, "after-reset");
}

TEST(IntegrationTest, Cap0102RetentionTrimsOldestPairsThroughCapabilityPath) {
    Cap0102MemoryFixture fixture(/*max_recent_messages=*/4);
    fixture.capture->generated_text = "a1";
    ASSERT_TRUE(fixture.engine
                    ->execute(PlatformRequest::create(
                        CapabilityId{"CAP-0102"},
                        InterfaceType::Internal,
                        {{"query", "Platform Request"}},
                        {},
                        UserInfo{.session_id = "retain-session"}))
                    .has_value());
    fixture.capture->generated_text = "a2";
    ASSERT_TRUE(fixture.engine
                    ->execute(PlatformRequest::create(
                        CapabilityId{"CAP-0102"},
                        InterfaceType::Internal,
                        {{"query", "Context Package"}},
                        {},
                        UserInfo{.session_id = "retain-session"}))
                    .has_value());
    fixture.capture->generated_text = "a3";
    ASSERT_TRUE(fixture.engine
                    ->execute(PlatformRequest::create(
                        CapabilityId{"CAP-0102"},
                        InterfaceType::Internal,
                        {{"query", "Platform Request"}},
                        {},
                        UserInfo{.session_id = "retain-session"}))
                    .has_value());

    const auto stored = fixture.memory->load("retain-session");
    ASSERT_EQ(stored.recent_messages.size(), 4U);
    EXPECT_EQ(stored.recent_messages[0].content, "Context Package");
    EXPECT_EQ(stored.recent_messages[1].content, "a2");
    EXPECT_EQ(stored.recent_messages[2].content, "Platform Request");
    EXPECT_EQ(stored.recent_messages[3].content, "a3");
    EXPECT_EQ(stored.recent_messages[0].role, context::ConversationRole::User);
    EXPECT_EQ(stored.recent_messages[1].role, context::ConversationRole::Assistant);
}

TEST(IntegrationTest, Cap0102HistoryDoesNotAlterResponseContentOrStatus) {
    Cap0102MemoryFixture with_history;
    with_history.memory->append("integrity-session", "prior-user", "prior-assistant");
    with_history.capture->generated_text = "integrity-answer";

    Cap0102MemoryFixture without_history;
    without_history.capture->generated_text = "integrity-answer";

    const auto historic = with_history.engine->execute(PlatformRequest::create(
        CapabilityId{"CAP-0102"},
        InterfaceType::Internal,
        {{"query", "Platform Request"}},
        {},
        UserInfo{.session_id = "integrity-session"}));
    const auto baseline = without_history.engine->execute(PlatformRequest::create(
        CapabilityId{"CAP-0102"},
        InterfaceType::Internal,
        {{"query", "Platform Request"}}));

    ASSERT_TRUE(historic.has_value());
    ASSERT_TRUE(baseline.has_value());
    EXPECT_EQ(historic->status(), baseline->status());
    EXPECT_EQ(historic->status(), ResponseStatus::Success);
    EXPECT_NE(historic->content().primary.find("integrity-answer"), std::string::npos);
    EXPECT_EQ(historic->content().primary.find("prior-user"), std::string::npos);
    EXPECT_EQ(historic->content().primary.find("prior-assistant"), std::string::npos);
    EXPECT_EQ(
        historic->content().structured.at("reasoning_pipeline"),
        baseline->content().structured.at("reasoning_pipeline"));
    EXPECT_EQ(
        historic->content().structured.at("search_result_count"),
        baseline->content().structured.at("search_result_count"));
}

TEST(IntegrationTest, Cap0102MultiTurnDoesNotDuplicateCurrentRequestInProviderHistory) {
    Cap0102MemoryFixture fixture;
    fixture.capture->generated_text = "first-answer";
    ASSERT_TRUE(fixture.engine
                    ->execute(PlatformRequest::create(
                        CapabilityId{"CAP-0102"},
                        InterfaceType::Internal,
                        {{"query", "Platform Request"}},
                        {},
                        UserInfo{.session_id = "nodupe-session"}))
                    .has_value());

    fixture.capture->generated_text = "second-answer";
    ASSERT_TRUE(fixture.engine
                    ->execute(PlatformRequest::create(
                        CapabilityId{"CAP-0102"},
                        InterfaceType::Internal,
                        {{"query", "Context Package"}},
                        {},
                        UserInfo{.session_id = "nodupe-session"}))
                    .has_value());

    ASSERT_EQ(fixture.capture->conversation_messages.size(), 2U);
    EXPECT_EQ(fixture.capture->conversation_messages[0].content, "Platform Request");
    EXPECT_EQ(fixture.capture->conversation_messages[1].content, "first-answer");
    for (const auto& turn : fixture.capture->conversation_messages) {
        EXPECT_NE(turn.content, "Context Package");
        EXPECT_NE(turn.content, "second-answer");
    }
}

}  // namespace
}  // namespace eve
