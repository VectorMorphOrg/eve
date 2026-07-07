#include "eve/ai/http_transport.hpp"

#include <format>
#include <optional>
#include <sstream>
#include <string_view>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

namespace eve::ai {
namespace {

struct ParsedUrl {
    std::string host;
    std::string port;
    std::string path;
};

std::optional<ParsedUrl> parse_http_url(std::string_view url) {
    constexpr std::string_view prefix = "http://";
    if (!url.starts_with(prefix)) {
        return std::nullopt;
    }

    url.remove_prefix(prefix.size());
    const auto path_start = url.find('/');
    const std::string_view authority = path_start == std::string_view::npos ? url : url.substr(0, path_start);
    const std::string_view path =
        path_start == std::string_view::npos ? "/" : url.substr(path_start);

    ParsedUrl parsed{
        .host = std::string{authority},
        .port = "80",
        .path = std::string{path.empty() ? "/" : path},
    };

    const auto colon = authority.find(':');
    if (colon != std::string_view::npos) {
        parsed.host = std::string{authority.substr(0, colon)};
        parsed.port = std::string{authority.substr(colon + 1)};
    }

    if (parsed.host.empty()) {
        return std::nullopt;
    }
    return parsed;
}

#ifdef _WIN32
struct WinsockInitializer {
    WinsockInitializer() {
        WSADATA data{};
        WSAStartup(MAKEWORD(2, 2), &data);
    }

    ~WinsockInitializer() { WSACleanup(); }
};

using native_socket = SOCKET;
constexpr native_socket invalid_socket = INVALID_SOCKET;

void close_socket(native_socket socket) {
    closesocket(socket);
}

std::expected<void, HttpError> set_timeouts(native_socket socket, std::chrono::milliseconds timeout) {
    const DWORD timeout_ms = static_cast<DWORD>(timeout.count());
    if (setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeout_ms), sizeof(timeout_ms)) !=
        0) {
        return std::unexpected(HttpError{"Failed to configure socket receive timeout."});
    }
    if (setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>(&timeout_ms), sizeof(timeout_ms)) !=
        0) {
        return std::unexpected(HttpError{"Failed to configure socket send timeout."});
    }
    return {};
}

std::expected<void, HttpError> send_all(native_socket socket, std::string_view data) {
    std::size_t sent = 0;
    while (sent < data.size()) {
        const int result = send(
            socket,
            data.data() + sent,
            static_cast<int>(data.size() - sent),
            0);
        if (result <= 0) {
            return std::unexpected(HttpError{"Failed to send HTTP request."});
        }
        sent += static_cast<std::size_t>(result);
    }
    return {};
}

std::expected<std::string, HttpError> recv_all(native_socket socket) {
    std::string response;
    char buffer[4096];
    while (true) {
        const int received = recv(socket, buffer, sizeof(buffer), 0);
        if (received == 0) {
            break;
        }
        if (received < 0) {
            const int error = WSAGetLastError();
            if (error == WSAETIMEDOUT) {
                return std::unexpected(HttpError{"Request timed out."});
            }
            return std::unexpected(HttpError{"Failed to receive HTTP response."});
        }
        response.append(buffer, static_cast<std::size_t>(received));
    }
    return response;
}
#else
using native_socket = int;
constexpr native_socket invalid_socket = -1;

void close_socket(native_socket socket) {
    close(socket);
}

std::expected<void, HttpError> set_timeouts(native_socket socket, std::chrono::milliseconds timeout) {
    const timeval tv{
        .tv_sec = static_cast<time_t>(timeout.count() / 1000),
        .tv_usec = static_cast<suseconds_t>((timeout.count() % 1000) * 1000),
    };
    if (setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) != 0) {
        return std::unexpected(HttpError{"Failed to configure socket receive timeout."});
    }
    if (setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) != 0) {
        return std::unexpected(HttpError{"Failed to configure socket send timeout."});
    }
    return {};
}

std::expected<void, HttpError> send_all(native_socket socket, std::string_view data) {
    std::size_t sent = 0;
    while (sent < data.size()) {
        const ssize_t result = ::send(
            socket,
            data.data() + sent,
            data.size() - sent,
            0);
        if (result <= 0) {
            return std::unexpected(HttpError{"Failed to send HTTP request."});
        }
        sent += static_cast<std::size_t>(result);
    }
    return {};
}

std::expected<std::string, HttpError> recv_all(native_socket socket) {
    std::string response;
    char buffer[4096];
    while (true) {
        const ssize_t received = recv(socket, buffer, sizeof(buffer), 0);
        if (received == 0) {
            break;
        }
        if (received < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return std::unexpected(HttpError{"Request timed out."});
            }
            return std::unexpected(HttpError{"Failed to receive HTTP response."});
        }
        response.append(buffer, static_cast<std::size_t>(received));
    }
    return response;
}
#endif

