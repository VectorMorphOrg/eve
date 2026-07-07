#include "eve/core/platform_request.hpp"

#include "eve/validation/validation_engine.hpp"
#include "eve/validation/validators.hpp"

#include <format>

namespace eve {

PlatformRequest PlatformRequest::create(
    CapabilityId capability,
    InterfaceType interface_type,
    ParameterMap parameters,
    ExecutionContext execution_context,
    UserInfo user,
    PermissionInfo permissions) {
    PlatformRequest request;
    request.metadata_.request_id = generate_request_id();
    request.metadata_.timestamp = Clock::now();
    request.metadata_.correlation_id = CorrelationId{request.metadata_.request_id.value};
    request.interface_.type = interface_type;
    request.user_ = std::move(user);
    request.capability_ = std::move(capability);
    request.execution_context_ = std::move(execution_context);
    request.parameters_ = std::move(parameters);
    request.permissions_ = std::move(permissions);
    request.trace_.origin = to_string(interface_type);
    return request;
}

std::optional<std::string> PlatformRequest::parameter(std::string_view key) const {
    const auto iterator = parameters_.find(std::string{key});
    if (iterator == parameters_.end()) {
        return std::nullopt;
    }
    return iterator->second;
}

std::string PlatformRequest::debug_string() const {
    return std::format(
        "PlatformRequest{{id={}, capability={}, interface={}}}",
        metadata_.request_id.value,
        capability_.value,
        to_string(interface_.type));
}

validation::ValidationResult validate_platform_request_structural(const PlatformRequest& request) {
    return validation::StructuralValidator{}.validate(request);
}

std::optional<ValidationError> validate_platform_request(const PlatformRequest& request) {
    return validate_platform_request_structural(request).first_error();
}

ValidatedPlatformRequest::ValidatedPlatformRequest(PlatformRequest request)
    : request_(std::move(request)) {}

std::expected<ValidatedPlatformRequest, validation::ValidationResult> ValidatedPlatformRequest::from(
    PlatformRequest request,
    const validation::ValidationEngine& engine) {
    return engine.adopt_request(std::move(request));
}

std::expected<ValidatedPlatformRequest, ValidationError> ValidatedPlatformRequest::from(
    PlatformRequest request) {
    if (const auto error = validate_platform_request(request)) {
        return std::unexpected(*error);
    }
    return ValidatedPlatformRequest(std::move(request));
}

ValidatedPlatformRequest ValidatedPlatformRequest::adopt(PlatformRequest request) {
    return ValidatedPlatformRequest(std::move(request));
}

}  // namespace eve
