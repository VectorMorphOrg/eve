#include "eve/core/platform_response.hpp"

#include "eve/validation/validation_engine.hpp"
#include "eve/validation/validators.hpp"

#include <format>
#include <utility>

namespace eve {

PlatformResponse::PlatformResponse(
    const PlatformRequest& request,
    ResponseStatus status,
    ResponseContent content,
    std::vector<Reference> references,
    std::vector<DiagnosticMessage> diagnostics,
    std::vector<SuggestedAction> actions,
    ResponseTraceInformation trace)
    : metadata_{
          .response_id = generate_response_id(),
          .request_id = request.metadata().request_id,
          .timestamp = Clock::now(),
          .response_version = "1.0.0",
          .correlation_id = request.metadata().correlation_id,
      },
      status_(status),
      content_(std::move(content)),
      references_(std::move(references)),
      diagnostics_(std::move(diagnostics)),
      actions_(std::move(actions)),
      trace_(std::move(trace)) {}

PlatformResponse PlatformResponse::success(
    const PlatformRequest& request,
    ResponseContent content,
    std::vector<Reference> references,
    std::vector<SuggestedAction> actions) {
    return PlatformResponse(
        request,
        ResponseStatus::Success,
        std::move(content),
        std::move(references),
        {},
        std::move(actions));
}

PlatformResponse PlatformResponse::partial_success(
    const PlatformRequest& request,
    ResponseContent content,
    std::vector<DiagnosticMessage> diagnostics,
    std::vector<Reference> references) {
    return PlatformResponse(
        request,
        ResponseStatus::PartialSuccess,
        std::move(content),
        std::move(references),
        std::move(diagnostics),
        {});
}

PlatformResponse PlatformResponse::warning(
    const PlatformRequest& request,
    ResponseContent content,
    std::vector<DiagnosticMessage> diagnostics) {
    return PlatformResponse(
        request,
        ResponseStatus::Warning,
        std::move(content),
        {},
        std::move(diagnostics),
        {});
}

PlatformResponse PlatformResponse::failure(
    const PlatformRequest& request,
    std::string message,
    std::vector<DiagnosticMessage> diagnostics) {
    diagnostics.push_back(DiagnosticMessage{"error", message});
    return PlatformResponse(
        request,
        ResponseStatus::Failure,
        ResponseContent{.primary = message, .sections = {}, .structured = {}},
        {},
        std::move(diagnostics),
        {});
}

PlatformResponse PlatformResponse::unauthorized(
    const PlatformRequest& request,
    std::string message) {
    return PlatformResponse(
        request,
        ResponseStatus::Unauthorized,
        ResponseContent{.primary = message, .sections = {}, .structured = {}},
        {},
        {DiagnosticMessage{"error", message}},
        {});
}

PlatformResponse PlatformResponse::not_found(
    const PlatformRequest& request,
    std::string message) {
    return PlatformResponse(
        request,
        ResponseStatus::NotFound,
        ResponseContent{.primary = message, .sections = {}, .structured = {}},
        {},
        {DiagnosticMessage{"error", message}},
        {});
}

PlatformResponse PlatformResponse::unsupported(
    const PlatformRequest& request,
    std::string message) {
    return PlatformResponse(
        request,
        ResponseStatus::Unsupported,
        ResponseContent{.primary = message, .sections = {}, .structured = {}},
        {},
        {DiagnosticMessage{"error", message}},
        {});
}

PlatformResponse PlatformResponse::with_trace(ResponseTraceInformation trace) const {
    PlatformResponse response = *this;
    response.trace_ = std::move(trace);
    return response;
}

validation::ValidationResult validate_platform_response_full(const PlatformResponse& response) {
    return validation::ResponseValidator{}.validate(response);
}

std::optional<ResponseValidationError> validate_platform_response(const PlatformResponse& response) {
    if (const auto error = validate_platform_response_full(response).first_error()) {
        return ResponseValidationError{error->field, error->message};
    }
    return std::nullopt;
}

ValidatedPlatformResponse::ValidatedPlatformResponse(PlatformResponse response)
    : response_(std::move(response)) {}

std::expected<ValidatedPlatformResponse, validation::ValidationResult> ValidatedPlatformResponse::from(
    PlatformResponse response,
    const validation::ValidationEngine& engine) {
    return engine.adopt_response(std::move(response));
}

std::expected<ValidatedPlatformResponse, ResponseValidationError> ValidatedPlatformResponse::from(
    PlatformResponse response) {
    if (const auto error = validate_platform_response(response)) {
        return std::unexpected(*error);
    }
    return ValidatedPlatformResponse(std::move(response));
}

ValidatedPlatformResponse ValidatedPlatformResponse::adopt(PlatformResponse response) {
    return ValidatedPlatformResponse(std::move(response));
}

std::string PlatformResponse::debug_string() const {
    return std::format(
        "PlatformResponse{{id={}, status={}, request_id={}}}",
        metadata_.response_id.value,
        to_string(status_),
        metadata_.request_id.value);
}

}  // namespace eve
