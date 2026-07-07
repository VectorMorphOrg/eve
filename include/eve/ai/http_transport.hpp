#pragma once

#include <chrono>
#include <expected>
#include <map>
#include <string>

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

class IHttpTransport {
public:
    virtual ~IHttpTransport() = default;

    [[nodiscard]] virtual std::expected<HttpResponse, HttpError> send(
        const HttpRequest& request) const = 0;
};

class SocketHttpTransport final : public IHttpTransport {
public:
    [[nodiscard]] std::expected<HttpResponse, HttpError> send(
        const HttpRequest& request) const override;
};

}  // namespace eve::ai
