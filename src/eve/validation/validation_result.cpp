#include "eve/validation/validation_result.hpp"

namespace eve::validation {

std::string to_string(ValidationStage stage) {
    switch (stage) {
        case ValidationStage::Structural:
            return "structural";
        case ValidationStage::Capability:
            return "capability";
        case ValidationStage::Permission:
            return "permission";
        case ValidationStage::Parameter:
            return "parameter";
        case ValidationStage::Policy:
            return "policy";
        case ValidationStage::Response:
            return "response";
    }
    return "unknown";
}

ValidationResult ValidationResult::pass(ValidationStage stage) {
    return ValidationResult{
        .status = ValidationStatus::Passed,
        .stage = stage,
    };
}

ValidationResult ValidationResult::fail(
    ValidationStage stage,
    std::string field,
    std::string message,
    std::optional<std::string> failure_reason) {
    ValidationResult result{
        .status = ValidationStatus::Failed,
        .stage = stage,
        .failure_reason = failure_reason ? std::move(failure_reason) : std::optional<std::string>{message},
        .errors = {ValidationError{std::move(field), std::move(message)}},
    };
    result.diagnostics.push_back(DiagnosticMessage{"error", result.errors.front().message});
    return result;
}

std::string ValidationResult::primary_message() const {
    if (!errors.empty()) {
        return errors.front().message;
    }
    if (failure_reason) {
        return *failure_reason;
    }
    return "Validation failed.";
}

std::optional<ValidationError> ValidationResult::first_error() const {
    if (errors.empty()) {
        return std::nullopt;
    }
    return errors.front();
}

}  // namespace eve::validation
