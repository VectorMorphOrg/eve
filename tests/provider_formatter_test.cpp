#include "eve/ai/ollama_provider.hpp"
#include "eve/ai/provider_formatting.hpp"
#include "eve/ai/provider_manager.hpp"
#include "eve/context/context_package.hpp"
#include "eve/core/platform_request.hpp"
#include "eve/services/service_implementations.hpp"

#include <gtest/gtest.h>

#include <atomic>
#include <filesystem>
#include <fstream>
#include <functional>
#include <thread>

namespace eve::ai {
namespace {

class MockHttpTransport final : public IHttpTransport {
public:
    std::function<std::expected<HttpResponse, HttpError>(const HttpRequest&)> handler;

    [[nodiscard]] std::expected<HttpResponse, HttpError> send(
        const HttpRequest& request) const override {
        if (!handler) {
            return std::unexpected(HttpError{"Mock transport handler not configured."});
        }
        return handler(request);
    }
};

context::ContextPackage make_test_package(
    std::vector<KnowledgeObject> objects = {},
    std::vector<context::Citation> citations = {},
    context::SystemInstructions system = {.language = "en"}) {
    const auto request = PlatformRequest::create(
        CapabilityId{"CAP-0102"},
        InterfaceType::Internal,
        {{"query", "Platform Request"}});

    if (objects.empty()) {
        KnowledgeObject object;
        object.identity.id = KnowledgeObjectId{"EVE-0010"};
        object.identity.document_identifier = "EVE-0010";
        object.identity.title = "Platform Request Specification";
        object.search.excerpt = "Defines the Platform Request contract.";
        object.content.sections.push_back(
            KnowledgeSection{.title = "Purpose", .content = "Defines request metadata."});
        object.content.headings.push_back(DocumentHeading{.level = 1, .text = "Overview"});
        object.relationships.related_headers = {"include/eve/core/platform_request.hpp"};
        object.relationships.related_sources = {"src/eve/core/platform_request.cpp"};
        object.relationships.related_tests = {"tests/platform_request_test.cpp"};
        object.relationships.next = "EVE-0011";
        objects.push_back(std::move(object));
    }

    if (citations.empty()) {
        citations.push_back(
            context::Citation{.identifier = "EVE-0010", .title = "Platform Request Specification"});
    }

    return context::ContextPackage::create(
        request,
        std::move(objects),
        {context::RepositoryMetadata{.name = "eve"}},
        std::move(citations),
        context::ContextConstraints{},
        std::move(system));
}

ProviderOptions test_options() {
    return ProviderOptions{
        .model = "qwen2.5:14b",
        .temperature = 0.2,
        .top_p = 0.8,
        .context_length = 8192,
    };
}

ProviderMetadata test_metadata() {
    return ProviderMetadata{
        .provider_id = ProviderId{"AI-0100"},
        .provider_name = "Ollama",
        .request_id = RequestId{"REQ-TEST"},
        .package_id = PackageId{"PKG-TEST"},
    };
}

OllamaProviderConfig test_config() {
    return OllamaProviderConfig{
        .base_url = "http://localhost:11434",
        .model = "qwen2.5:14b",
        .temperature = 0.2,
        .top_p = 0.8,
        .context_length = 8192,
        .timeout = std::chrono::milliseconds{1500},
    };
}

TEST(ProviderFormatterTest, GeneratesProviderRequestWithSystemAndContextMessages) {
    const ProviderFormatter formatter;
    const auto package = make_test_package();
    const auto request = formatter.format(
        package,
        ollama_provider_capabilities(),
        test_options(),
        test_metadata());

    ASSERT_GE(request.messages.size(), 2U);
    EXPECT_EQ(request.messages.front().role, ProviderMessageRole::System);
    EXPECT_NE(request.messages.front().content.find("ENGINEERING RULES"), std::string::npos);
    EXPECT_NE(request.messages.front().content.find("Do not invent facts"), std::string::npos);
    EXPECT_EQ(request.messages[1].role, ProviderMessageRole::User);
    EXPECT_NE(request.messages[1].content.find("Platform Request"), std::string::npos);
    EXPECT_EQ(request.messages.back().role, ProviderMessageRole::User);
    EXPECT_NE(request.messages.back().content.find("CONTEXT:"), std::string::npos);
}

TEST(ProviderFormatterTest, PreservesCitationOrdering) {
    const ProviderFormatter formatter;
    auto package = make_test_package(
        {},
        {
            context::Citation{.identifier = "EVE-0001", .title = "First"},
            context::Citation{.identifier = "EVE-0002", .title = "Second"},
        });

    const auto request = formatter.format(
        package,
        ollama_provider_capabilities(),
        test_options(),
        test_metadata());

    const auto& context_message = request.messages.back().content;
    const auto first = context_message.find("EVE-0001");
    const auto second = context_message.find("EVE-0002");
    ASSERT_NE(first, std::string::npos);
    ASSERT_NE(second, std::string::npos);
    EXPECT_LT(first, second);
}

TEST(ProviderFormatterTest, PreservesEvidenceOrdering) {
    KnowledgeObject first;
    first.identity.document_identifier = "EVE-0001";
    first.identity.title = "First Document";

    KnowledgeObject second;
    second.identity.document_identifier = "EVE-0002";
    second.identity.title = "Second Document";

    const ProviderFormatter formatter;
    const auto request = formatter.format(
        make_test_package({first, second}),
        ollama_provider_capabilities(),
        test_options(),
        test_metadata());

    const auto& context_message = request.messages.back().content;
    EXPECT_LT(context_message.find("EVE-0001"), context_message.find("EVE-0002"));
}

TEST(ProviderFormatterTest, DeterministicFormattingProducesIdenticalRequests) {
    const ProviderFormatter formatter;
    const auto package = make_test_package();
    const auto first = formatter.format(
        package,
        ollama_provider_capabilities(),
        test_options(),
        test_metadata());
    const auto second = formatter.format(
        package,
        ollama_provider_capabilities(),
        test_options(),
        test_metadata());

    ASSERT_EQ(first.messages.size(), second.messages.size());
    for (std::size_t index = 0; index < first.messages.size(); ++index) {
        EXPECT_EQ(first.messages[index].role, second.messages[index].role);
        EXPECT_EQ(first.messages[index].content, second.messages[index].content);
    }
}

TEST(ProviderFormatterTest, EmptyContextProducesRequestAndDiagnosticsFriendlyMessages) {
    const ProviderFormatter formatter;
    const auto platform_request = PlatformRequest::create(
        CapabilityId{"CAP-0102"},
        InterfaceType::Internal,
        {{"query", "empty"}});
    const auto package = context::ContextPackage::create(
        platform_request,
        {},
        {},
        {},
        context::ContextConstraints{},
        context::SystemInstructions{},
        context::PackageDiagnostics{{}, {"missing"}, {}});

    const auto request = formatter.format(
        package,
        ollama_provider_capabilities(),
        test_options(),
        test_metadata());

    EXPECT_FALSE(request.messages.empty());
    EXPECT_NE(request.messages.front().content.find("ENGINEERING RULES"), std::string::npos);
}

TEST(ProviderFormatterTest, MinimalContextIncludesUserRequestOnlyInFirstUserMessage) {
    const ProviderFormatter formatter;
    const auto request = formatter.format(
        make_test_package(),
        ollama_provider_capabilities(),
        test_options(),
        test_metadata());

    ASSERT_GE(request.messages.size(), 2U);
    EXPECT_NE(request.messages[1].content.find("query: Platform Request"), std::string::npos);
    EXPECT_EQ(request.messages[1].content.find("CONTEXT:"), std::string::npos);
}

TEST(ProviderFormatterTest, AdaptsWhenSystemPromptsUnsupported) {
    auto capabilities = ollama_provider_capabilities();
    capabilities.supports_system_prompts = false;

    const ProviderFormatter formatter;
    const auto request = formatter.format(
        make_test_package(),
        capabilities,
        test_options(),
        test_metadata());

    EXPECT_EQ(request.messages.size(), 2U);
    EXPECT_EQ(request.messages.front().role, ProviderMessageRole::User);
    EXPECT_NE(request.messages.front().content.find("ENGINEERING RULES"), std::string::npos);
    EXPECT_NE(request.messages.back().content.find("CONTEXT:"), std::string::npos);
}

TEST(ProviderFormatterTest, AdaptsWhenMultipleMessagesUnsupported) {
    auto capabilities = ollama_provider_capabilities();
    capabilities.supports_multiple_messages = false;

    const ProviderFormatter formatter;
    const auto request = formatter.format(
        make_test_package(),
        capabilities,
        test_options(),
        test_metadata());

    EXPECT_EQ(request.messages.size(), 2U);
    EXPECT_EQ(request.messages.front().role, ProviderMessageRole::System);
    EXPECT_NE(request.messages.back().content.find("CONTEXT:"), std::string::npos);
}

TEST(ProviderFormatterTest, PropagatesProviderOptions) {
    const ProviderFormatter formatter;
    auto options = test_options();
    options.max_tokens = 4096;
    options.seed = 42;

    const auto request = formatter.format(
        make_test_package(),
        ollama_provider_capabilities(),
        options,
        test_metadata());

    EXPECT_EQ(request.options.model, "qwen2.5:14b");
    EXPECT_DOUBLE_EQ(request.options.temperature, 0.2);
    EXPECT_DOUBLE_EQ(request.options.top_p, 0.8);
    ASSERT_TRUE(request.options.context_length.has_value());
    EXPECT_EQ(*request.options.context_length, 8192U);
    ASSERT_TRUE(request.options.max_tokens.has_value());
    EXPECT_EQ(*request.options.max_tokens, 4096U);
    ASSERT_TRUE(request.options.seed.has_value());
    EXPECT_EQ(*request.options.seed, 42);
}

TEST(ProviderFormatterTest, IncludesImplementationAndNavigationEvidence) {
    const ProviderFormatter formatter;
    const auto request = formatter.format(
        make_test_package(),
        ollama_provider_capabilities(),
        test_options(),
        test_metadata());

    const auto& context_message = request.messages.back().content;
    EXPECT_NE(context_message.find("Implementation Header:"), std::string::npos);
    EXPECT_NE(context_message.find("Implementation Source:"), std::string::npos);
    EXPECT_NE(context_message.find("Validation Test:"), std::string::npos);
    EXPECT_NE(context_message.find("Next: EVE-0011"), std::string::npos);
}

TEST(ProviderFormatterTest, LargeContextIncludesAllDocumentsAndSections) {
    std::vector<KnowledgeObject> objects;
    for (int index = 0; index < 20; ++index) {
        KnowledgeObject object;
        object.identity.document_identifier = std::format("EVE-{:04d}", index);
        object.identity.title = std::format("Document {}", index);
        object.search.excerpt = std::string(200, static_cast<char>('a' + (index % 26)));
        object.content.sections.push_back(KnowledgeSection{
            .title = std::format("Section {}", index),
            .content = std::string(300, 'x'),
        });
        objects.push_back(std::move(object));
    }

    const ProviderFormatter formatter;
    const auto request = formatter.format(
        make_test_package(std::move(objects)),
        ollama_provider_capabilities(),
        test_options(),
        test_metadata());

    const auto& context_message = request.messages.back().content;
    EXPECT_NE(context_message.find("EVE-0000"), std::string::npos);
    EXPECT_NE(context_message.find("EVE-0019"), std::string::npos);
    EXPECT_LT(context_message.find("EVE-0000"), context_message.find("EVE-0019"));
    EXPECT_GT(context_message.size(), 10000U);
}

TEST(ProviderFormatterTest, ImmutabilityPreservesFormattedRequestAfterCopyMutation) {
    const ProviderFormatter formatter;
    const auto package = make_test_package();
    const auto request = formatter.format(
        package,
        ollama_provider_capabilities(),
        test_options(),
        test_metadata());
    const auto message_count = request.messages.size();
    const auto first_content = request.messages.front().content;

    auto mutated = request;
    mutated.messages.front().content = "changed";

    EXPECT_EQ(request.messages.size(), message_count);
    EXPECT_EQ(request.messages.front().content, first_content);
}

TEST(ProviderFormatterTest, ConcurrentFormattingProducesConsistentResults) {
    const ProviderFormatter formatter;
    const auto package = make_test_package();
    const auto expected = formatter.format(
        package,
        ollama_provider_capabilities(),
        test_options(),
        test_metadata());

    std::atomic<int> mismatches{0};
    std::vector<std::thread> threads;
    for (int index = 0; index < 8; ++index) {
        threads.emplace_back([&]() {
            const auto result = formatter.format(
                package,
                ollama_provider_capabilities(),
                test_options(),
                test_metadata());
            if (result.messages.size() != expected.messages.size() ||
                result.messages.front().content != expected.messages.front().content) {
                mismatches.fetch_add(1);
            }
        });
    }
    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(mismatches.load(), 0);
}

TEST(ProviderFormatterTest, ProviderManagerFormatsBeforeProviderExecution) {
    auto transport = std::make_shared<MockHttpTransport>();
    transport->handler = [](const HttpRequest& request) -> std::expected<HttpResponse, HttpError> {
        EXPECT_NE(request.body.find("\"role\": \"system\""), std::string::npos);
        EXPECT_NE(request.body.find("ENGINEERING RULES"), std::string::npos);
        return HttpResponse{
            .status_code = 200,
            .body = R"({"model":"qwen2.5:14b","message":{"role":"assistant","content":"Formatted response."},"done":true})",
        };
    };

    ProviderManager manager;
    manager.register_provider(std::make_shared<OllamaProvider>(test_config(), transport));
    manager.set_active_provider(ProviderId{"AI-0100"});

    const auto response = manager.generate(make_test_package());
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->generated_text, "Formatted response.");
}

TEST(OllamaProviderTest, RegistersWithProviderManager) {
    ProviderManager manager;
    manager.register_provider(std::make_shared<NullProvider>());
    manager.register_provider(std::make_shared<OllamaProvider>(
        test_config(),
        std::make_shared<MockHttpTransport>()));

    manager.set_active_provider(ProviderId{"AI-0100"});
    const auto active = manager.active_provider();
    ASSERT_NE(active, nullptr);
    EXPECT_EQ(active->id().value, "AI-0100");
    EXPECT_EQ(active->name(), "Ollama");
}

TEST(OllamaProviderTest, LoadsConfigurationDefaults) {
    const services::ConfigurationService configuration(
        std::filesystem::path{"config/does-not-exist.json"});
    const auto config = OllamaProviderConfig::from_configuration(configuration);

    EXPECT_EQ(config.base_url, "http://localhost:11434");
    EXPECT_EQ(config.model, "qwen2.5:14b");
    EXPECT_DOUBLE_EQ(config.temperature, 0.7);
    EXPECT_DOUBLE_EQ(config.top_p, 0.9);
    EXPECT_EQ(config.context_length, 32768U);
    EXPECT_EQ(config.timeout.count(), 120000);
}

TEST(OllamaProviderTest, SerializesProviderRequestToOllamaJson) {
    const ProviderFormatter formatter;
    const auto provider_request = formatter.format(
        make_test_package(),
        ollama_provider_capabilities(),
        test_options(),
        test_metadata());

    const auto json = serialize_ollama_chat_request(provider_request);
    EXPECT_NE(json.find("\"model\": \"qwen2.5:14b\""), std::string::npos);
    EXPECT_NE(json.find("\"stream\": false"), std::string::npos);
    EXPECT_NE(json.find("\"role\": \"system\""), std::string::npos);
    EXPECT_NE(json.find("\"role\": \"user\""), std::string::npos);
    EXPECT_NE(json.find("\"temperature\": 0.2"), std::string::npos);
    EXPECT_NE(json.find("\"top_p\": 0.8"), std::string::npos);
    EXPECT_NE(json.find("\"num_ctx\": 8192"), std::string::npos);
}

TEST(OllamaProviderTest, ParsesSuccessfulChatResponse) {
    const auto parsed = parse_ollama_chat_response(R"({
      "model": "qwen2.5:14b",
      "message": {"role": "assistant", "content": "Platform Request defines request metadata."},
      "done": true,
      "prompt_eval_count": 42,
      "eval_count": 12,
      "total_duration": 987654321
    })");

