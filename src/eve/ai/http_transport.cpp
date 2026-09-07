#include "eve/ai/http_transport.hpp"

#include <cctype>
#include <format>
#include <limits>
#include <optional>
#include <sstream>
#include <string_view>
#include <utility>
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

bool ascii_iequals(std::string_view left, std::string_view right) {
    if (left.size() != right.size()) {
        return false;
    }
    for (std::size_t i = 0; i < left.size(); ++i) {
        const auto a = static_cast<unsigned char>(left[i]);
        const auto b = static_cast<unsigned char>(right[i]);
        if (std::tolower(a) != std::tolower(b)) {
            return false;
        }
    }
    return true;
}

struct ParsedHttpHeaders {
    int status_code{0};
    std::optional<std::size_t> content_length;
    bool chunked{false};
};

std::expected<ParsedHttpHeaders, HttpError> parse_http_headers(std::string_view headers) {
    const auto status_line_end = headers.find("\r\n");
    if (status_line_end == std::string_view::npos) {
        return std::unexpected(HttpError{"Malformed HTTP status line."});
    }

    const std::string_view status_line = headers.substr(0, status_line_end);
    const auto first_space = status_line.find(' ');
    const auto second_space = status_line.find(' ', first_space + 1);
    if (first_space == std::string_view::npos || second_space == std::string_view::npos) {
        return std::unexpected(HttpError{"Malformed HTTP status line."});
    }

    ParsedHttpHeaders parsed{
        .status_code = std::stoi(std::string{
            status_line.substr(first_space + 1, second_space - first_space - 1)}),
    };

    std::size_t search_start = 0;
    while (search_start < headers.size()) {
        const auto line_end = headers.find("\r\n", search_start);
        const std::string_view line = headers.substr(
            search_start,
            line_end == std::string_view::npos ? std::string_view::npos : line_end - search_start);

        const auto colon = line.find(':');
        if (colon != std::string_view::npos) {
            const auto name = line.substr(0, colon);
            auto value = line.substr(colon + 1);
            while (!value.empty() && (value.front() == ' ' || value.front() == '\t')) {
                value.remove_prefix(1);
            }

            if (ascii_iequals(name, "Content-Length")) {
                parsed.content_length = static_cast<std::size_t>(std::stoull(std::string{value}));
            } else if (ascii_iequals(name, "Transfer-Encoding")) {
                // Token may include commas; treat presence of "chunked" as chunked mode.
                std::string lowered{value};
                for (char& ch : lowered) {
                    ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
                }
                parsed.chunked = lowered.find("chunked") != std::string::npos;
            }
        }

        if (line_end == std::string_view::npos) {
            break;
        }
        search_start = line_end + 2;
    }

    if (parsed.chunked) {
        // RFC 7230: ignore Content-Length when Transfer-Encoding is present.
        parsed.content_length.reset();
    }

    return parsed;
}

void emit_body_segment(
    std::string& body,
    const HttpBodyConsumer& consumer,
    std::string_view segment,
    std::optional<std::size_t> content_length) {
    if (segment.empty()) {
        return;
    }

    if (content_length.has_value()) {
        const auto remaining = *content_length > body.size() ? *content_length - body.size() : 0;
        if (remaining == 0) {
            return;
        }
        if (segment.size() > remaining) {
            segment = segment.substr(0, remaining);
        }
    }

    body.append(segment);
    if (consumer) {
        consumer(segment);
    }
}

}  // namespace

HttpChunkedBodyDecoder::HttpChunkedBodyDecoder(HttpBodyConsumer consumer)
    : consumer_(std::move(consumer)) {}

std::expected<std::size_t, HttpError> HttpChunkedBodyDecoder::parse_chunk_size(
    std::string_view size_line) {
    const auto ext = size_line.find(';');
    const std::string_view hex = ext == std::string_view::npos ? size_line : size_line.substr(0, ext);
    if (hex.empty()) {
        return std::unexpected(HttpError{"Malformed chunk-size line."});
    }

    std::size_t value = 0;
    for (const char ch : hex) {
        const auto c = static_cast<unsigned char>(ch);
        std::size_t digit = 0;
        if (c >= '0' && c <= '9') {
            digit = static_cast<std::size_t>(c - '0');
        } else if (c >= 'a' && c <= 'f') {
            digit = static_cast<std::size_t>(c - 'a' + 10);
        } else if (c >= 'A' && c <= 'F') {
            digit = static_cast<std::size_t>(c - 'A' + 10);
        } else {
            return std::unexpected(HttpError{"Invalid hexadecimal chunk size."});
        }
        if (value > (std::numeric_limits<std::size_t>::max() - digit) / 16) {
            return std::unexpected(HttpError{"Invalid hexadecimal chunk size."});
        }
        value = value * 16 + digit;
    }
    return value;
}

