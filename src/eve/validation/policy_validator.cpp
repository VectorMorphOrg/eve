#include "eve/validation/policy_validator.hpp"

namespace eve::validation {

void PolicyValidatorChain::add_validator(std::shared_ptr<IPolicyValidator> validator) {
    validators_.push_back(std::move(validator));
}

ValidationResult PolicyValidatorChain::validate(const PlatformRequest& request) const {
    for (const auto& validator : validators_) {
        const auto result = validator->validate(request);
        if (result.failed()) {
            return result;
        }
    }
    return ValidationResult::pass(ValidationStage::Policy);
}

}  // namespace eve::validation