    EXPECT_FALSE(parsed.error.has_value());
    EXPECT_EQ(parsed.model, "qwen2.5:14b");
    EXPECT_EQ(parsed.content, "Platform Request defines request metadata.");
    ASSERT_TRUE(parsed.prompt_eval_count.has_value());
    ASSERT_TRUE(parsed.eval_count.has_value());
    ASSERT_TRUE(parsed.total_duration_ns.has_value());
    EXPECT_EQ(*parsed.prompt_eval_count, 42U);
    EXPECT_EQ(*parsed.eval_count, 12U);
    EXPECT_EQ(*parsed.total_duration_ns, 987654321);
}

TEST(OllamaProviderTest, GeneratesResponseFromProviderRequest) {
    auto transport = std::make_shared<MockHttpTransport>();
    transport->handler = [](const HttpRequest& request) -> std::expected<HttpResponse, HttpError> {
        EXPECT_EQ(request.method, "POST");
        EXPECT_EQ(request.url, "http://localhost:11434/api/chat");
        EXPECT_NE(request.body.find("qwen2.5:14b"), std::string::npos);
        EXPECT_NE(request.body.find("Platform Request Specification"), std::string::npos);
        return HttpResponse{
            .status_code = 200,
            .body = R"({"model":"qwen2.5:14b","message":{"role":"assistant","content":"Answer from Ollama."},"done":true,"prompt_eval_count":10,"eval_count":4,"total_duration":1000})",
        };
    };

    const ProviderFormatter formatter;
    const auto provider_request = formatter.format(
        make_test_package(),
        ollama_provider_capabilities(),
        test_options(),
        test_metadata());

    OllamaProvider provider(test_config(), transport);
    const auto response = provider.generate(provider_request);
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->generated_text, "Answer from Ollama.");
    EXPECT_EQ(response->provider_id, "AI-0100");
    ASSERT_TRUE(response->prompt_eval_count.has_value());
    EXPECT_EQ(*response->prompt_eval_count, 10U);
}