std::expected<bool, HttpError> HttpChunkedBodyDecoder::process_buffer() {
    while (state_ != State::Done) {
        switch (state_) {
            case State::SizeLine: {
                const auto line_end = buffer_.find("\r\n");
                if (line_end == std::string::npos) {
                    return false;
                }
                const auto size = parse_chunk_size(std::string_view{buffer_}.substr(0, line_end));
                if (!size) {
                    return std::unexpected(size.error());
                }
                buffer_.erase(0, line_end + 2);
                if (*size == 0) {
                    state_ = State::Trailer;
                } else {
                    pending_data_bytes_ = *size;
                    state_ = State::Data;
                }
                break;
            }
            case State::Data: {
                if (buffer_.size() < pending_data_bytes_) {
                    return false;
                }
                const std::string_view decoded{
                    buffer_.data(),
                    pending_data_bytes_,
                };
                decoded_body_.append(decoded);
                if (consumer_) {
                    consumer_(decoded);
                }
                buffer_.erase(0, pending_data_bytes_);
                pending_data_bytes_ = 0;
                state_ = State::DataCRLF;
                break;
            }
            case State::DataCRLF: {
                if (buffer_.size() < 2) {
                    return false;
                }
                if (!buffer_.starts_with("\r\n")) {
                    return std::unexpected(HttpError{"Malformed chunk framing."});
                }
                buffer_.erase(0, 2);
                state_ = State::SizeLine;
                break;
            }
            case State::Trailer: {
                const auto line_end = buffer_.find("\r\n");
                if (line_end == std::string::npos) {
                    return false;
                }
                if (line_end == 0) {
                    buffer_.erase(0, 2);
                    state_ = State::Done;
                    return true;
                }
                // Ignore trailer header line.
                buffer_.erase(0, line_end + 2);
                break;
            }
            case State::Done:
                return true;
        }
    }
    return true;
}

std::expected<bool, HttpError> HttpChunkedBodyDecoder::feed(std::string_view data) {
    if (state_ == State::Done) {
        if (!data.empty()) {
            return std::unexpected(HttpError{"Unexpected data after chunked body."});
        }
        return true;
    }
    buffer_.append(data);
    return process_buffer();
}

std::expected<void, HttpError> HttpChunkedBodyDecoder::finish() {
    if (state_ == State::Done) {
        return {};
    }
    if (buffer_.empty() && state_ == State::SizeLine && decoded_body_.empty()) {
        return std::unexpected(HttpError{"Incomplete chunked HTTP response."});
    }
    return std::unexpected(HttpError{"Incomplete chunked HTTP response."});
}

std::expected<HttpResponse, HttpError> IHttpTransport::send_stream(
    const HttpRequest& request,
    const HttpBodyConsumer& consumer) const {
    auto response = send(request);
    if (!response.has_value()) {
        return response;
    }
    if (consumer) {
        consumer(response->body);
    }
    return response;
}

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

std::expected<HttpResponse, HttpError> SocketHttpTransport::send_stream(
    const HttpRequest& request,
    const HttpBodyConsumer& consumer) const {
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

    std::string header_buffer;
    std::string body;
    bool headers_parsed = false;
    bool chunked = false;
    int status_code = 0;
    std::optional<std::size_t> content_length;
    std::optional<HttpChunkedBodyDecoder> chunked_decoder;
    char buffer[4096];

    auto close_and_error = [&](HttpError error) -> std::expected<HttpResponse, HttpError> {
        close_socket(*socket);
        return std::unexpected(std::move(error));
    };

    while (true) {
#ifdef _WIN32
        const int received = recv(*socket, buffer, sizeof(buffer), 0);
#else
        const ssize_t received = recv(*socket, buffer, sizeof(buffer), 0);
#endif
        if (received == 0) {
            break;
        }
        if (received < 0) {
            close_socket(*socket);
#ifdef _WIN32
            const int error = WSAGetLastError();
            if (error == WSAETIMEDOUT) {
                return std::unexpected(HttpError{"Request timed out."});
            }
#else
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return std::unexpected(HttpError{"Request timed out."});
            }
#endif
            return std::unexpected(HttpError{"Failed to receive HTTP response."});
        }

        const std::string_view chunk{buffer, static_cast<std::size_t>(received)};
        if (!headers_parsed) {
            header_buffer.append(chunk);
            const auto header_end = header_buffer.find("\r\n\r\n");
            if (header_end == std::string::npos) {
                continue;
            }

            const auto parsed_headers = parse_http_headers(
                std::string_view{header_buffer}.substr(0, header_end));
            if (!parsed_headers) {
                return close_and_error(parsed_headers.error());
            }
            status_code = parsed_headers->status_code;
            content_length = parsed_headers->content_length;
            chunked = parsed_headers->chunked;
            headers_parsed = true;

            const std::string_view initial_body =
                std::string_view{header_buffer}.substr(header_end + 4);
            header_buffer.clear();
            header_buffer.shrink_to_fit();

            if (chunked) {
                chunked_decoder.emplace(consumer);
                auto fed = chunked_decoder->feed(initial_body);
                if (!fed) {
                    return close_and_error(fed.error());
                }
                if (*fed) {
                    break;
                }
            } else {
                emit_body_segment(body, consumer, initial_body, content_length);
                if (content_length.has_value() && body.size() >= *content_length) {
                    break;
                }
            }
            continue;
        }

        if (chunked) {
            auto fed = chunked_decoder->feed(chunk);
            if (!fed) {
                return close_and_error(fed.error());
            }
            if (*fed) {
                break;
            }
        } else {
            emit_body_segment(body, consumer, chunk, content_length);
            if (content_length.has_value() && body.size() >= *content_length) {
                break;
            }
        }
    }

    close_socket(*socket);

    if (!headers_parsed) {
        return std::unexpected(HttpError{"Malformed HTTP response."});
    }

    if (chunked) {
        if (!chunked_decoder.has_value()) {
            return std::unexpected(HttpError{"Incomplete chunked HTTP response."});
        }
        if (!chunked_decoder->complete()) {
            if (auto finished = chunked_decoder->finish(); !finished) {
                return std::unexpected(finished.error());
            }
        }
        return HttpResponse{
            .status_code = status_code,
            .body = chunked_decoder->decoded_body(),
        };
    }

    return HttpResponse{
        .status_code = status_code,
        .body = std::move(body),
    };
}

}  // namespace eve::ai
