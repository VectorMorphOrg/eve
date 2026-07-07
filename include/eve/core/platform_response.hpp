#pragma once

#include "eve/core/errors.hpp"
#include "eve/core/platform_request.hpp"
#include "eve/core/types.hpp"

#include <optional>
#include <string>
#include <vector>

namespace eve {

struct ResponseMetadata {
    ResponseId response_id;
    RequestId request_id;
    TimePoint timestamp{};
    std::string response_version{"1.0.0"};
    CorrelationId correlation_id;
};

struct ResponseContent {
    std::string primary;
    std::vector<std::string> sections;
    std::map<std::string, std::string> structured;
};

struct ResponseTraceInformation {
    std::optional<std::int64_t> duration_ms;
    std::vector<std::string> services_executed;
    std::optional<std::string> capability_executed;
    std::optional<std::string> provider_selected;
    std::vector<TraceEntry> entries;
};

class PlatformResponse {
public:
    [[nodiscard]] static PlatformResponse success(
        const PlatformRequest& request,
        ResponseContent content,
        std::vector<Reference> references = {},
        std::vector<SuggestedAction> actions = {});

    [[nodiscard]] static PlatformResponse partial_success(
        const PlatformRequest& request,
        ResponseContent content,
        std::vector<DiagnosticMessage> diagnostics,
        std::vector<Reference> references = {});

    [[nodiscard]] static PlatformResponse warning(
        const PlatformRequest& request,
        ResponseContent content,
        std::vector<DiagnosticMessage> diagnostics);

    [[nodiscard]] static PlatformResponse failure(
        const PlatformRequest& request,
        std::string message,
        std::vector<DiagnosticMessage> diagnostics = {});

    [[nodiscard]] static PlatformResponse unauthorized(
        const PlatformRequest& request,
        std::string message);

    [[nodiscard]] static PlatformResponse not_found(
        const PlatformRequest& request,
        std::string message);

    [[nodiscard]] static PlatformResponse unsupported(
        const PlatformRequest& request,
        std::string message);

    [[nodiscard]] const ResponseMetadata& metadata() const noexcept { return metadata_; }
    [[nodiscard]] ResponseStatus status() const noexcept { return status_; }
    [[nodiscard]] const ResponseContent& content() const noexcept { return content_; }
    [[nodiscard]] const std::vector<Reference>& references() const noexcept {
        return references_;
    }
    [[nodiscard]] const std::vector<DiagnosticMessage>& diagnostics() const noexcept {
        return diagnostics_;
    }
    [[nodiscard]] const std::vector<SuggestedAction>& actions() const noexcept {
        return actions_;
    }
    [[nodiscard]] const ResponseTraceInformation& trace() const noexcept { return trace_; }

    void set_trace(ResponseTraceInformation trace);
    [[nodiscard]] std::string debug_string() const;

private:
    PlatformResponse(
        const PlatformRequest& request,
        ResponseStatus status,
        ResponseContent content,
        std::vector<Reference> references,
        std::vector<DiagnosticMessage> diagnostics,
        std::vector<SuggestedAction> actions);

    ResponseMetadata metadata_{};
    ResponseStatus status_{ResponseStatus::Failure};
    ResponseContent content_{};
    std::vector<Reference> references_{};
    std::vector<DiagnosticMessage> diagnostics_{};
    std::vector<SuggestedAction> actions_{};
    ResponseTraceInformation trace_{};
};

}  // namespace eve
