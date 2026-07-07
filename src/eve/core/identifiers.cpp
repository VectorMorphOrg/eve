#include "eve/core/types.hpp"

#include <atomic>
#include <format>
#include <iomanip>
#include <sstream>

namespace eve {
namespace {

std::string format_date(TimePoint time_point) {
    const auto time = Clock::to_time_t(time_point);
    std::tm tm{};
#if defined(_WIN32)
    gmtime_s(&tm, &time);
#else
    gmtime_r(&time, &tm);
#endif
    std::ostringstream stream;
    stream << std::put_time(&tm, "%Y%m%d");
    return stream.str();
}

std::string next_sequence_id(std::string_view prefix, TimePoint now) {
    static std::atomic<std::uint32_t> counter{0};
    const auto sequence = counter.fetch_add(1) + 1;
    return std::format("{}-{}-{:06d}", prefix, format_date(now), sequence);
}

}  // namespace

std::string to_string(InterfaceType type) {
    switch (type) {
        case InterfaceType::Discord:
            return "discord";
        case InterfaceType::Website:
            return "website";
        case InterfaceType::Cli:
            return "cli";
        case InterfaceType::RestApi:
            return "rest_api";
        case InterfaceType::Internal:
            return "internal";
    }
    return "unknown";
}

std::string to_string(PermissionLevel level) {
    switch (level) {
        case PermissionLevel::Public:
            return "public";
        case PermissionLevel::Developer:
            return "developer";
        case PermissionLevel::Maintainer:
            return "maintainer";
        case PermissionLevel::Administrator:
            return "administrator";
    }
    return "unknown";
}

std::string to_string(ResponseStatus status) {
    switch (status) {
        case ResponseStatus::Success:
            return "success";
        case ResponseStatus::PartialSuccess:
            return "partial_success";
        case ResponseStatus::Warning:
            return "warning";
        case ResponseStatus::Failure:
            return "failure";
        case ResponseStatus::Unauthorized:
            return "unauthorized";
        case ResponseStatus::NotFound:
            return "not_found";
        case ResponseStatus::Unsupported:
            return "unsupported";
    }
    return "unknown";
}

std::string to_string(DocumentType type) {
    switch (type) {
        case DocumentType::Specification:
            return "specification";
        case DocumentType::ReferenceCatalog:
            return "reference_catalog";
        case DocumentType::Rfc:
            return "rfc";
        case DocumentType::Whitepaper:
            return "whitepaper";
        case DocumentType::Readme:
            return "readme";
        case DocumentType::Guide:
            return "guide";
        case DocumentType::Changelog:
            return "changelog";
        case DocumentType::Other:
            return "other";
    }
    return "unknown";
}

RequestId generate_request_id(TimePoint now) {
    return RequestId{next_sequence_id("REQ", now)};
}

ResponseId generate_response_id(TimePoint now) {
    return ResponseId{next_sequence_id("RES", now)};
}

PackageId generate_package_id(TimePoint now) {
    return PackageId{next_sequence_id("CTX", now)};
}

KnowledgeObjectId generate_knowledge_object_id(std::uint32_t sequence) {
    return KnowledgeObjectId{std::format("KO-{:08d}", sequence)};
}

}  // namespace eve