std::expected<native_socket, HttpError> connect_socket(
    const ParsedUrl& url,
    std::chrono::milliseconds timeout) {
#ifdef _WIN32
    static WinsockInitializer winsock_initializer;
#endif

    addrinfo hints{
        .ai_family = AF_UNSPEC,
        .ai_socktype = SOCK_STREAM,
    };
    addrinfo* result = nullptr;
    if (getaddrinfo(url.host.c_str(), url.port.c_str(), &hints, &result) != 0 || result == nullptr) {
        return std::unexpected(HttpError{
            std::format("Unable to resolve host '{}'.", url.host),
        });
    }

    native_socket socket = invalid_socket;
    for (addrinfo* iterator = result; iterator != nullptr; iterator = iterator->ai_next) {
        socket = ::socket(iterator->ai_family, iterator->ai_socktype, iterator->ai_protocol);
        if (socket == invalid_socket) {
            continue;
        }
        if (connect(socket, iterator->ai_addr, static_cast<int>(iterator->ai_addrlen)) == 0) {
            break;
        }
        close_socket(socket);
        socket = invalid_socket;
    }
    freeaddrinfo(result);

    if (socket == invalid_socket) {
        return std::unexpected(HttpError{
            std::format("Unable to connect to {}:{}.", url.host, url.port),
        });
    }

    if (auto timeouts = set_timeouts(socket, timeout); !timeouts) {
        close_socket(socket);
        return std::unexpected(timeouts.error());
    }

    return socket;
}

std::string build_request(const HttpRequest& request, const ParsedUrl& url) {
    std::ostringstream stream;
    stream << request.method << ' ' << url.path << " HTTP/1.1\r\n";
    stream << "Host: " << url.host;
    if (url.port != "80") {
        stream << ':' << url.port;
    }
    stream << "\r\n";
    stream << "Content-Type: application/json\r\n";
    stream << "Accept: application/json\r\n";
    stream << "Connection: close\r\n";
    for (const auto& [name, value] : request.headers) {
        stream << name << ": " << value << "\r\n";
    }
    stream << "Content-Length: " << request.body.size() << "\r\n\r\n";
    stream << request.body;
    return stream.str();
}

std::expected<HttpResponse, HttpError> parse_http_response(std::string raw_response) {
    const auto header_end = raw_response.find("\r\n\r\n");
    if (header_end == std::string::npos) {
        return std::unexpected(HttpError{"Malformed HTTP response."});
    }

    const std::string headers = raw_response.substr(0, header_end);
    std::string body = raw_response.substr(header_end + 4);

    const auto status_line_end = headers.find("\r\n");
    if (status_line_end == std::string::npos) {
        return std::unexpected(HttpError{"Malformed HTTP status line."});
    }

    const std::string status_line = headers.substr(0, status_line_end);
    const auto first_space = status_line.find(' ');
    const auto second_space = status_line.find(' ', first_space + 1);
    if (first_space == std::string::npos || second_space == std::string::npos) {
        return std::unexpected(HttpError{"Malformed HTTP status line."});
    }

    const int status_code = std::stoi(status_line.substr(first_space + 1, second_space - first_space - 1));

    std::optional<std::size_t> content_length;
    std::size_t search_start = 0;
    while (search_start < headers.size()) {
        const auto line_end = headers.find("\r\n", search_start);
        const std::string line = headers.substr(
            search_start,
            line_end == std::string::npos ? std::string::npos : line_end - search_start);
        if (line.starts_with("Content-Length:")) {
            content_length = static_cast<std::size_t>(std::stoull(line.substr(15)));
            break;
        }
        if (line_end == std::string::npos) {
            break;
        }
        search_start = line_end + 2;
    }

    if (content_length.has_value() && body.size() > *content_length) {
        body.resize(*content_length);
    }

    return HttpResponse{
        .status_code = status_code,
        .body = std::move(body),
    };
}

}  // namespace

std::expected<HttpResponse, HttpError> SocketHttpTransport::send(const HttpRequest& request) const {
    const auto parsed_url = parse_http_url(request.url);
    if (!parsed_url.has_value()) {
        return std::unexpected(HttpError{"Only http:// URLs are supported."});
    }

    auto socket = connect_socket(*parsed_url, request.timeout);
    if (!socket) {
        return std::unexpected(socket.error());
    }

    const auto payload = build_request(request, *parsed_url);
    if (auto sent = send_all(*socket, payload); !sent) {
        close_socket(*socket);
        return std::unexpected(sent.error());
    }

    auto raw_response = recv_all(*socket);
    close_socket(*socket);
    if (!raw_response) {
        return std::unexpected(raw_response.error());
    }

    return parse_http_response(std::move(*raw_response));
}

}  // namespace eve::ai
