#include "eve/ai/lm_studio_provider.hpp"
#include "eve/ai/provider_formatting.hpp"
#include "eve/ai/provider_manager.hpp"
#include "eve/context/context_package.hpp"
#include "eve/core/platform_request.hpp"
#include "eve/services/service_implementations.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace eve::ai {
namespace {

class MockHttpTransport final : public IHttpTransport {
public:
    std::function<std::expected<HttpResponse, HttpError>(const HttpRequest&)> handler;
    mutable HttpRequest last_request{};

    [[nodiscard]] std::expected<HttpResponse, HttpError> send(
        const HttpRequest& request) const override {
        last_request = request;
        if (!handler) {
            return std::unexpected(HttpError{"Mock transport handler not configured."});
        }
        return handler(request);
    }
};

context::ContextPackage make_test_package() {
    const auto request = PlatformRequest::create(
        CapabilityId{"CAP-0102"},
        InterfaceType::Internal,
        {{"query", "Platform Request"}});

    KnowledgeObject object;
    object.identity.id = KnowledgeObjectId{"EVE-0010"};
    object.identity.document_identifier = "EVE-0010";
    object.identity.title = "Platform Request Specification";
    object.search.excerpt = "Defines the Platform Request contract.";

    return context::ContextPackage::create(
        request,
        {std::move(object)},
        {context::RepositoryMetadata{.name = "eve"}},
        {context::Citation{.identifier = "EVE-0010", .title = "Platform Request Specification"}},
        context::ContextConstraints{},
        context::SystemInstructions{.language = "en"});
}

ProviderOptions test_options() {
    return ProviderOptions{
        .model = "local-model",
        .temperature = 0.2,
        .top_p = 0.8,
        .max_tokens = 256,
    };
}

ProviderMetadata test_metadata() {
    return ProviderMetadata{
        .provider_id = ProviderId{"AI-0102"},
        .provider_name = "LM Studio",
        .request_id = RequestId{"REQ-LM"},
        .package_id = PackageId{"PKG-LM"},
    };
}

ProviderRequest make_provider_request() {
    const ProviderFormatter formatter;
    return formatter.format(
        make_test_package(),
        lm_studio_provider_capabilities(),
        test_options(),
        test_metadata());
}

LMStudioProviderConfig test_config() {
    return LMStudioProviderConfig{
        .base_url = "http://localhost:1234",
        .model = "local-model",
        .temperature = 0.2,
        .top_p = 0.8,
        .timeout = std::chrono::milliseconds{1500},
    };
}

TEST(LMStudioProviderTest, IdentityIsAI0102) {
    const LMStudioProvider provider(test_config(), std::make_shared<MockHttpTransport>());
    EXPECT_EQ(provider.id().value, "AI-0102");
    EXPECT_EQ(provider.name(), "LM Studio");
}

TEST(LMStudioProviderTest, AdvertisesStreamingSupported) {
    EXPECT_TRUE(lm_studio_provider_capabilities().supports_streaming);
    const LMStudioProvider provider(test_config(), std::make_shared<MockHttpTransport>());
    EXPECT_TRUE(provider.capabilities().supports_streaming);
}

TEST(LMStudioProviderTest, LoadsConfigurationDefaults) {
    const services::ConfigurationService configuration(
        std::filesystem::path{"config/does-not-exist.json"});
    const auto config = LMStudioProviderConfig::from_configuration(configuration);

    EXPECT_EQ(config.base_url, "http://localhost:1234");
    EXPECT_TRUE(config.model.empty());
    EXPECT_DOUBLE_EQ(config.temperature, 0.7);
    EXPECT_DOUBLE_EQ(config.top_p, 0.9);
    EXPECT_EQ(config.timeout.count(), 120000);
    EXPECT_FALSE(config.api_key.has_value());
}

TEST(LMStudioProviderTest, BuildsChatCompletionsEndpoint) {
    EXPECT_EQ(
        build_lm_studio_chat_url("http://localhost:1234"),
        "http://localhost:1234/v1/chat/completions");
    EXPECT_EQ(
        build_lm_studio_chat_url("http://localhost:1234/"),
        "http://localhost:1234/v1/chat/completions");
}

TEST(LMStudioProviderTest, SerializesSynchronousChatRequest) {
    const auto json = serialize_lm_studio_chat_request(make_provider_request());
    EXPECT_NE(json.find("\"model\": \"local-model\""), std::string::npos);
    EXPECT_NE(json.find("\"stream\": false"), std::string::npos);
    EXPECT_EQ(json.find("\"stream\": true"), std::string::npos);
    EXPECT_NE(json.find("\"role\": \"system\""), std::string::npos);
    EXPECT_NE(json.find("\"role\": \"user\""), std::string::npos);
    EXPECT_NE(json.find("\"temperature\": 0.2"), std::string::npos);
    EXPECT_NE(json.find("\"top_p\": 0.8"), std::string::npos);
    EXPECT_NE(json.find("\"max_tokens\": 256"), std::string::npos);
}

TEST(LMStudioProviderTest, ParsesSuccessfulChatResponse) {
    const auto parsed = parse_lm_studio_chat_response(R"({
      "id": "chatcmpl-1",
      "model": "local-model",
      "choices": [
        {
          "index": 0,
          "message": {"role": "assistant", "content": "Answer from LM Studio."},
          "finish_reason": "stop"
        }
      ],
      "usage": {"prompt_tokens": 11, "completion_tokens": 7, "total_tokens": 18}
    })");