TEST(OllamaProviderTest, HandlesUnavailableServer) {
    auto transport = std::make_shared<MockHttpTransport>();
    transport->handler = [](const HttpRequest&) -> std::expected<HttpResponse, HttpError> {
        return std::unexpected(HttpError{"Unable to connect to localhost:11434."});
    };

    const ProviderFormatter formatter;
    const auto provider_request = formatter.format(
        make_test_package(),
        ollama_provider_capabilities(),
        test_options(),
        test_metadata());

    OllamaProvider provider(test_config(), transport);
    const auto response = provider.generate(provider_request);
    ASSERT_FALSE(response.has_value());
    EXPECT_NE(response.error().message.find("Unable to connect"), std::string::npos);
}

TEST(OllamaProviderTest, HandlesTimeout) {
    auto transport = std::make_shared<MockHttpTransport>();
    transport->handler = [](const HttpRequest&) -> std::expected<HttpResponse, HttpError> {
        return std::unexpected(HttpError{"Request timed out."});
    };

    const ProviderFormatter formatter;
    const auto provider_request = formatter.format(
        make_test_package(),
        ollama_provider_capabilities(),
        test_options(),
        test_metadata());

    OllamaProvider provider(test_config(), transport);
    const auto response = provider.generate(provider_request);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error().message, "Request timed out.");
}

