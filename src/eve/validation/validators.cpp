#include "eve/validation/validators.hpp"

#include <format>

namespace eve::validation {

ValidationResult StructuralValidator::validate(const PlatformRequest& request) const {
    if (request.metadata().request_id.empty()) {
        return ValidationResult::fail(
            ValidationStage::Structural,
            "metadata.request_id",
            "Request identifier is required.");
    }
    if (request.metadata().correlation_id.value.empty()) {
        return ValidationResult::fail(
            ValidationStage::Structural,
            "metadata.correlation_id",
            "Correlation identifier is required.");
    }
    if (!request.capability()) {
        return ValidationResult::fail(
            ValidationStage::Structural,
            "capability",
            "Capability identifier is required.");
    }
    if (request.capability().value.size() < 4) {
        return ValidationResult::fail(
            ValidationStage::Structural,
            "capability",
            "Capability identifier format is invalid.");
    }
    if (request.metadata().request_version.empty()) {
        return ValidationResult::fail(
            ValidationStage::Structural,
            "metadata.request_version",
            "Request version is required.");
    }
    return ValidationResult::pass(ValidationStage::Structural);
}

CapabilityValidator::CapabilityValidator(const capability::CapabilityRegistry& registry)
    : registry_(registry) {}

void CapabilityValidator::set_capability_enabled(CapabilityId capability, bool enabled) {
    if (enabled) {
        disabled_capabilities_.erase(capability.value);
    } else {
        disabled_capabilities_.insert(capability.value);
    }
}

CapabilityId CapabilityValidator::resolved_capability(const PlatformRequest& request) const {
    return registry_.resolve_capability(request.capability());
}

ValidationResult CapabilityValidator::validate(const PlatformRequest& request) const {
    const auto requested = request.capability();
    const auto resolved = registry_.resolve_capability(requested);

    const auto handler = registry_.resolve(requested);
    if (!handler) {
        if (requested.value != resolved.value) {
            return ValidationResult::fail(
                ValidationStage::Capability,
                "capability",
                std::format(
                    "Capability alias '{}' does not resolve to a registered capability.",
                    requested.value));
        }
        return ValidationResult::fail(
            ValidationStage::Capability,
            "capability",
            std::format("Capability '{}' is not registered.", requested.value),
            std::format("Unknown capability '{}'.", requested.value));
    }

    if (disabled_capabilities_.contains(resolved.value)) {
        return ValidationResult::fail(
            ValidationStage::Capability,
            "capability",
            std::format("Capability '{}' is disabled.", resolved.value));
    }

    return ValidationResult::pass(ValidationStage::Capability);
}

bool PermissionValidator::satisfies(PermissionLevel granted, PermissionLevel required) {
    return static_cast<int>(granted) >= static_cast<int>(required);
}

void PermissionValidator::register_requirement(
    CapabilityId capability,
    PermissionLevel minimum_level) {
    requirements_[capability.value] = minimum_level;
}

ValidationResult PermissionValidator::validate(
    const PlatformRequest& request,
    CapabilityId resolved_capability) const {
    const auto iterator = requirements_.find(resolved_capability.value);
    const auto required =
        iterator == requirements_.end() ? PermissionLevel::Public : iterator->second;

    if (!satisfies(request.permissions().level, required)) {
        return ValidationResult::fail(
            ValidationStage::Permission,
            "permissions.level",
            std::format(
                "Permission level '{}' is insufficient for capability '{}'.",
                to_string(request.permissions().level),
                resolved_capability.value),
            "Permission denied.");
    }

    return ValidationResult::pass(ValidationStage::Permission);
}

ValidationResult ResponseValidator::validate(const PlatformResponse& response) const {
    if (response.metadata().response_id.empty()) {
        return ValidationResult::fail(
            ValidationStage::Response,
            "metadata.response_id",
            "Response identifier is required.");
    }
    if (response.metadata().request_id.empty()) {
        return ValidationResult::fail(
            ValidationStage::Response,
            "metadata.request_id",
            "Request identifier is required.");
    }
    if (response.metadata().correlation_id.value.empty()) {
        return ValidationResult::fail(
            ValidationStage::Response,
            "metadata.correlation_id",
            "Correlation identifier is required.");
    }
    if (response.content().primary.empty() && response.diagnostics().empty()) {
        return ValidationResult::fail(
            ValidationStage::Response,
            "content",
            "Response must contain content or diagnostics.");
    }
    return ValidationResult::pass(ValidationStage::Response);
}

}  // namespace eve::validation
