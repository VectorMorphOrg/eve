#include "eve/validation/parameter_schema.hpp"

namespace eve::validation {

void ParameterValidator::register_schema(
    CapabilityId capability,
    std::shared_ptr<IParameterSchema> schema) {
    schemas_[capability.value] = std::move(schema);
}

ValidationResult ParameterValidator::validate(
    const PlatformRequest& request,
    CapabilityId resolved_capability) const {
    const auto iterator = schemas_.find(resolved_capability.value);
    if (iterator == schemas_.end()) {
        return ValidationResult::pass(ValidationStage::Parameter);
    }
    return iterator->second->validate(request);
}

}  // namespace eve::validation
