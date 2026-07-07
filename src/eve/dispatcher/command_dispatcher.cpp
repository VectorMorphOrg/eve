#include "eve/dispatcher/command_dispatcher.hpp"

namespace eve::dispatcher {

std::expected<void, ValidationError> RequestValidator::validate(
    const PlatformRequest& request) const {
    if (const auto error = validate_platform_request(request)) {
        return std::unexpected(*error);
    }
    return {};
}

CommandDispatcher::CommandDispatcher(
    std::shared_ptr<IRequestValidator> validator,
    capability::CapabilityEngine capability_engine,
    std::shared_ptr<services::ILoggingService> logger,
    std::map<std::string, CapabilityId> aliases)
    : validator_(std::move(validator)),
      capability_engine_(std::move(capability_engine)),
      logger_(std::move(logger)),
      aliases_(std::move(aliases)) {}

PlatformResponse CommandDispatcher::dispatch(PlatformRequest request) const {
    if (const auto validation = validator_->validate(request); !validation) {
        return PlatformResponse::failure(request, validation.error().message);
    }

    CapabilityId capability = request.capability();
    if (const auto alias = aliases_.find(capability.value); alias != aliases_.end()) {
        capability = alias->second;
        request = PlatformRequest::create(
            capability,
            request.interface_info().type,
            request.parameters(),
            request.execution_context(),
            request.user(),
            request.permissions());
        if (const auto validation = validator_->validate(request); !validation) {
            return PlatformResponse::failure(request, validation.error().message);
        }
    }

    logger_->info(std::format(
        "Dispatching capability {} for request {}",
        request.capability().value,
        request.metadata().request_id.value));

    const auto result = capability_engine_.execute(request);
    if (!result) {
        return PlatformResponse::unsupported(request, result.error().message);
    }

    auto response = *result;
    ResponseTraceInformation trace{
        .capability_executed = request.capability().value,
    };
    response.set_trace(std::move(trace));
    return response;
}

}  // namespace eve::dispatcher
