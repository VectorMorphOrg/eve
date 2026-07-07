#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace eve {

using Clock = std::chrono::system_clock;
using TimePoint = Clock::time_point;

struct RequestId {
    std::string value;

    [[nodiscard]] bool empty() const noexcept { return value.empty(); }
    [[nodiscard]] explicit operator bool() const noexcept { return !empty(); }

    friend bool operator==(const RequestId& lhs, const RequestId& rhs) noexcept {
        return lhs.value == rhs.value;
    }
};

struct ResponseId {
    std::string value;

    [[nodiscard]] bool empty() const noexcept { return value.empty(); }
    [[nodiscard]] explicit operator bool() const noexcept { return !empty(); }
};

struct CorrelationId {
    std::string value;
};

struct CapabilityId {
    std::string value;

    [[nodiscard]] bool empty() const noexcept { return value.empty(); }
    [[nodiscard]] explicit operator bool() const noexcept { return !empty(); }

    friend bool operator==(const CapabilityId& lhs, const CapabilityId& rhs) noexcept {
        return lhs.value == rhs.value;
    }
};

struct PackageId {
    std::string value;
};

struct KnowledgeObjectId {
    std::string value;
};

struct ProviderId {
    std::string value;
};

enum class InterfaceType {
    Discord,
    Website,
    Cli,
    RestApi,
    Internal
};

enum class PermissionLevel {
    Public,
    Developer,
    Maintainer,
    Administrator
};

enum class ResponseStatus {
    Success,
    PartialSuccess,
    Warning,
    Failure,
    Unauthorized,
    NotFound,
    Unsupported
};

enum class DocumentType {
    Specification,
    ReferenceCatalog,
    Rfc,
    Whitepaper,
    Readme,
    Guide,
    Changelog,
    Other
};

[[nodiscard]] std::string to_string(InterfaceType type);
[[nodiscard]] std::string to_string(PermissionLevel level);
[[nodiscard]] std::string to_string(ResponseStatus status);
[[nodiscard]] std::string to_string(DocumentType type);

[[nodiscard]] RequestId generate_request_id(TimePoint now = Clock::now());
[[nodiscard]] ResponseId generate_response_id(TimePoint now = Clock::now());
[[nodiscard]] PackageId generate_package_id(TimePoint now = Clock::now());
[[nodiscard]] KnowledgeObjectId generate_knowledge_object_id(std::uint32_t sequence);

}  // namespace eve
