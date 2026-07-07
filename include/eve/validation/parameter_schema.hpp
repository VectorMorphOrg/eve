#pragma once

#include "eve/core/platform_request.hpp"
#include "eve/validation/validation_result.hpp"

#include <map>
#include <memory>

namespace eve::validation {

/// Capability-specific parameter schema extension point (EVE-0012).
class IParameterSchema {
public:
    virtual ~IParameterSchema() = default;

    [[nodiscard]] virtual ValidationResult validate(const PlatformRequest& request) const = 0;
};

class ParameterValidator {
public:
    void register_schema(CapabilityId capability, std::shared_ptr<IParameterSchema> schema);

    [[nodiscard]] ValidationResult validate(
        const PlatformRequest& request,
        CapabilityId resolved_capability) const;

private:
    std::map<std::string, std::shared_ptr<IParameterSchema>> schemas_;
};

}  // namespace eve::validation
