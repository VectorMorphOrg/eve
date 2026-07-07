#pragma once

#include "eve/core/platform_request.hpp"
#include "eve/core/platform_response.hpp"

#include <expected>
#include <map>
#include <memory>
#include <string>

namespace eve::capability {

class ICapabilityHandler {
public:
    virtual ~ICapabilityHandler() = default;

    [[nodiscard]] virtual CapabilityId id() const = 0;
    [[nodiscard]] virtual std::string name() const = 0;
    [[nodiscard]] virtual std::expected<PlatformResponse, CapabilityError> execute(
        const PlatformRequest& request) const = 0;
};

class CapabilityRegistry {
public:
    void register_handler(std::shared_ptr<ICapabilityHandler> handler);
    void register_alias(std::string alias, CapabilityId target);
    [[nodiscard]] CapabilityId resolve_capability(CapabilityId capability) const;
    [[nodiscard]] std::shared_ptr<ICapabilityHandler> resolve(
        CapabilityId capability) const;
    [[nodiscard]] std::vector<CapabilityId> list_capabilities() const;

private:
    std::map<std::string, std::shared_ptr<ICapabilityHandler>> handlers_;
    std::map<std::string, CapabilityId> aliases_;
};

class CapabilityEngine {
public:
    explicit CapabilityEngine(std::shared_ptr<CapabilityRegistry> registry);
    explicit CapabilityEngine(CapabilityRegistry registry);

    [[nodiscard]] const CapabilityRegistry& registry() const noexcept { return *registry_; }
    [[nodiscard]] std::expected<PlatformResponse, CapabilityError> execute(
        const PlatformRequest& request) const;

private:
    std::shared_ptr<CapabilityRegistry> registry_;
};

}  // namespace eve::capability
