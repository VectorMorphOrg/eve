#pragma once

#include "eve/capability/capability_engine.hpp"
#include "eve/core/platform_request.hpp"
#include "eve/core/platform_response.hpp"
#include "eve/services/service_interfaces.hpp"

#include <expected>
#include <map>
#include <memory>

namespace eve::dispatcher {

class IRequestValidator {
public:
    virtual ~IRequestValidator() = default;
    [[nodiscard]] virtual std::expected<void, ValidationError> validate(
        const PlatformRequest& request) const = 0;
};

class RequestValidator final : public IRequestValidator {
public:
    [[nodiscard]] std::expected<void, ValidationError> validate(
        const PlatformRequest& request) const override;
};

class CommandDispatcher {
public:
    CommandDispatcher(
        std::shared_ptr<IRequestValidator> validator,
        capability::CapabilityEngine capability_engine,
        std::shared_ptr<services::ILoggingService> logger,
        std::map<std::string, CapabilityId> aliases = {});

    [[nodiscard]] PlatformResponse dispatch(PlatformRequest request) const;

private:
    std::shared_ptr<IRequestValidator> validator_;
    capability::CapabilityEngine capability_engine_;
    std::shared_ptr<services::ILoggingService> logger_;
    std::map<std::string, CapabilityId> aliases_;
};

}  // namespace eve::dispatcher
