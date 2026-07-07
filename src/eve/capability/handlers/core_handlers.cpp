#include "eve/capability/handlers/capability_handlers.hpp"

#include <format>
#include <sstream>

namespace eve::capability::handlers {
namespace {

class RequestProcessingHandler final : public ICapabilityHandler {
public:
    [[nodiscard]] CapabilityId id() const override { return CapabilityId{"CAP-0001"}; }
    [[nodiscard]] std::string name() const override { return "Request Processing"; }

    [[nodiscard]] std::expected<PlatformResponse, CapabilityError> execute(
        const PlatformRequest& request) const override {
        ResponseContent content{
            .primary = std::format(
                "Request {} accepted for capability {}.",
                request.metadata().request_id.value,
                request.capability().value),
        };
        return PlatformResponse::success(request, std::move(content));
    }
};

class ResponseGenerationHandler final : public ICapabilityHandler {
public:
    [[nodiscard]] CapabilityId id() const override { return CapabilityId{"CAP-0002"}; }
    [[nodiscard]] std::string name() const override { return "Response Generation"; }

    [[nodiscard]] std::expected<PlatformResponse, CapabilityError> execute(
        const PlatformRequest& request) const override {
        return PlatformResponse::success(
            request,
            ResponseContent{.primary = "Response generation capability is available."});
    }
};

class ContextConstructionHandler final : public ICapabilityHandler {
public:
    explicit ContextConstructionHandler(const context::ContextBuilder& context_builder)
        : context_builder_(context_builder) {}

    [[nodiscard]] CapabilityId id() const override { return CapabilityId{"CAP-0003"}; }
    [[nodiscard]] std::string name() const override { return "Context Construction"; }

    [[nodiscard]] std::expected<PlatformResponse, CapabilityError> execute(
        const PlatformRequest& request) const override {
        const auto package = context_builder_.build(request, {});
        return PlatformResponse::success(
            request,
            ResponseContent{.primary = std::format(
                                "Context package {} created with {} knowledge object(s).",
                                package.metadata().package_id.value,
                                package.knowledge_objects().size())});
    }

private:
    const context::ContextBuilder& context_builder_;
};

class CapabilityRoutingHandler final : public ICapabilityHandler {
public:
    explicit CapabilityRoutingHandler(const CapabilityRegistry& registry)
        : registry_(registry) {}

    [[nodiscard]] CapabilityId id() const override { return CapabilityId{"CAP-0004"}; }
    [[nodiscard]] std::string name() const override { return "Capability Routing"; }

    [[nodiscard]] std::expected<PlatformResponse, CapabilityError> execute(
        const PlatformRequest& request) const override {
        std::ostringstream stream;
        stream << "Registered capabilities:\n";
        for (const auto& capability : registry_.list_capabilities()) {
            stream << "- " << capability.value << '\n';
        }
        return PlatformResponse::success(
            request,
            ResponseContent{.primary = stream.str()});
    }

private:
    const CapabilityRegistry& registry_;
};

class PermissionValidationHandler final : public ICapabilityHandler {
public:
    [[nodiscard]] CapabilityId id() const override { return CapabilityId{"CAP-0005"}; }
    [[nodiscard]] std::string name() const override { return "Permission Validation"; }

    [[nodiscard]] std::expected<PlatformResponse, CapabilityError> execute(
        const PlatformRequest& request) const override {
        return PlatformResponse::success(
            request,
            ResponseContent{.primary = std::format(
                                "Permission level '{}' validated.",
                                to_string(request.permissions().level))});
    }
};

}  // namespace

void register_core_handlers(
    CapabilityRegistry& registry,
    const services::IConfigurationService& configuration,
    const services::IStatusService& status,
    const services::IDiagnosticsService& diagnostics,
    const context::ContextBuilder& context_builder) {
    (void)configuration;
    (void)status;
    (void)diagnostics;

    registry.register_handler(std::make_shared<RequestProcessingHandler>());
    registry.register_handler(std::make_shared<ResponseGenerationHandler>());
    registry.register_handler(std::make_shared<ContextConstructionHandler>(context_builder));
    registry.register_handler(std::make_shared<CapabilityRoutingHandler>(registry));
    registry.register_handler(std::make_shared<PermissionValidationHandler>());
}

}  // namespace eve::capability::handlers
