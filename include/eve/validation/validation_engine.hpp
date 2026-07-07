#pragma once

#include "eve/capability/capability_engine.hpp"
#include "eve/core/platform_request.hpp"
#include "eve/core/platform_response.hpp"
#include "eve/validation/parameter_schema.hpp"
#include "eve/validation/policy_validator.hpp"
#include "eve/validation/validators.hpp"

#include <expected>
#include <memory>

namespace eve::validation {

class ValidationEngine {
public:
    ValidationEngine(
        const capability::CapabilityRegistry& capability_registry,
        ParameterValidator parameter_validator = {},
        PermissionValidator permission_validator = {},
        PolicyValidatorChain policy_validators = {});

    [[nodiscard]] const CapabilityValidator& capability_validator() const noexcept {
        return capability_validator_;
    }
    [[nodiscard]] CapabilityValidator& capability_validator() noexcept {
        return capability_validator_;
    }
    [[nodiscard]] const PermissionValidator& permission_validator() const noexcept {
        return permission_validator_;
    }
    [[nodiscard]] ParameterValidator& parameter_validator() noexcept {
        return parameter_validator_;
    }
    [[nodiscard]] PolicyValidatorChain& policy_validators() noexcept {
        return policy_validators_;
    }

    [[nodiscard]] ValidationResult validate_request(const PlatformRequest& request) const;
    [[nodiscard]] ValidationResult validate_response(const PlatformResponse& response) const;

    [[nodiscard]] std::expected<ValidatedPlatformRequest, ValidationResult> adopt_request(
        PlatformRequest request) const;

    [[nodiscard]] std::expected<ValidatedPlatformResponse, ValidationResult> adopt_response(
        PlatformResponse response) const;

private:
    StructuralValidator structural_validator_;
    CapabilityValidator capability_validator_;
    PermissionValidator permission_validator_;
    ParameterValidator parameter_validator_;
    PolicyValidatorChain policy_validators_;
    ResponseValidator response_validator_;
};

}  // namespace eve::validation