    EXPECT_FALSE(parsed.error.has_value());
    EXPECT_EQ(parsed.model, "local-model");
    EXPECT_EQ(parsed.content, "Answer from LM Studio.");
    ASSERT_TRUE(parsed.prompt_tokens.has_value());
    ASSERT_TRUE(parsed.completion_tokens.has_value());
    EXPECT_EQ(*parsed.prompt_tokens, 11U);
    EXPECT_EQ(*parsed.completion_tokens, 7U);
}

TEST(LMStudioProviderTest, GeneratesResponseFromProviderRequest) {
    auto transport = std::make_shared<MockHttpTransport>();
    transport->handler = [](const HttpRequest& request) -> std::expected<HttpResponse, HttpError> {
        EXPECT_EQ(request.method, "POST");
        EXPECT_EQ(request.url, "http://localhost:1234/v1/chat/completions");
        EXPECT_NE(request.body.find("\"stream\": false"), std::string::npos);
        EXPECT_NE(request.body.find("local-model"), std::string::npos);
        EXPECT_NE(request.body.find("Platform Request"), std::string::npos);
        EXPECT_TRUE(request.headers.find("Authorization") == request.headers.end());
        return HttpResponse{
            .status_code = 200,
            .body = R"({"model":"local-model","choices":[{"message":{"role":"assistant","content":"Answer from LM Studio."}}],"usage":{"prompt_tokens":10,"completion_tokens":4}})",
        };
    };

    LMStudioProvider provider(test_config(), transport);
    const auto response = provider.generate(make_provider_request());
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->generated_text, "Answer from LM Studio.");
    EXPECT_EQ(response->provider_id, "AI-0102");
    ASSERT_TRUE(response->model.has_value());
    EXPECT_EQ(*response->model, "local-model");
    ASSERT_TRUE(response->prompt_eval_count.has_value());
    ASSERT_TRUE(response->eval_count.has_value());
    EXPECT_EQ(*response->prompt_eval_count, 10U);
    EXPECT_EQ(*response->eval_count, 4U);
    EXPECT_FALSE(response->total_duration_ns.has_value());
}

TEST(LMStudioProviderTest, OptionalAuthorizationHeaderWithoutExposingKey) {
    auto transport = std::make_shared<MockHttpTransport>();
    transport->handler = [](const HttpRequest& request) -> std::expected<HttpResponse, HttpError> {
        const auto auth = request.headers.find("Authorization");
        EXPECT_NE(auth, request.headers.end());
        EXPECT_EQ(auth->second, "Bearer secret-test-key");
        return HttpResponse{
            .status_code = 200,
            .body = R"({"choices":[{"message":{"role":"assistant","content":"ok"}}]})",
        };
    };

    auto config = test_config();
    config.api_key = "secret-test-key";
    LMStudioProvider provider(config, transport);
    const auto response = provider.generate(make_provider_request());
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->generated_text, "ok");
    EXPECT_EQ(transport->last_request.headers.at("Authorization"), "Bearer secret-test-key");
}

