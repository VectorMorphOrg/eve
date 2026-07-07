#include "eve/core/core_platform.hpp"

namespace eve {

CorePlatform::CorePlatform(CorePlatformDependencies dependencies)
    : dependencies_(std::move(dependencies)) {}

std::expected<PlatformResponse, PlatformError> CorePlatform::process(
    PlatformRequest request) const {
    const auto start = Clock::now();
    auto response = dependencies_.dispatcher.dispatch(std::move(request));

    const auto end = Clock::now();
    const auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    auto trace = response.trace();
    trace.duration_ms = duration;
    trace.services_executed = {
        "CommandDispatcher",
        "CapabilityEngine",
    };
    response.set_trace(std::move(trace));
    return response;
}

std::vector<KnowledgeObject> CorePlatform::gather_knowledge(
    const PlatformRequest& request) const {
    (void)request;
    return {};
}

}  // namespace eve
