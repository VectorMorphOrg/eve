#include "eve/core/core_platform.hpp"
#include "eve/core/platform_request.hpp"

#include <gtest/gtest.h>

#ifndef EVE_PROJECT_ROOT
#define EVE_PROJECT_ROOT "."
#endif

namespace eve {
namespace {

TEST(IntegrationTest, DocumentationSearchEndToEnd) {
    CorePlatform platform = PlatformBootstrap::create(EVE_PROJECT_ROOT);
    const auto request = PlatformRequest::create(
        CapabilityId{"CAP-0102"},
        InterfaceType::Internal,
        {{"query", "Platform Request"}});

    const auto response = platform.process(std::move(request));
    ASSERT_TRUE(response.has_value());
    EXPECT_TRUE(
        response->status() == ResponseStatus::Success ||
        response->status() == ResponseStatus::PartialSuccess);
    EXPECT_NE(response->content().primary.find("Platform Request"), std::string::npos);
    EXPECT_FALSE(response->references().empty());
}

TEST(IntegrationTest, StatusCapabilityViaAlias) {
    CorePlatform platform = PlatformBootstrap::create(EVE_PROJECT_ROOT);
    const auto request = PlatformRequest::create(
        CapabilityId{"status"},
        InterfaceType::Cli);

    const auto response = platform.process(std::move(request));
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->status(), ResponseStatus::Success);
    EXPECT_NE(response->content().primary.find("0.1.0"), std::string::npos);
}

}  // namespace
}  // namespace eve
