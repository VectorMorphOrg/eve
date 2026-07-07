#include "eve/core/platform_response.hpp"

#include <format>

namespace eve {

PlatformResponse::PlatformResponse(
    const PlatformRequest& request,
    ResponseStatus status,
    ResponseContent content,
    std::vector<Reference> references,
    std::vector<DiagnosticMessage> diagnostics,
    std::vector<SuggestedAction> actions)
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
      actions_(std::move(actions)) {}

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
        ResponseContent{.primary = message},
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
        ResponseContent{.primary = message},
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
        ResponseContent{.primary = message},
        {},
        {DiagnosticMessage{"error", message}},
        {});
}

void PlatformResponse::set_trace(ResponseTraceInformation trace) {
    trace_ = std::move(trace);
}

std::string PlatformResponse::debug_string() const {
    return std::format(
        "PlatformResponse{{id={}, status={}, request_id={}}}",
        metadata_.response_id.value,
        to_string(status_),
        metadata_.request_id.value);
}

}  // namespace eve
