#include "eve/capability/capability_engine.hpp"

#include <memory>

namespace eve::capability {

void CapabilityRegistry::register_handler(std::shared_ptr<ICapabilityHandler> handler) {
    handlers_[handler->id().value] = std::move(handler);
}

void CapabilityRegistry::register_alias(std::string alias, CapabilityId target) {
    aliases_[std::move(alias)] = std::move(target);
}

CapabilityId CapabilityRegistry::resolve_capability(CapabilityId capability) const {
    if (const auto alias = aliases_.find(capability.value); alias != aliases_.end()) {
        return alias->second;
    }
    return capability;
}

std::shared_ptr<ICapabilityHandler> CapabilityRegistry::resolve(CapabilityId capability) const {
    capability = resolve_capability(capability);
    const auto iterator = handlers_.find(capability.value);
    if (iterator == handlers_.end()) {
        return nullptr;
    }
    return iterator->second;
}

std::vector<CapabilityId> CapabilityRegistry::list_capabilities() const {
    std::vector<CapabilityId> capabilities;
    capabilities.reserve(handlers_.size());
    for (const auto& [id, handler] : handlers_) {
        (void)handler;
        capabilities.push_back(CapabilityId{id});
    }
    return capabilities;
}

CapabilityEngine::CapabilityEngine(std::shared_ptr<CapabilityRegistry> registry)
    : registry_(std::move(registry)) {}

CapabilityEngine::CapabilityEngine(CapabilityRegistry registry)
    : registry_(std::make_shared<CapabilityRegistry>(std::move(registry))) {}

std::expected<PlatformResponse, CapabilityError> CapabilityEngine::execute(
    const PlatformRequest& request) const {
    const auto handler = registry_->resolve(request.capability());
    if (!handler) {
        return std::unexpected(CapabilityError{
            .capability_id = request.capability().value,
            .message = "Unsupported capability.",
        });
    }
    return handler->execute(request);
}

}  // namespace eve::capability