TEST(LMStudioProviderTest, PropagatesTransportFailure) {
    auto transport = std::make_shared<MockHttpTransport>();
    transport->handler = [](const HttpRequest&) -> std::expected<HttpResponse, HttpError> {
        return std::unexpected(HttpError{"Unable to connect to localhost:1234."});
    };

    LMStudioProvider provider(test_config(), transport);
    const auto response = provider.generate(make_provider_request());
    ASSERT_FALSE(response.has_value());
    EXPECT_NE(response.error().message.find("Unable to connect"), std::string::npos);
}

TEST(LMStudioProviderTest, PropagatesHttpNon2xx) {
    auto transport = std::make_shared<MockHttpTransport>();
    transport->handler = [](const HttpRequest&) -> std::expected<HttpResponse, HttpError> {
        return HttpResponse{.status_code = 503, .body = "{}"};
    };

    LMStudioProvider provider(test_config(), transport);
    const auto response = provider.generate(make_provider_request());
    ASSERT_FALSE(response.has_value());
    EXPECT_NE(response.error().message.find("HTTP 503"), std::string::npos);
}

TEST(LMStudioProviderTest, MalformedJsonBecomesProviderError) {
    auto transport = std::make_shared<MockHttpTransport>();
    transport->handler = [](const HttpRequest&) -> std::expected<HttpResponse, HttpError> {
        return HttpResponse{.status_code = 200, .body = "not-json"};
    };

    LMStudioProvider provider(test_config(), transport);
    const auto response = provider.generate(make_provider_request());
    ASSERT_FALSE(response.has_value());
    EXPECT_NE(response.error().message.find("Malformed"), std::string::npos);
}

TEST(LMStudioProviderTest, MissingChoicesContentBecomesProviderError) {
    auto transport = std::make_shared<MockHttpTransport>();
    transport->handler = [](const HttpRequest&) -> std::expected<HttpResponse, HttpError> {
        return HttpResponse{
            .status_code = 200,
            .body = R"({"model":"local-model","choices":[]})",
        };
    };

    LMStudioProvider provider(test_config(), transport);
    const auto response = provider.generate(make_provider_request());
    ASSERT_FALSE(response.has_value());
    EXPECT_NE(response.error().message.find("missing message content"), std::string::npos);
}

TEST(LMStudioProviderTest, ApplicationErrorBecomesProviderError) {
    auto transport = std::make_shared<MockHttpTransport>();
    transport->handler = [](const HttpRequest&) -> std::expected<HttpResponse, HttpError> {
        return HttpResponse{
            .status_code = 400,
            .body = R"({"error":{"message":"model not loaded","type":"invalid_request_error"}})",
        };
    };

    LMStudioProvider provider(test_config(), transport);
    const auto response = provider.generate(make_provider_request());
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error().message, "model not loaded");
}

TEST(LMStudioProviderTest, ProviderManagerDispatchesToAI0102) {
    auto transport = std::make_shared<MockHttpTransport>();
    transport->handler = [](const HttpRequest& request) -> std::expected<HttpResponse, HttpError> {
        EXPECT_EQ(request.url, "http://localhost:1234/v1/chat/completions");
        return HttpResponse{
            .status_code = 200,
            .body = R"({"choices":[{"message":{"role":"assistant","content":"manager-ok"}}]})",
        };
    };

    ProviderManager manager;
    manager.register_provider(std::make_shared<NullProvider>());
    manager.register_provider(std::make_shared<LMStudioProvider>(test_config(), transport));
    manager.set_active_provider(ProviderId{"AI-0102"});

    const auto active = manager.active_provider();
    ASSERT_NE(active, nullptr);
    EXPECT_EQ(active->id().value, "AI-0102");

    const auto response = manager.generate(make_test_package());
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->generated_text, "manager-ok");
    EXPECT_EQ(response->provider_id, "AI-0102");
}

