#include "eve/ai/http_transport.hpp"

#include <gtest/gtest.h>

#include <optional>
#include <string>
#include <vector>

namespace eve::ai {
namespace {

class FakeStreamingHttpTransport final : public IHttpTransport {
public:
    std::vector<std::string> body_segments;
    int status_code{200};
    std::optional<HttpError> stream_error;
    std::optional<HttpError> send_error;
    mutable std::size_t send_calls{0};
    mutable std::size_t send_stream_calls{0};

    [[nodiscard]] std::expected<HttpResponse, HttpError> send(
        const HttpRequest& /*request*/) const override {
        ++send_calls;
        if (send_error.has_value()) {
            return std::unexpected(*send_error);
        }
        std::string body;
        for (const auto& segment : body_segments) {
            body += segment;
        }
        return HttpResponse{
            .status_code = status_code,
            .body = std::move(body),
        };
    }

    [[nodiscard]] std::expected<HttpResponse, HttpError> send_stream(
        const HttpRequest& /*request*/,
        const HttpBodyConsumer& consumer) const override {
        ++send_stream_calls;
        if (stream_error.has_value()) {
            return std::unexpected(*stream_error);
        }

        std::string body;
        for (const auto& segment : body_segments) {
            body += segment;
            if (consumer) {
                consumer(segment);
            }
        }

        return HttpResponse{
            .status_code = status_code,
            .body = std::move(body),
        };
    }
};

TEST(HttpTransportStreamingTest, DeliversMultipleOrderedSegments) {
    FakeStreamingHttpTransport transport;
    transport.body_segments = {"alpha", "beta", "gamma"};

    std::vector<std::string> observed;
    bool returned = false;
    const auto response = transport.send_stream(
        HttpRequest{.url = "http://example.test/"},
        [&](std::string_view segment) {
            EXPECT_FALSE(returned);
            observed.emplace_back(segment);
        });
    returned = true;

    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->status_code, 200);
    ASSERT_EQ(observed.size(), 3U);
    EXPECT_EQ(observed[0], "alpha");
    EXPECT_EQ(observed[1], "beta");
    EXPECT_EQ(observed[2], "gamma");
}

TEST(HttpTransportStreamingTest, ReconstructsExactBody) {
    FakeStreamingHttpTransport transport;
    transport.body_segments = {"{\"a\":1}\n", "{\"b\":2}\n", "{\"done\":true}\n"};

    std::string concatenated;
    const auto response = transport.send_stream(
        HttpRequest{.url = "http://example.test/"},
        [&](std::string_view segment) { concatenated.append(segment); });

    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(concatenated, response->body);
    EXPECT_EQ(response->body, "{\"a\":1}\n{\"b\":2}\n{\"done\":true}\n");
}

TEST(HttpTransportStreamingTest, PropagatesTransportErrorsWithoutFabricatingBody) {
    FakeStreamingHttpTransport transport;
    transport.stream_error = HttpError{"stream read failed"};

    int callback_calls = 0;
    const auto response = transport.send_stream(
        HttpRequest{.url = "http://example.test/"},
        [&](std::string_view) { ++callback_calls; });

    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error().message, "stream read failed");
    EXPECT_EQ(callback_calls, 0);
}

TEST(HttpTransportStreamingTest, DefaultSendStreamUsesSynchronousSend) {
    class DefaultOnlyTransport final : public IHttpTransport {
    public:
        mutable std::size_t send_calls{0};

        [[nodiscard]] std::expected<HttpResponse, HttpError> send(
            const HttpRequest& /*request*/) const override {
            ++send_calls;
            return HttpResponse{.status_code = 201, .body = "full-body"};
        }
    };

    DefaultOnlyTransport transport;
    std::vector<std::string> observed;
    bool returned = false;

    const auto response = transport.send_stream(
        HttpRequest{.url = "http://example.test/"},
        [&](std::string_view segment) {
            EXPECT_FALSE(returned);
            observed.emplace_back(segment);
        });
    returned = true;

    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(transport.send_calls, 1U);
    EXPECT_EQ(response->status_code, 201);
    ASSERT_EQ(observed.size(), 1U);
    EXPECT_EQ(observed[0], "full-body");
    EXPECT_EQ(response->body, "full-body");
}

TEST(HttpTransportStreamingTest, SynchronousSendRemainsUnchanged) {
    FakeStreamingHttpTransport transport;
    transport.body_segments = {"one", "two"};
    transport.status_code = 204;

    const auto response = transport.send(HttpRequest{.url = "http://example.test/"});
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(transport.send_calls, 1U);
    EXPECT_EQ(transport.send_stream_calls, 0U);
    EXPECT_EQ(response->status_code, 204);
    EXPECT_EQ(response->body, "onetwo");
}

TEST(HttpTransportStreamingTest, SocketSendRejectsNonHttpUrls) {
    const SocketHttpTransport transport;
    const auto response = transport.send(HttpRequest{
        .method = "GET",
        .url = "https://example.test/",
    });
    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error().message, "Only http:// URLs are supported.");
}

TEST(HttpTransportStreamingTest, SocketSendStreamRejectsNonHttpUrls) {
    const SocketHttpTransport transport;
    int callback_calls = 0;
    const auto response = transport.send_stream(
        HttpRequest{
            .method = "GET",
            .url = "https://example.test/",
        },
        [&](std::string_view) { ++callback_calls; });

    ASSERT_FALSE(response.has_value());
    EXPECT_EQ(response.error().message, "Only http:// URLs are supported.");
    EXPECT_EQ(callback_calls, 0);
}

