#include "eve/core/platform_request.hpp"
#include "eve/core/platform_response.hpp"
#include "eve/capability/capability_engine.hpp"
#include "eve/core/core_platform.hpp"

#include <gtest/gtest.h>

#ifndef EVE_PROJECT_ROOT
#define EVE_PROJECT_ROOT "."
#endif

namespace eve {
namespace {

TEST(ContractComplianceTest, ValidatedPlatformRequestWrapsWithoutMutation) {
    auto request = PlatformRequest::create(
        CapabilityId{"CAP-0001"},
        InterfaceType::Internal,
        {{"query", "architecture"}});
    const auto request_id = request.metadata().request_id;

    const auto validated = ValidatedPlatformRequest::from(std::move(request));
    ASSERT_TRUE(validated.has_value());
    EXPECT_EQ(validated->capability().value, "CAP-0001");
    EXPECT_EQ(validated->parameter("query"), "architecture");
    EXPECT_EQ(validated->metadata().request_id.value, request_id.value);
}

TEST(ContractComplianceTest, CapabilityAliasResolutionDoesNotAlterRequest) {
    capability::CapabilityRegistry registry;
    registry.register_alias("search", CapabilityId{"CAP-0102"});

    const auto request = PlatformRequest::create(
        CapabilityId{"search"},
        InterfaceType::Internal,
        {{"query", "Platform Request"}});

    EXPECT_EQ(request.capability().value, "search");
    EXPECT_EQ(registry.resolve_capability(request.capability()).value, "CAP-0102");
    EXPECT_EQ(request.capability().value, "search");
}

TEST(ContractComplianceTest, AliasDispatchPreservesRequestIdentifier) {
    const auto request = PlatformRequest::create(
        CapabilityId{"status"},
        InterfaceType::Cli);
    const auto request_id = request.metadata().request_id;

    CorePlatform platform = PlatformBootstrap::create(EVE_PROJECT_ROOT);
    const auto response = platform.process(std::move(request));

    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->metadata().request_id.value, request_id.value);
    EXPECT_EQ(response->trace().capability_executed, "CAP-0002-STATUS");
}

TEST(ContractComplianceTest, WithTraceLeavesOriginalResponseUnchanged) {
    const auto request = PlatformRequest::create(
        CapabilityId{"CAP-0001"},
        InterfaceType::Internal);
    const auto response = PlatformResponse::success(
        request,
        ResponseContent{.primary = "ok", .sections = {}, .structured = {}});

    const auto traced = response.with_trace(ResponseTraceInformation{
        .duration_ms = 42,
        .capability_executed = "CAP-0001",
    });

    EXPECT_EQ(traced.trace().duration_ms, 42);
    EXPECT_FALSE(response.trace().duration_ms.has_value());
    EXPECT_EQ(response.content().primary, "ok");
}

TEST(ContractComplianceTest, ResponseValidationRejectsEmptyPayload) {
    const auto request = PlatformRequest::create(
        CapabilityId{"CAP-0001"},
        InterfaceType::Internal);

    const auto response = PlatformResponse::success(
        request,
        ResponseContent{.primary = "", .sections = {}, .structured = {}});

    EXPECT_TRUE(validate_platform_response(response).has_value());
}

}  // namespace
}  // namespace eve