class SegmentedHttpTransport final : public IHttpTransport {
public:
    std::vector<std::string> segments;
    int status_code{200};
    std::optional<HttpError> transport_error;
    mutable HttpRequest last_request{};
    mutable std::size_t send_calls{0};
    mutable std::size_t send_stream_calls{0};

    [[nodiscard]] std::expected<HttpResponse, HttpError> send(
        const HttpRequest& request) const override {
        ++send_calls;
        last_request = request;
        if (transport_error.has_value()) {
            return std::unexpected(*transport_error);
        }
        std::string body;
        for (const auto& segment : segments) {
            body += segment;
        }
        return HttpResponse{.status_code = status_code, .body = std::move(body)};
    }

    [[nodiscard]] std::expected<HttpResponse, HttpError> send_stream(
        const HttpRequest& request,
        const HttpBodyConsumer& consumer) const override {
        ++send_stream_calls;
        last_request = request;
        if (transport_error.has_value()) {
            return std::unexpected(*transport_error);
        }
        std::string body;
        for (const auto& segment : segments) {
            body += segment;
            if (consumer) {
                consumer(segment);
            }
        }
        return HttpResponse{.status_code = status_code, .body = std::move(body)};
    }
};

TEST(LMStudioStreamingTest, RequestSetsStreamTrue) {
    auto transport = std::make_shared<SegmentedHttpTransport>();
    transport->segments = {
        "data: {\"choices\":[{\"delta\":{\"content\":\"Hi\"}}]}\n\n"
        "data: [DONE]\n\n",
    };

    LMStudioProvider provider(test_config(), transport);
    const auto response =
        provider.generate_stream(make_provider_request(), [](const StreamChunk&) {});
    ASSERT_TRUE(response.has_value());
    EXPECT_NE(transport->last_request.body.find("\"stream\": true"), std::string::npos);
    EXPECT_EQ(transport->last_request.body.find("\"stream\": false"), std::string::npos);
    EXPECT_EQ(transport->last_request.url, "http://localhost:1234/v1/chat/completions");
    EXPECT_EQ(transport->send_stream_calls, 1U);
    EXPECT_EQ(transport->send_calls, 0U);
}

TEST(LMStudioStreamingTest, SynchronousRequestRemainsStreamFalse) {
    auto transport = std::make_shared<SegmentedHttpTransport>();
    transport->segments = {
        R"({"choices":[{"message":{"role":"assistant","content":"sync"}}]})",
    };

    LMStudioProvider provider(test_config(), transport);
    const auto response = provider.generate(make_provider_request());
    ASSERT_TRUE(response.has_value());
    EXPECT_NE(transport->last_request.body.find("\"stream\": false"), std::string::npos);
    EXPECT_EQ(transport->send_calls, 1U);
    EXPECT_EQ(transport->send_stream_calls, 0U);
}

TEST(LMStudioStreamingTest, EmitsOrderedContentChunksAndTerminalDone) {
    auto transport = std::make_shared<SegmentedHttpTransport>();
    transport->segments = {
        "data: {\"model\":\"local-model\",\"choices\":[{\"delta\":{\"content\":\"Hel\"}}]}\n\n",
        "data: {\"choices\":[{\"delta\":{\"content\":\"lo\"}}]}\n\n",
        "data: {\"choices\":[{\"delta\":{\"content\":\"!\"}}],"
        "\"usage\":{\"prompt_tokens\":9,\"completion_tokens\":3}}\n\n",
        "data: [DONE]\n\n",
    };

    std::vector<StreamChunk> chunks;
    LMStudioProvider provider(test_config(), transport);
    const auto response = provider.generate_stream(
        make_provider_request(),
        [&](const StreamChunk& chunk) { chunks.push_back(chunk); });

    ASSERT_TRUE(response.has_value());
    ASSERT_EQ(chunks.size(), 4U);
    EXPECT_EQ(chunks[0].text_delta, "Hel");
    EXPECT_FALSE(chunks[0].done);
    EXPECT_EQ(chunks[1].text_delta, "lo");
    EXPECT_FALSE(chunks[1].done);
    EXPECT_EQ(chunks[2].text_delta, "!");
    EXPECT_FALSE(chunks[2].done);
    EXPECT_TRUE(chunks[3].done);
    EXPECT_TRUE(chunks[3].text_delta.empty());
    EXPECT_TRUE(chunks.back().done);

    std::string concatenated;
    for (const auto& chunk : chunks) {
        concatenated += chunk.text_delta;
    }
    EXPECT_EQ(concatenated, response->generated_text);
    EXPECT_EQ(response->generated_text, "Hello!");
    ASSERT_TRUE(response->model.has_value());
    EXPECT_EQ(*response->model, "local-model");
    ASSERT_TRUE(response->prompt_eval_count.has_value());
    ASSERT_TRUE(response->eval_count.has_value());
    EXPECT_EQ(*response->prompt_eval_count, 9U);
    EXPECT_EQ(*response->eval_count, 3U);
}

