#include "eve/dispatcher/command_dispatcher.hpp"

namespace eve::dispatcher {

CommandDispatcher::CommandDispatcher(
    std::shared_ptr<validation::ValidationEngine> validation_engine,
    capability::CapabilityEngine capability_engine,
    std::shared_ptr<services::ILoggingService> logger)
    : validation_engine_(std::move(validation_engine)),
      capability_engine_(std::move(capability_engine)),
      logger_(std::move(logger)) {}

PlatformResponse CommandDispatcher::finalize_response(
    PlatformResponse response,
    const PlatformRequest& request) const {
    const auto validated = validation_engine_->adopt_response(std::move(response));
    if (!validated) {
        return PlatformResponse::failure(request, validated.error().primary_message());
    }
    return validated->release();
}

PlatformResponse CommandDispatcher::dispatch(PlatformRequest request) const {
    if (const auto validation = validation_engine_->validate_request(request); validation.failed()) {
        return PlatformResponse::failure(request, validation.primary_message());
    }

    const ValidatedPlatformRequest validated = ValidatedPlatformRequest::adopt(std::move(request));
    const auto& platform_request = validated.request();
    const auto resolved_capability =
        validation_engine_->capability_validator().resolved_capability(platform_request);

    logger_->info(std::format(
        "Dispatching capability {} (resolved {}) for request {}",
        platform_request.capability().value,
        resolved_capability.value,
        platform_request.metadata().request_id.value));

    const auto result = capability_engine_.execute(platform_request);
    if (!result) {
        return finalize_response(
            PlatformResponse::unsupported(platform_request, result.error().message),
            platform_request);
    }

    ResponseTraceInformation trace{
        .capability_executed = resolved_capability.value,
    };
    return finalize_response(result->with_trace(std::move(trace)), platform_request);
}

}  // namespace eve::dispatcher
