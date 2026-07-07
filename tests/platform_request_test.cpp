#include "eve/core/platform_request.hpp"

#include <gtest/gtest.h>

namespace eve {
namespace {

TEST(PlatformRequestTest, CreatesValidRequest) {
    const auto request = PlatformRequest::create(
        CapabilityId{"CAP-0001"},
        InterfaceType::Internal,
        {{"key", "value"}});

    EXPECT_EQ(request.capability().value, "CAP-0001");
    EXPECT_FALSE(request.metadata().request_id.empty());
    EXPECT_EQ(request.parameter("key"), "value");
    EXPECT_FALSE(validate_platform_request(request).has_value());
}

TEST(PlatformRequestTest, RejectsMissingCapability) {
    const auto request = PlatformRequest::create(CapabilityId{}, InterfaceType::Internal);
    EXPECT_TRUE(validate_platform_request(request).has_value());
}

TEST(PlatformRequestTest, ValidatedPlatformRequestRejectsInvalidRequest) {
    const auto request = PlatformRequest::create(CapabilityId{}, InterfaceType::Internal);
    const auto validated = ValidatedPlatformRequest::from(std::move(request));
    EXPECT_FALSE(validated.has_value());
}

}  // namespace
}  // namespace eve
