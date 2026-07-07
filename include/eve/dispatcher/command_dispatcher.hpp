#pragma once

#include "eve/capability/capability_engine.hpp"
#include "eve/core/platform_request.hpp"
#include "eve/core/platform_response.hpp"
#include "eve/services/service_interfaces.hpp"
#include "eve/validation/validation_engine.hpp"

#include <memory>

namespace eve::dispatcher {

class CommandDispatcher {
public:
    CommandDispatcher(
        std::shared_ptr<validation::ValidationEngine> validation_engine,
        capability::CapabilityEngine capability_engine,
        std::shared_ptr<services::ILoggingService> logger);

    [[nodiscard]] PlatformResponse dispatch(PlatformRequest request) const;

private:
    [[nodiscard]] PlatformResponse finalize_response(
        PlatformResponse response,
        const PlatformRequest& request) const;

    std::shared_ptr<validation::ValidationEngine> validation_engine_;
    capability::CapabilityEngine capability_engine_;
    std::shared_ptr<services::ILoggingService> logger_;
};

}  // namespace eve::dispatcher