TEST(OllamaProviderTest, HandlesInvalidModelError) {
    auto transport = std::make_shared<MockHttpTransport>();
    transport->handler = [](const HttpRequest&) -> std::expected<HttpResponse, HttpError> {
        return HttpResponse{
            .status_code = 404,
            .body = R"({"error":"model 'qwen2.5:14b' not found"})",
        };
    };

    const ProviderFormatter formatter;
    const auto provider_request = formatter.format(
        make_test_package(),
        ollama_provider_capabilities(),
        test_options(),
        test_metadata());

    OllamaProvider provider(test_config(), transport);
    const auto response = provider.generate(provider_request);
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error().message, "model 'qwen2.5:14b' not found");
}

TEST(OllamaProviderTest, HealthCheckSucceedsWithMockTransport) {
    auto transport = std::make_shared<MockHttpTransport>();
    transport->handler = [](const HttpRequest& request) -> std::expected<HttpResponse, HttpError> {
        EXPECT_EQ(request.method, "GET");
        EXPECT_EQ(request.url, "http://localhost:11434/api/tags");
        return HttpResponse{
            .status_code = 200,
            .body = R"({"models":[{"name":"qwen2.5:14b"}]})",
        };
    };

    OllamaProvider provider(test_config(), transport);
    const auto health = provider.health_check();
    EXPECT_TRUE(health.has_value());
}

}  // namespace
}  // namespace eve::ai
