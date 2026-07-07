#pragma once

#include "eve/core/platform_request.hpp"
#include "eve/validation/validation_result.hpp"

#include <memory>
#include <vector>

namespace eve::validation {

/// Policy validation extension point (EVE-0012). Not implemented in Phase 1.2.
class IPolicyValidator {
public:
    virtual ~IPolicyValidator() = default;

    [[nodiscard]] virtual ValidationResult validate(const PlatformRequest& request) const = 0;
};

class PolicyValidatorChain {
public:
    void add_validator(std::shared_ptr<IPolicyValidator> validator);

    [[nodiscard]] ValidationResult validate(const PlatformRequest& request) const;

private:
    std::vector<std::shared_ptr<IPolicyValidator>> validators_;
};

}  // namespace eve::validation