TEST(LMStudioStreamingTest, HandlesEventSplitAcrossCallbacks) {
    auto transport = std::make_shared<SegmentedHttpTransport>();
    transport->segments = {
        "data: {\"choices\":[{\"delta\":{\"content\":\"ab",
        "c\"}}]}\n\n"
        "data: [DONE]\n\n",
    };

    LMStudioProvider provider(test_config(), transport);
    const auto response =
        provider.generate_stream(make_provider_request(), [](const StreamChunk&) {});
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->generated_text, "abc");
}

TEST(LMStudioStreamingTest, HandlesLineSplitAcrossCallbacks) {
    auto transport = std::make_shared<SegmentedHttpTransport>();
    transport->segments = {
        "data: {\"choices\":[{\"delta\":{\"content\":\"X\"}}]}\r",
        "\n\n"
        "data: [DONE]\n\n",
    };

    LMStudioProvider provider(test_config(), transport);
    const auto response =
        provider.generate_stream(make_provider_request(), [](const StreamChunk&) {});
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->generated_text, "X");
}

TEST(LMStudioStreamingTest, HandlesMultipleEventsInOneCallback) {
    auto transport = std::make_shared<SegmentedHttpTransport>();
    transport->segments = {
        "data: {\"choices\":[{\"delta\":{\"content\":\"A\"}}]}\n\n"
        "data: {\"choices\":[{\"delta\":{\"content\":\"B\"}}]}\n\n"
        "data: [DONE]\n\n",
    };

    std::vector<StreamChunk> chunks;
    LMStudioProvider provider(test_config(), transport);
    const auto response = provider.generate_stream(
        make_provider_request(),
        [&](const StreamChunk& chunk) { chunks.push_back(chunk); });
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->generated_text, "AB");
    ASSERT_GE(chunks.size(), 3U);
    EXPECT_TRUE(chunks.back().done);
}

TEST(LMStudioStreamingTest, MetadataOnlyEventsDoNotManufactureText) {
    auto transport = std::make_shared<SegmentedHttpTransport>();
    transport->segments = {
        "data: {\"choices\":[{\"delta\":{\"role\":\"assistant\"}}]}\n\n",
        "data: {\"choices\":[{\"delta\":{\"content\":\"ok\"}}]}\n\n",
        "data: {\"choices\":[{\"delta\":{},\"finish_reason\":\"stop\"}]}\n\n",
        "data: [DONE]\n\n",
    };

    std::vector<StreamChunk> chunks;
    LMStudioProvider provider(test_config(), transport);
    const auto response = provider.generate_stream(
        make_provider_request(),
        [&](const StreamChunk& chunk) { chunks.push_back(chunk); });
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->generated_text, "ok");
    ASSERT_EQ(chunks.size(), 2U);
    EXPECT_EQ(chunks[0].text_delta, "ok");
    EXPECT_FALSE(chunks[0].done);
    EXPECT_TRUE(chunks[1].done);
}

