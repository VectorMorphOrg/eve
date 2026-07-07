#include "eve/core/platform_response.hpp"

#include <gtest/gtest.h>

namespace eve {
namespace {

TEST(PlatformResponseTest, SuccessFactorySetsStatus) {
    const auto request = PlatformRequest::create(
        CapabilityId{"CAP-0001"},
        InterfaceType::Internal);
    const auto response = PlatformResponse::success(
        request,
        ResponseContent{.primary = "ok"});

    EXPECT_EQ(response.status(), ResponseStatus::Success);
    EXPECT_EQ(response.content().primary, "ok");
    EXPECT_EQ(response.metadata().request_id.value, request.metadata().request_id.value);
}

TEST(PlatformResponseTest, FailureFactorySetsDiagnostics) {
    const auto request = PlatformRequest::create(
        CapabilityId{"CAP-0001"},
        InterfaceType::Internal);
    const auto response = PlatformResponse::failure(request, "failed");

    EXPECT_EQ(response.status(), ResponseStatus::Failure);
    EXPECT_FALSE(response.diagnostics().empty());
    EXPECT_FALSE(validate_platform_response(response).has_value());
}

TEST(PlatformResponseTest, ValidatedPlatformResponseAdoptsValidResponse) {
    const auto request = PlatformRequest::create(
        CapabilityId{"CAP-0001"},
        InterfaceType::Internal);
    const auto response = PlatformResponse::success(
        request,
        ResponseContent{.primary = "ok", .sections = {}, .structured = {}});

    const auto validated = ValidatedPlatformResponse::from(response);
    ASSERT_TRUE(validated.has_value());
    EXPECT_EQ(validated->status(), ResponseStatus::Success);
}

}  // namespace
}  // namespace eve
