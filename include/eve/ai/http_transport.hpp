#pragma once

#include <chrono>
#include <expected>
#include <functional>
#include <map>
#include <string>
#include <string_view>

namespace eve::ai {

struct HttpError {
    std::string message;
};

struct HttpRequest {
    std::string method{"POST"};
    std::string url;
    std::string body;
    std::map<std::string, std::string> headers;
    std::chrono::milliseconds timeout{std::chrono::seconds{120}};
};

struct HttpResponse {
    int status_code{0};
    std::string body;
};

/// Incremental HTTP response-body bytes.
/// Views are valid only for the duration of the callback; must not be retained.
using HttpBodyConsumer = std::function<void(std::string_view)>;

/// Incremental decoder for Transfer-Encoding: chunked response bodies.
/// Delivers decoded body bytes only (no size lines / framing CRLFs).
class HttpChunkedBodyDecoder {
public:
    explicit HttpChunkedBodyDecoder(HttpBodyConsumer consumer = {});

    /// Feed framed bytes. Returns true when the chunked body is complete.
    [[nodiscard]] std::expected<bool, HttpError> feed(std::string_view data);

    /// Signal peer EOF. Errors if decoding is incomplete.
    [[nodiscard]] std::expected<void, HttpError> finish();

    [[nodiscard]] const std::string& decoded_body() const noexcept { return decoded_body_; }
    [[nodiscard]] bool complete() const noexcept { return state_ == State::Done; }

private:
    enum class State {
        SizeLine,
        Data,
        DataCRLF,
        Trailer,
        Done,
    };

    [[nodiscard]] std::expected<bool, HttpError> process_buffer();
    [[nodiscard]] static std::expected<std::size_t, HttpError> parse_chunk_size(
        std::string_view size_line);

    HttpBodyConsumer consumer_;
    std::string buffer_;
    std::string decoded_body_;
    State state_{State::SizeLine};
    std::size_t pending_data_bytes_{0};
};

class IHttpTransport {
public:
    virtual ~IHttpTransport() = default;

    [[nodiscard]] virtual std::expected<HttpResponse, HttpError> send(
        const HttpRequest& request) const = 0;

    /// Default: completes via send(), then invokes consumer once with the full body.
    /// Concrete transports may override for genuine incremental delivery.
    [[nodiscard]] virtual std::expected<HttpResponse, HttpError> send_stream(
        const HttpRequest& request,
        const HttpBodyConsumer& consumer) const;
};

class SocketHttpTransport final : public IHttpTransport {
public:
    [[nodiscard]] std::expected<HttpResponse, HttpError> send(
        const HttpRequest& request) const override;

    [[nodiscard]] std::expected<HttpResponse, HttpError> send_stream(
        const HttpRequest& request,
        const HttpBodyConsumer& consumer) const override;
};

}  // namespace eve::ai
