#include "eve/ai/ollama_provider.hpp"
#include "eve/ai/provider_formatting.hpp"
#include "eve/context/context_package.hpp"
#include "eve/core/platform_request.hpp"

#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <vector>

namespace eve::ai {
namespace {

class SegmentedHttpTransport final : public IHttpTransport {
public:
    std::vector<std::string> segments;
    int status_code{200};
    std::optional<HttpError> transport_error;
    mutable std::string last_request_body;
    mutable std::size_t send_calls{0};
    mutable std::size_t send_stream_calls{0};

    [[nodiscard]] std::expected<HttpResponse, HttpError> send(
        const HttpRequest& request) const override {
        ++send_calls;
        last_request_body = request.body;
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
        last_request_body = request.body;
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

ProviderRequest make_ollama_stream_request() {
    const auto platform_request = PlatformRequest::create(
        CapabilityId{"CAP-0102"},
        InterfaceType::Internal,
        {{"query", "Platform Request"}});

    KnowledgeObject object;
    object.identity.id = KnowledgeObjectId{"EVE-0010"};
    object.identity.document_identifier = "EVE-0010";
    object.identity.title = "Platform Request Specification";

    const auto package = context::ContextPackage::create(
        platform_request,
        {std::move(object)},
        {context::RepositoryMetadata{.name = "eve"}},
        {context::Citation{.identifier = "EVE-0010", .title = "Platform Request Specification"}},
        context::ContextConstraints{},
        context::SystemInstructions{.language = "en"});

    const ProviderFormatter formatter;
    return formatter.format(
        package,
        ollama_provider_capabilities(),
        ProviderOptions{
            .model = "qwen2.5:14b",
            .temperature = 0.2,
            .top_p = 0.8,
            .context_length = 8192,
        },
        ProviderMetadata{
            .provider_id = ProviderId{"AI-0100"},
            .provider_name = "Ollama",
            .request_id = RequestId{"REQ-OLLAMA-STREAM"},
            .package_id = PackageId{"PKG-OLLAMA-STREAM"},
        });
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

TEST(OllamaStreamingTest, AdvertisesSupportsStreaming) {
    EXPECT_TRUE(ollama_provider_capabilities().supports_streaming);
    const OllamaProvider provider(test_config(), std::make_shared<SegmentedHttpTransport>());
    EXPECT_TRUE(provider.capabilities().supports_streaming);
}

TEST(OllamaStreamingTest, RequestSetsStreamTrue) {
    auto transport = std::make_shared<SegmentedHttpTransport>();
    transport->segments = {
        R"({"model":"qwen2.5:14b","message":{"role":"assistant","content":"Hi"},"done":false})"
        "\n"
        R"({"model":"qwen2.5:14b","message":{"role":"assistant","content":""},"done":true,"prompt_eval_count":1,"eval_count":2,"total_duration":3})"
        "\n",
    };

    OllamaProvider provider(test_config(), transport);
    const auto response = provider.generate_stream(make_ollama_stream_request(), [](const StreamChunk&) {});
    ASSERT_TRUE(response.has_value());
    EXPECT_NE(transport->last_request_body.find("\"stream\": true"), std::string::npos);
    EXPECT_EQ(transport->last_request_body.find("\"stream\": false"), std::string::npos);
    EXPECT_EQ(transport->send_stream_calls, 1U);
    EXPECT_EQ(transport->send_calls, 0U);
}

TEST(OllamaStreamingTest, EmitsIncrementalChunksWithTerminalDone) {
    auto transport = std::make_shared<SegmentedHttpTransport>();
    transport->segments = {
        R"({"message":{"role":"assistant","content":"Hel"},"done":false})" "\n",
        R"({"message":{"role":"assistant","content":"lo"},"done":false})" "\n",
        R"({"model":"qwen2.5:14b","message":{"role":"assistant","content":"!"},"done":true,"prompt_eval_count":10,"eval_count":4,"total_duration":999})"
        "\n",
    };

    std::vector<StreamChunk> chunks;
    OllamaProvider provider(test_config(), transport);
    const auto response = provider.generate_stream(
        make_ollama_stream_request(),
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

    std::size_t done_count = 0;
    for (const auto& chunk : chunks) {
        if (chunk.done) {
            ++done_count;
        }
    }
    EXPECT_EQ(done_count, 1U);
    EXPECT_TRUE(chunks.back().done);

    std::string concatenated;
    for (const auto& chunk : chunks) {
        concatenated += chunk.text_delta;
    }
    EXPECT_EQ(concatenated, response->generated_text);
    EXPECT_EQ(response->generated_text, "Hello!");
    ASSERT_TRUE(response->model.has_value());
    EXPECT_EQ(*response->model, "qwen2.5:14b");
    ASSERT_TRUE(response->prompt_eval_count.has_value());
    ASSERT_TRUE(response->eval_count.has_value());
    ASSERT_TRUE(response->total_duration_ns.has_value());
    EXPECT_EQ(*response->prompt_eval_count, 10U);
    EXPECT_EQ(*response->eval_count, 4U);
    EXPECT_EQ(*response->total_duration_ns, 999);
}

TEST(OllamaStreamingTest, HandlesJsonSplitAcrossTransportCallbacks) {
    auto transport = std::make_shared<SegmentedHttpTransport>();
    transport->segments = {
        R"({"message":{"role":"assistant","content":"ab")",
        R"(},"done":false})" "\n",
        R"({"message":{"role":"assistant","content":"c"},"done":true})" "\n",
    };

    std::string concatenated;
    OllamaProvider provider(test_config(), transport);
    const auto response = provider.generate_stream(
        make_ollama_stream_request(),
        [&](const StreamChunk& chunk) { concatenated += chunk.text_delta; });

    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(concatenated, "abc");
    EXPECT_EQ(response->generated_text, "abc");
}

TEST(OllamaStreamingTest, HandlesMultipleRecordsInOneCallback) {
    auto transport = std::make_shared<SegmentedHttpTransport>();
    transport->segments = {
        std::string(R"({"message":{"role":"assistant","content":"A"},"done":false})") + "\n" +
            R"({"message":{"role":"assistant","content":"B"},"done":false})" + "\n" +
            R"({"message":{"role":"assistant","content":""},"done":true})" + "\n",
    };

    std::vector<StreamChunk> chunks;
    OllamaProvider provider(test_config(), transport);
    const auto response = provider.generate_stream(
        make_ollama_stream_request(),
        [&](const StreamChunk& chunk) { chunks.push_back(chunk); });

    ASSERT_TRUE(response.has_value());
    ASSERT_GE(chunks.size(), 3U);
    EXPECT_EQ(response->generated_text, "AB");
    EXPECT_TRUE(chunks.back().done);
}

TEST(OllamaStreamingTest, HandlesNewlineSplitAcrossCallbacks) {
    auto transport = std::make_shared<SegmentedHttpTransport>();
    transport->segments = {
        R"({"message":{"role":"assistant","content":"X"},"done":false})" "\r",
        "\n"
        R"({"message":{"role":"assistant","content":"Y"},"done":true})" "\n",
    };

    OllamaProvider provider(test_config(), transport);
    const auto response = provider.generate_stream(
        make_ollama_stream_request(),
        [](const StreamChunk&) {});
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->generated_text, "XY");
}

TEST(OllamaStreamingTest, PropagatesTransportFailure) {
    auto transport = std::make_shared<SegmentedHttpTransport>();
    transport->transport_error = HttpError{"Unable to connect to localhost:11434."};

    int callback_calls = 0;
    OllamaProvider provider(test_config(), transport);
    const auto response = provider.generate_stream(
        make_ollama_stream_request(),
        [&](const StreamChunk&) { ++callback_calls; });

    ASSERT_FALSE(response.has_value());
    EXPECT_NE(response.error().message.find("Unable to connect"), std::string::npos);
    EXPECT_EQ(callback_calls, 0);
}

TEST(OllamaStreamingTest, MalformedNdjsonProducesProviderError) {
    auto transport = std::make_shared<SegmentedHttpTransport>();
    transport->segments = {"not-json\n"};

    OllamaProvider provider(test_config(), transport);
    const auto response = provider.generate_stream(
        make_ollama_stream_request(),
        [](const StreamChunk&) {});
    ASSERT_FALSE(response.has_value());
    EXPECT_NE(response.error().message.find("Malformed"), std::string::npos);
}

TEST(OllamaStreamingTest, ApplicationErrorPropagates) {
    auto transport = std::make_shared<SegmentedHttpTransport>();
    transport->segments = {R"({"error":"model not found"})" "\n"};

    OllamaProvider provider(test_config(), transport);
    const auto response = provider.generate_stream(
        make_ollama_stream_request(),
        [](const StreamChunk&) {});
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error().message, "model not found");
}

TEST(OllamaStreamingTest, EmptyValidStreamSucceeds) {
    auto transport = std::make_shared<SegmentedHttpTransport>();
    transport->segments = {
        R"({"model":"qwen2.5:14b","message":{"role":"assistant","content":""},"done":true,"eval_count":0})"
        "\n",
    };

    std::vector<StreamChunk> chunks;
    OllamaProvider provider(test_config(), transport);
    const auto response = provider.generate_stream(
        make_ollama_stream_request(),
        [&](const StreamChunk& chunk) { chunks.push_back(chunk); });

    ASSERT_TRUE(response.has_value());
    EXPECT_TRUE(response->generated_text.empty());
    ASSERT_EQ(chunks.size(), 1U);
    EXPECT_TRUE(chunks[0].done);
    EXPECT_TRUE(chunks[0].text_delta.empty());
}

TEST(OllamaStreamingTest, SynchronousGenerateRemainsUnchanged) {
    auto transport = std::make_shared<SegmentedHttpTransport>();
    transport->segments = {
        R"({"model":"qwen2.5:14b","message":{"role":"assistant","content":"sync-ok"},"done":true})",
    };

    OllamaProvider provider(test_config(), transport);
    const auto response = provider.generate(make_ollama_stream_request());
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->generated_text, "sync-ok");
    EXPECT_EQ(transport->send_calls, 1U);
    EXPECT_EQ(transport->send_stream_calls, 0U);
    EXPECT_NE(transport->last_request_body.find("\"stream\": false"), std::string::npos);
}

TEST(OllamaStreamingTest, IncompleteStreamWithoutDoneErrors) {
    auto transport = std::make_shared<SegmentedHttpTransport>();
    transport->segments = {
        R"({"message":{"role":"assistant","content":"partial"},"done":false})" "\n",
    };

    OllamaProvider provider(test_config(), transport);
    const auto response = provider.generate_stream(
        make_ollama_stream_request(),
        [](const StreamChunk&) {});
    ASSERT_FALSE(response.has_value());
    EXPECT_NE(response.error().message.find("terminal done"), std::string::npos);
}

}  // namespace
}  // namespace eve::ai
