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
}

}  // namespace
}  // namespace eve
