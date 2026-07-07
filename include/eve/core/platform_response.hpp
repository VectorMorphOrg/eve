#pragma once

#include "eve/core/errors.hpp"
#include "eve/core/platform_request.hpp"
#include "eve/core/types.hpp"
#include "eve/validation/validation_result.hpp"

#include <expected>
#include <optional>
#include <string>
#include <vector>

namespace eve {

namespace validation {
class ValidationEngine;
}

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

    /// Returns a new Platform Response with trace information applied. Does not modify
    /// this response (EVE-0011 immutability).
    [[nodiscard]] PlatformResponse with_trace(ResponseTraceInformation trace) const;

    [[nodiscard]] std::string debug_string() const;

private:
    PlatformResponse(
        const PlatformRequest& request,
        ResponseStatus status,
        ResponseContent content,
        std::vector<Reference> references,
        std::vector<DiagnosticMessage> diagnostics,
        std::vector<SuggestedAction> actions,
        ResponseTraceInformation trace = {});

    ResponseMetadata metadata_{};
    ResponseStatus status_{ResponseStatus::Failure};
    ResponseContent content_{};
    std::vector<Reference> references_{};
    std::vector<DiagnosticMessage> diagnostics_{};
    std::vector<SuggestedAction> actions_{};
    ResponseTraceInformation trace_{};
};

struct ResponseValidationError {
    std::string field;
    std::string message;
};

[[nodiscard]] std::optional<ResponseValidationError> validate_platform_response(
    const PlatformResponse& response);

/// Prefer ValidationEngine for full response validation.
[[nodiscard]] validation::ValidationResult validate_platform_response_full(
    const PlatformResponse& response);

class ValidatedPlatformResponse {
public:
    [[nodiscard]] static std::expected<ValidatedPlatformResponse, validation::ValidationResult>
    from(PlatformResponse response, const validation::ValidationEngine& engine);

    /// Prefer from(response, engine) for pipeline-validated adoption.
    [[nodiscard]] static std::expected<ValidatedPlatformResponse, ResponseValidationError> from(
        PlatformResponse response);

    /// Adopts a response that has already passed validate_platform_response().
    [[nodiscard]] static ValidatedPlatformResponse adopt(PlatformResponse response);

    [[nodiscard]] const PlatformResponse& response() const noexcept { return response_; }

    [[nodiscard]] const ResponseMetadata& metadata() const noexcept {
        return response_.metadata();
    }
    [[nodiscard]] ResponseStatus status() const noexcept { return response_.status(); }
    [[nodiscard]] const ResponseContent& content() const noexcept { return response_.content(); }
    [[nodiscard]] const std::vector<Reference>& references() const noexcept {
        return response_.references();
    }
    [[nodiscard]] const std::vector<DiagnosticMessage>& diagnostics() const noexcept {
        return response_.diagnostics();
    }
    [[nodiscard]] const std::vector<SuggestedAction>& actions() const noexcept {
        return response_.actions();
    }
    [[nodiscard]] const ResponseTraceInformation& trace() const noexcept {
        return response_.trace();
    }

    [[nodiscard]] PlatformResponse release() const { return response_; }

private:
    explicit ValidatedPlatformResponse(PlatformResponse response);

    PlatformResponse response_;
};

}  // namespace eve
