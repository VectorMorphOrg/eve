#include "eve/validation/validation_engine.hpp"

namespace eve::validation {

ValidationEngine::ValidationEngine(
    const capability::CapabilityRegistry& capability_registry,
    ParameterValidator parameter_validator,
    PermissionValidator permission_validator,
    PolicyValidatorChain policy_validators)
    : capability_validator_(capability_registry),
      permission_validator_(std::move(permission_validator)),
      parameter_validator_(std::move(parameter_validator)),
      policy_validators_(std::move(policy_validators)) {}

ValidationResult ValidationEngine::validate_request(const PlatformRequest& request) const {
    if (const auto result = structural_validator_.validate(request); result.failed()) {
        return result;
    }

    if (const auto result = capability_validator_.validate(request); result.failed()) {
        return result;
    }

    const auto resolved = capability_validator_.resolved_capability(request);

    if (const auto result = permission_validator_.validate(request, resolved); result.failed()) {
        return result;
    }

    if (const auto result = parameter_validator_.validate(request, resolved); result.failed()) {
        return result;
    }

    if (const auto result = policy_validators_.validate(request); result.failed()) {
        return result;
    }

    return ValidationResult::pass(ValidationStage::Policy);
}

ValidationResult ValidationEngine::validate_response(const PlatformResponse& response) const {
    return response_validator_.validate(response);
}

std::expected<ValidatedPlatformRequest, ValidationResult> ValidationEngine::adopt_request(
    PlatformRequest request) const {
    if (const auto result = validate_request(request); result.failed()) {
        return std::unexpected(result);
    }
    return ValidatedPlatformRequest::adopt(std::move(request));
}

std::expected<ValidatedPlatformResponse, ValidationResult> ValidationEngine::adopt_response(
    PlatformResponse response) const {
    if (const auto result = validate_response(response); result.failed()) {
        return std::unexpected(result);
    }
    return ValidatedPlatformResponse::adopt(std::move(response));
}

}  // namespace eve::validation