std::expected<std::string, HttpError> decode_chunked_segments(
    const std::vector<std::string>& segments,
    std::vector<std::string>* observed = nullptr) {
    HttpChunkedBodyDecoder decoder([&](std::string_view decoded) {
        if (observed != nullptr) {
            observed->emplace_back(decoded);
        }
    });

    for (const auto& segment : segments) {
        const auto fed = decoder.feed(segment);
        if (!fed) {
            return std::unexpected(fed.error());
        }
        if (*fed) {
            return decoder.decoded_body();
        }
    }

    if (auto finished = decoder.finish(); !finished) {
        return std::unexpected(finished.error());
    }
    return decoder.decoded_body();
}

TEST(HttpChunkedDecodingTest, SingleChunkExactBody) {
    const auto body = decode_chunked_segments({"5\r\nhello\r\n0\r\n\r\n"});
    ASSERT_TRUE(body.has_value());
    EXPECT_EQ(*body, "hello");
}

TEST(HttpChunkedDecodingTest, MultipleChunksReconstructExactBody) {
    const auto body = decode_chunked_segments({"5\r\nhello\r\n6\r\n world\r\n0\r\n\r\n"});
    ASSERT_TRUE(body.has_value());
    EXPECT_EQ(*body, "hello world");
}

TEST(HttpChunkedDecodingTest, IgnoresChunkExtensions) {
    const auto body = decode_chunked_segments({"5;ext=1\r\nhello\r\n0;last\r\n\r\n"});
    ASSERT_TRUE(body.has_value());
    EXPECT_EQ(*body, "hello");
}

TEST(HttpChunkedDecodingTest, SizeLineSplitAcrossFeeds) {
    const auto body = decode_chunked_segments({"5\r", "\nhello\r\n0\r\n\r\n"});
    ASSERT_TRUE(body.has_value());
    EXPECT_EQ(*body, "hello");
}

TEST(HttpChunkedDecodingTest, ChunkBodySplitAcrossFeeds) {
    const auto body = decode_chunked_segments({"5\r\nhel", "lo\r\n0\r\n\r\n"});
    ASSERT_TRUE(body.has_value());
    EXPECT_EQ(*body, "hello");
}

TEST(HttpChunkedDecodingTest, CrlfSplitAcrossFeeds) {
    const auto body = decode_chunked_segments({"5\r\nhello\r", "\n0\r\n", "\r\n"});
    ASSERT_TRUE(body.has_value());
    EXPECT_EQ(*body, "hello");
}

TEST(HttpChunkedDecodingTest, MultipleChunksInOneFeed) {
    const auto body = decode_chunked_segments({"3\r\nA\nB\r\n3\r\nCDE\r\n0\r\n\r\n"});
    ASSERT_TRUE(body.has_value());
    EXPECT_EQ(*body, "A\nBCDE");
}

TEST(HttpChunkedDecodingTest, ZeroLengthTerminatingChunk) {
    const auto body = decode_chunked_segments({"0\r\n\r\n"});
    ASSERT_TRUE(body.has_value());
    EXPECT_EQ(*body, "");
}

TEST(HttpChunkedDecodingTest, MalformedHexadecimalSizeErrors) {
    const auto body = decode_chunked_segments({"ZZ\r\n"});
    ASSERT_FALSE(body.has_value());
    EXPECT_NE(body.error().message.find("hexadecimal"), std::string::npos);
}

TEST(HttpChunkedDecodingTest, PrematureEofDuringChunkBodyErrors) {
    const auto body = decode_chunked_segments({"5\r\nhel"});
    ASSERT_FALSE(body.has_value());
    EXPECT_NE(body.error().message.find("Incomplete"), std::string::npos);
}

TEST(HttpChunkedDecodingTest, IncompleteTerminatingSequenceErrors) {
    const auto body = decode_chunked_segments({"5\r\nhello\r\n"});
    ASSERT_FALSE(body.has_value());
    EXPECT_NE(body.error().message.find("Incomplete"), std::string::npos);
}

TEST(HttpChunkedDecodingTest, ConsumerReceivesDecodedBytesOnly) {
    std::vector<std::string> observed;
    const auto body = decode_chunked_segments(
        {"5\r\nhello\r\n6\r\n world\r\n0\r\n\r\n"},
        &observed);
    ASSERT_TRUE(body.has_value());
    ASSERT_EQ(observed.size(), 2U);
    EXPECT_EQ(observed[0], "hello");
    EXPECT_EQ(observed[1], " world");
    for (const auto& segment : observed) {
        EXPECT_EQ(segment.find("\r\n"), std::string::npos);
        EXPECT_EQ(segment.find(';'), std::string::npos);
    }
    EXPECT_EQ(*body, "hello world");
}

TEST(HttpChunkedDecodingTest, DecodedBodyEqualsConcatenatedConsumerBytes) {
    std::vector<std::string> observed;
    const auto body = decode_chunked_segments(
        {"a\r\n0123456789\r\n0\r\n\r\n"},
        &observed);
    ASSERT_TRUE(body.has_value());
    std::string concatenated;
    for (const auto& segment : observed) {
        concatenated += segment;
    }
    EXPECT_EQ(concatenated, *body);
    EXPECT_EQ(*body, "0123456789");
}

}  // namespace
}  // namespace eve::ai
