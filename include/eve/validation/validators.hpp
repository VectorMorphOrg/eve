#pragma once

#include "eve/capability/capability_engine.hpp"
#include "eve/core/platform_request.hpp"
#include "eve/core/platform_response.hpp"
#include "eve/validation/validation_result.hpp"

#include <map>
#include <set>

namespace eve::validation {

class StructuralValidator {
public:
    [[nodiscard]] ValidationResult validate(const PlatformRequest& request) const;
};

class CapabilityValidator {
public:
    explicit CapabilityValidator(const capability::CapabilityRegistry& registry);

    void set_capability_enabled(CapabilityId capability, bool enabled);

    [[nodiscard]] ValidationResult validate(const PlatformRequest& request) const;

    [[nodiscard]] CapabilityId resolved_capability(const PlatformRequest& request) const;

private:
    const capability::CapabilityRegistry& registry_;
    std::set<std::string> disabled_capabilities_;
};

class PermissionValidator {
public:
    void register_requirement(CapabilityId capability, PermissionLevel minimum_level);

    [[nodiscard]] ValidationResult validate(
        const PlatformRequest& request,
        CapabilityId resolved_capability) const;

private:
    [[nodiscard]] static bool satisfies(PermissionLevel granted, PermissionLevel required);

    std::map<std::string, PermissionLevel> requirements_;
};

class ResponseValidator {
public:
    [[nodiscard]] ValidationResult validate(const PlatformResponse& response) const;
};

}  // namespace eve::validation
