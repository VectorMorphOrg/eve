#pragma once

#include "eve/core/errors.hpp"

#include <optional>
#include <string>
#include <vector>

namespace eve::validation {

enum class ValidationStatus {
    Passed,
    Failed,
};

enum class ValidationStage {
    Structural,
    Capability,
    Permission,
    Parameter,
    Policy,
    Response,
};

[[nodiscard]] std::string to_string(ValidationStage stage);

struct ValidationResult {
    ValidationStatus status{ValidationStatus::Passed};
    ValidationStage stage{ValidationStage::Structural};
    std::optional<std::string> failure_reason;
    std::vector<ValidationError> errors;
    std::vector<DiagnosticMessage> warnings;
    std::vector<DiagnosticMessage> diagnostics;

    [[nodiscard]] bool passed() const noexcept { return status == ValidationStatus::Passed; }
    [[nodiscard]] bool failed() const noexcept { return status == ValidationStatus::Failed; }

    [[nodiscard]] static ValidationResult pass(ValidationStage stage);
    [[nodiscard]] static ValidationResult fail(
        ValidationStage stage,
        std::string field,
        std::string message,
        std::optional<std::string> failure_reason = std::nullopt);

    [[nodiscard]] std::string primary_message() const;
    [[nodiscard]] std::optional<ValidationError> first_error() const;
};

}  // namespace eve::validation
