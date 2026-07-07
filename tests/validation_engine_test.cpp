#include "eve/capability/capability_engine.hpp"
#include "eve/core/platform_request.hpp"
#include "eve/core/platform_response.hpp"
#include "eve/validation/parameter_schema.hpp"
#include "eve/validation/validation_engine.hpp"
#include "eve/validation/validators.hpp"

#include <gtest/gtest.h>

namespace eve {
namespace {

capability::CapabilityRegistry make_test_registry() {
    capability::CapabilityRegistry registry;
    registry.register_alias("search", CapabilityId{"CAP-0102"});
    return registry;
}

class RequiredQuerySchema final : public validation::IParameterSchema {
public:
    [[nodiscard]] validation::ValidationResult validate(const PlatformRequest& request) const override {
        const auto query = request.parameter("query");
        if (!query || query->empty()) {
            return validation::ValidationResult::fail(
                validation::ValidationStage::Parameter,
                "parameters.query",
                "Required parameter 'query' is missing.");
        }
        return validation::ValidationResult::pass(validation::ValidationStage::Parameter);
    }
};

TEST(StructuralValidatorTest, AcceptsWellFormedRequest) {
    const auto request = PlatformRequest::create(CapabilityId{"CAP-0001"}, InterfaceType::Internal);

    const validation::StructuralValidator validator;
    const auto result = validator.validate(request);
    EXPECT_TRUE(result.passed());
}

TEST(StructuralValidatorTest, RejectsMissingCapability) {
    const auto request = PlatformRequest::create(CapabilityId{}, InterfaceType::Internal);
    const validation::StructuralValidator validator;
    const auto result = validator.validate(request);
    EXPECT_TRUE(result.failed());
    EXPECT_EQ(result.stage, validation::ValidationStage::Structural);
}

TEST(CapabilityValidatorTest, AcceptsRegisteredCapability) {
    capability::CapabilityRegistry registry;
    class TestHandler final : public capability::ICapabilityHandler {
    public:
        [[nodiscard]] CapabilityId id() const override { return CapabilityId{"CAP-0001"}; }
        [[nodiscard]] std::string name() const override { return "Test"; }
        [[nodiscard]] std::expected<PlatformResponse, CapabilityError> execute(
            const PlatformRequest&) const override {
            return PlatformResponse::success(
                PlatformRequest::create(CapabilityId{"CAP-0001"}, InterfaceType::Internal),
                ResponseContent{.primary = "ok", .sections = {}, .structured = {}});
        }
    };
    registry.register_handler(std::make_shared<TestHandler>());

    validation::ValidationEngine engine(registry);
    const auto request = PlatformRequest::create(CapabilityId{"CAP-0001"}, InterfaceType::Internal);
    const auto result = engine.validate_request(request);
    EXPECT_TRUE(result.passed());
}

TEST(CapabilityValidatorTest, RejectsUnknownCapability) {
    const capability::CapabilityRegistry registry;
    validation::ValidationEngine engine(registry);
    const auto request = PlatformRequest::create(CapabilityId{"CAP-9999"}, InterfaceType::Internal);
    const auto result = engine.validate_request(request);
    EXPECT_TRUE(result.failed());
    EXPECT_EQ(result.stage, validation::ValidationStage::Capability);
}

TEST(CapabilityValidatorTest, ResolvesAliasWithoutMutatingRequest) {
    capability::CapabilityRegistry registry;
    class TestHandler final : public capability::ICapabilityHandler {
    public:
        [[nodiscard]] CapabilityId id() const override { return CapabilityId{"CAP-0102"}; }
        [[nodiscard]] std::string name() const override { return "Search"; }
        [[nodiscard]] std::expected<PlatformResponse, CapabilityError> execute(
            const PlatformRequest&) const override {
            return PlatformResponse::success(
                PlatformRequest::create(CapabilityId{"CAP-0102"}, InterfaceType::Internal),
                ResponseContent{.primary = "ok", .sections = {}, .structured = {}});
        }
    };
    registry.register_handler(std::make_shared<TestHandler>());
    registry.register_alias("search", CapabilityId{"CAP-0102"});

    validation::ValidationEngine engine(registry);
    const auto request = PlatformRequest::create(CapabilityId{"search"}, InterfaceType::Internal);
    EXPECT_EQ(request.capability().value, "search");
    const auto result = engine.validate_request(request);
    EXPECT_TRUE(result.passed());
    EXPECT_EQ(
        engine.capability_validator().resolved_capability(request).value,
        "CAP-0102");
}

TEST(CapabilityValidatorTest, RejectsBrokenAlias) {
    capability::CapabilityRegistry registry;
    registry.register_alias("broken", CapabilityId{"CAP-4040"});

    validation::ValidationEngine engine(registry);
    const auto request = PlatformRequest::create(CapabilityId{"broken"}, InterfaceType::Internal);
    const auto result = engine.validate_request(request);
    EXPECT_TRUE(result.failed());
    EXPECT_EQ(result.stage, validation::ValidationStage::Capability);
}

TEST(CapabilityValidatorTest, RejectsDisabledCapability) {
    capability::CapabilityRegistry registry;
    class TestHandler final : public capability::ICapabilityHandler {
    public:
        [[nodiscard]] CapabilityId id() const override { return CapabilityId{"CAP-0001"}; }
        [[nodiscard]] std::string name() const override { return "Test"; }
        [[nodiscard]] std::expected<PlatformResponse, CapabilityError> execute(
            const PlatformRequest&) const override {
            return PlatformResponse::success(
                PlatformRequest::create(CapabilityId{"CAP-0001"}, InterfaceType::Internal),
                ResponseContent{.primary = "ok", .sections = {}, .structured = {}});
        }
    };
    registry.register_handler(std::make_shared<TestHandler>());

    validation::ValidationEngine engine(registry);
    engine.capability_validator().set_capability_enabled(CapabilityId{"CAP-0001"}, false);

    const auto request = PlatformRequest::create(CapabilityId{"CAP-0001"}, InterfaceType::Internal);
    const auto result = engine.validate_request(request);
    EXPECT_TRUE(result.failed());
    EXPECT_EQ(result.stage, validation::ValidationStage::Capability);
}

TEST(PermissionValidatorTest, RejectsInsufficientPermission) {
    capability::CapabilityRegistry registry;
    class TestHandler final : public capability::ICapabilityHandler {
    public:
        [[nodiscard]] CapabilityId id() const override { return CapabilityId{"CAP-0001"}; }
        [[nodiscard]] std::string name() const override { return "Test"; }
        [[nodiscard]] std::expected<PlatformResponse, CapabilityError> execute(
            const PlatformRequest&) const override {
            return PlatformResponse::success(
                PlatformRequest::create(CapabilityId{"CAP-0001"}, InterfaceType::Internal),
                ResponseContent{.primary = "ok", .sections = {}, .structured = {}});
        }
    };
    registry.register_handler(std::make_shared<TestHandler>());

    validation::PermissionValidator permission_validator;
    permission_validator.register_requirement(CapabilityId{"CAP-0001"}, PermissionLevel::Administrator);

    validation::ValidationEngine engine(registry, {}, permission_validator);
    const auto request = PlatformRequest::create(
        CapabilityId{"CAP-0001"},
        InterfaceType::Internal,
        {},
        {},
        {},
        PermissionInfo{.level = PermissionLevel::Public});

    const auto result = engine.validate_request(request);
    EXPECT_TRUE(result.failed());
    EXPECT_EQ(result.stage, validation::ValidationStage::Permission);
}

TEST(ParameterValidatorTest, UsesRegisteredSchema) {
    capability::CapabilityRegistry registry;
    class TestHandler final : public capability::ICapabilityHandler {
    public:
        [[nodiscard]] CapabilityId id() const override { return CapabilityId{"CAP-0102"}; }
        [[nodiscard]] std::string name() const override { return "Search"; }
        [[nodiscard]] std::expected<PlatformResponse, CapabilityError> execute(
            const PlatformRequest&) const override {
            return PlatformResponse::success(
                PlatformRequest::create(CapabilityId{"CAP-0102"}, InterfaceType::Internal),
                ResponseContent{.primary = "ok", .sections = {}, .structured = {}});
        }
    };
    registry.register_handler(std::make_shared<TestHandler>());

    validation::ParameterValidator parameter_validator;
    parameter_validator.register_schema(CapabilityId{"CAP-0102"}, std::make_shared<RequiredQuerySchema>());

    validation::ValidationEngine engine(registry, parameter_validator);
    const auto missing_query = PlatformRequest::create(CapabilityId{"CAP-0102"}, InterfaceType::Internal);
    const auto result = engine.validate_request(missing_query);
    EXPECT_TRUE(result.failed());
    EXPECT_EQ(result.stage, validation::ValidationStage::Parameter);
}

TEST(ResponseValidatorTest, RejectsEmptyPayload) {
    const auto request = PlatformRequest::create(CapabilityId{"CAP-0001"}, InterfaceType::Internal);
    const auto response = PlatformResponse::success(
        request,
        ResponseContent{.primary = "", .sections = {}, .structured = {}});

    validation::ValidationEngine engine(make_test_registry());
    const auto result = engine.validate_response(response);
    EXPECT_TRUE(result.failed());
    EXPECT_EQ(result.stage, validation::ValidationStage::Response);
}

TEST(ValidationEngineTest, PipelineOrderingStopsAtFirstFailure) {
    const capability::CapabilityRegistry registry;
    validation::ValidationEngine engine(registry);

    const auto request = PlatformRequest::create(CapabilityId{}, InterfaceType::Internal);
    const auto result = engine.validate_request(request);
    EXPECT_TRUE(result.failed());
    EXPECT_EQ(result.stage, validation::ValidationStage::Structural);
}

TEST(ValidationEngineTest, PipelineReachesCapabilityAfterStructuralPass) {
    const capability::CapabilityRegistry registry;
    validation::ValidationEngine engine(registry);

    const auto request = PlatformRequest::create(CapabilityId{"CAP-9999"}, InterfaceType::Internal);
    const auto result = engine.validate_request(request);
    EXPECT_TRUE(result.failed());
    EXPECT_EQ(result.stage, validation::ValidationStage::Capability);
}

}  // namespace
}  // namespace eve
