#include "eve/capability/capability_engine.hpp"
#include "eve/core/platform_request.hpp"

#include <gtest/gtest.h>

namespace eve {
namespace {

class EchoHandler final : public capability::ICapabilityHandler {
public:
    [[nodiscard]] CapabilityId id() const override { return CapabilityId{"CAP-TEST"}; }
    [[nodiscard]] std::string name() const override { return "Echo"; }

    [[nodiscard]] std::expected<PlatformResponse, CapabilityError> execute(
        const PlatformRequest& request) const override {
        return PlatformResponse::success(
            request,
            ResponseContent{.primary = "echo"});
    }
};

TEST(CapabilityEngineTest, ExecutesRegisteredHandler) {
    capability::CapabilityRegistry registry;
    registry.register_handler(std::make_shared<EchoHandler>());
    const capability::CapabilityEngine engine(std::move(registry));

    const auto request = PlatformRequest::create(
        CapabilityId{"CAP-TEST"},
        InterfaceType::Internal);
    const auto response = engine.execute(request);
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->content().primary, "echo");
}

}  // namespace
}  // namespace eve
