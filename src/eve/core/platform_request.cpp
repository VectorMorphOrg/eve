#include "eve/core/platform_request.hpp"

#include <format>

namespace eve {

PlatformRequest PlatformRequest::create(
    CapabilityId capability,
    InterfaceType interface_type,
    ParameterMap parameters,
    ExecutionContext execution_context,
    UserInfo user,
    PermissionInfo permissions) {
    PlatformRequest request;
    request.metadata_.request_id = generate_request_id();
    request.metadata_.timestamp = Clock::now();
    request.metadata_.correlation_id = CorrelationId{request.metadata_.request_id.value};
    request.interface_.type = interface_type;
    request.user_ = std::move(user);
    request.capability_ = std::move(capability);
    request.execution_context_ = std::move(execution_context);
    request.parameters_ = std::move(parameters);
    request.permissions_ = std::move(permissions);
    request.trace_.origin = to_string(interface_type);
    return request;
}

std::optional<std::string> PlatformRequest::parameter(std::string_view key) const {
    const auto iterator = parameters_.find(std::string{key});
    if (iterator == parameters_.end()) {
        return std::nullopt;
    }
    return iterator->second;
}

std::string PlatformRequest::debug_string() const {
    return std::format(
        "PlatformRequest{{id={}, capability={}, interface={}}}",
        metadata_.request_id.value,
        capability_.value,
        to_string(interface_.type));
}

std::optional<ValidationError> validate_platform_request(const PlatformRequest& request) {
    if (!request.capability()) {
        return ValidationError{"capability", "Capability identifier is required."};
    }
    if (request.capability().value.size() < 4) {
        return ValidationError{"capability", "Capability identifier format is invalid."};
    }
    return std::nullopt;
}

}  // namespace eve