TEST(LMStudioStreamingTest, EmptyValidStreamSucceeds) {
    auto transport = std::make_shared<SegmentedHttpTransport>();
    transport->segments = {"data: [DONE]\n\n"};

    std::vector<StreamChunk> chunks;
    LMStudioProvider provider(test_config(), transport);
    const auto response = provider.generate_stream(
        make_provider_request(),
        [&](const StreamChunk& chunk) { chunks.push_back(chunk); });
    ASSERT_TRUE(response.has_value());
    EXPECT_TRUE(response->generated_text.empty());
    ASSERT_EQ(chunks.size(), 1U);
    EXPECT_TRUE(chunks[0].done);
    EXPECT_TRUE(chunks[0].text_delta.empty());
}

TEST(LMStudioStreamingTest, MissingDoneProducesProviderError) {
    auto transport = std::make_shared<SegmentedHttpTransport>();
    transport->segments = {
        "data: {\"choices\":[{\"delta\":{\"content\":\"partial\"}}]}\n\n",
    };

    LMStudioProvider provider(test_config(), transport);
    const auto response =
        provider.generate_stream(make_provider_request(), [](const StreamChunk&) {});
    ASSERT_FALSE(response.has_value());
    EXPECT_NE(response.error().message.find("[DONE]"), std::string::npos);
}

TEST(LMStudioStreamingTest, MalformedSseJsonProducesProviderError) {
    auto transport = std::make_shared<SegmentedHttpTransport>();
    transport->segments = {"data: {not-json\n\n"};

    LMStudioProvider provider(test_config(), transport);
    const auto response =
        provider.generate_stream(make_provider_request(), [](const StreamChunk&) {});
    ASSERT_FALSE(response.has_value());
    EXPECT_NE(response.error().message.find("Malformed"), std::string::npos);
}

TEST(LMStudioStreamingTest, TransportFailureProducesProviderError) {
    auto transport = std::make_shared<SegmentedHttpTransport>();
    transport->transport_error = HttpError{"Unable to connect to localhost:1234."};

    int callbacks = 0;
    LMStudioProvider provider(test_config(), transport);
    const auto response = provider.generate_stream(
        make_provider_request(),
        [&](const StreamChunk&) { ++callbacks; });
    ASSERT_FALSE(response.has_value());
    EXPECT_NE(response.error().message.find("Unable to connect"), std::string::npos);
    EXPECT_EQ(callbacks, 0);
}

TEST(LMStudioStreamingTest, ApplicationErrorProducesProviderError) {
    auto transport = std::make_shared<SegmentedHttpTransport>();
    transport->segments = {
        "data: {\"error\":{\"message\":\"context overflow\"}}\n\n",
    };

    LMStudioProvider provider(test_config(), transport);
    const auto response =
        provider.generate_stream(make_provider_request(), [](const StreamChunk&) {});
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error().message, "context overflow");
}

TEST(LMStudioStreamingTest, AuthorizationHeaderRemainsCorrect) {
    auto transport = std::make_shared<SegmentedHttpTransport>();
    transport->segments = {
        "data: {\"choices\":[{\"delta\":{\"content\":\"x\"}}]}\n\n"
        "data: [DONE]\n\n",
    };

    auto config = test_config();
    config.api_key = "secret-stream-key";
    LMStudioProvider provider(config, transport);
    const auto response =
        provider.generate_stream(make_provider_request(), [](const StreamChunk&) {});
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(
        transport->last_request.headers.at("Authorization"),
        "Bearer secret-stream-key");
}

TEST(LMStudioStreamingTest, ProviderManagerDispatchesStreaming) {
    auto transport = std::make_shared<SegmentedHttpTransport>();
    transport->segments = {
        "data: {\"choices\":[{\"delta\":{\"content\":\"mgr\"}}]}\n\n"
        "data: [DONE]\n\n",
    };

    ProviderManager manager;
    manager.register_provider(std::make_shared<LMStudioProvider>(test_config(), transport));
    manager.set_active_provider(ProviderId{"AI-0102"});

    std::vector<StreamChunk> chunks;
    const auto response = manager.generate_stream(
        make_test_package(),
        [&](const StreamChunk& chunk) { chunks.push_back(chunk); });
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->generated_text, "mgr");
    ASSERT_GE(chunks.size(), 2U);
    EXPECT_TRUE(chunks.back().done);
    EXPECT_EQ(transport->send_stream_calls, 1U);
}

}  // namespace
}  // namespace eve::ai
