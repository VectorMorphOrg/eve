#include "eve/core/core_platform.hpp"

namespace eve {

CorePlatform::CorePlatform(CorePlatformDependencies dependencies)
    : dependencies_(std::move(dependencies)) {}

std::expected<PlatformResponse, PlatformError> CorePlatform::process(
    PlatformRequest request) const {
    const auto start = Clock::now();
    PlatformResponse response = dependencies_.dispatcher.dispatch(std::move(request));

    const auto end = Clock::now();
    const auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    auto trace = response.trace();
    trace.duration_ms = duration;
    trace.services_executed = {
        "CommandDispatcher",
        "CapabilityEngine",
    };
    response = response.with_trace(std::move(trace));

    const auto validated = dependencies_.validation_engine->adopt_response(std::move(response));
    if (!validated) {
        return std::unexpected(PlatformError{
            .type = "response_validation_error",
            .description = validated.error().primary_message(),
            .resolution = "Inspect platform response construction.",
        });
    }

    return validated->release();
}

std::vector<KnowledgeObject> CorePlatform::gather_knowledge(
    const PlatformRequest& request) const {
    (void)request;
    return {};
}

}  // namespace eve
