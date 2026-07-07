#pragma once

#include "eve/core/errors.hpp"
#include "eve/core/types.hpp"
#include "eve/validation/validation_result.hpp"

#include <expected>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace eve {

namespace validation {
class ValidationEngine;
}

struct RequestMetadata {
    RequestId request_id;
    TimePoint timestamp{};
    std::string request_version{"1.0.0"};
    CorrelationId correlation_id;
};

struct UserInfo {
    std::optional<std::string> user_id;
    std::optional<std::string> session_id;
    std::optional<std::string> organization;
    std::optional<std::string> locale;
};

struct InterfaceInfo {
    InterfaceType type{InterfaceType::Internal};
    std::optional<std::string> channel;
    std::optional<std::string> origin;
};

struct ExecutionContext {
    std::vector<std::string> conversation_refs;
    std::vector<std::string> referenced_documents;
    std::optional<std::string> repository_selection;
    std::optional<std::string> current_project;
    std::optional<std::string> working_session;
};

using ParameterMap = std::map<std::string, std::string>;

struct PermissionInfo {
    PermissionLevel level{PermissionLevel::Public};
    std::vector<std::string> roles;
};

struct TraceInformation {
    std::optional<std::string> origin;
    std::vector<TraceEntry> processing_history;
    std::optional<std::int64_t> elapsed_ms;
};

class PlatformRequest {
public:
    [[nodiscard]] static PlatformRequest create(
        CapabilityId capability,
        InterfaceType interface_type,
        ParameterMap parameters = {},
        ExecutionContext execution_context = {},
        UserInfo user = {},
        PermissionInfo permissions = {});

    [[nodiscard]] const RequestMetadata& metadata() const noexcept { return metadata_; }
    [[nodiscard]] const InterfaceInfo& interface_info() const noexcept { return interface_; }
    [[nodiscard]] const UserInfo& user() const noexcept { return user_; }
    [[nodiscard]] const CapabilityId& capability() const noexcept { return capability_; }
    [[nodiscard]] const ExecutionContext& execution_context() const noexcept {
        return execution_context_;
    }
    [[nodiscard]] const ParameterMap& parameters() const noexcept { return parameters_; }
    [[nodiscard]] const PermissionInfo& permissions() const noexcept { return permissions_; }
    [[nodiscard]] const TraceInformation& trace() const noexcept { return trace_; }

    [[nodiscard]] std::optional<std::string> parameter(std::string_view key) const;
    [[nodiscard]] std::string debug_string() const;

private:
    PlatformRequest() = default;

    RequestMetadata metadata_{};
    InterfaceInfo interface_{};
    UserInfo user_{};
    CapabilityId capability_{};
    ExecutionContext execution_context_{};
    ParameterMap parameters_{};
    PermissionInfo permissions_{};
    TraceInformation trace_{};
};

[[nodiscard]] std::optional<ValidationError> validate_platform_request(
    const PlatformRequest& request);

/// Runs structural validation only. Prefer ValidationEngine for full pipeline validation.
[[nodiscard]] validation::ValidationResult validate_platform_request_structural(
    const PlatformRequest& request);

class ValidatedPlatformRequest {
public:
    [[nodiscard]] static std::expected<ValidatedPlatformRequest, validation::ValidationResult>
    from(PlatformRequest request, const validation::ValidationEngine& engine);

    /// Structural validation only. Prefer from(request, engine) for full pipeline validation.
    [[nodiscard]] static std::expected<ValidatedPlatformRequest, ValidationError> from(
        PlatformRequest request);

    /// Adopts a request that has already passed validate_platform_request().
    [[nodiscard]] static ValidatedPlatformRequest adopt(PlatformRequest request);

    [[nodiscard]] const PlatformRequest& request() const noexcept { return request_; }

    [[nodiscard]] const RequestMetadata& metadata() const noexcept {
        return request_.metadata();
    }
    [[nodiscard]] const InterfaceInfo& interface_info() const noexcept {
        return request_.interface_info();
    }
    [[nodiscard]] const UserInfo& user() const noexcept { return request_.user(); }
    [[nodiscard]] const CapabilityId& capability() const noexcept {
        return request_.capability();
    }
    [[nodiscard]] const ExecutionContext& execution_context() const noexcept {
        return request_.execution_context();
    }
    [[nodiscard]] const ParameterMap& parameters() const noexcept {
        return request_.parameters();
    }
    [[nodiscard]] const PermissionInfo& permissions() const noexcept {
        return request_.permissions();
    }
    [[nodiscard]] const TraceInformation& trace() const noexcept { return request_.trace(); }

    [[nodiscard]] std::optional<std::string> parameter(std::string_view key) const {
        return request_.parameter(key);
    }

private:
    explicit ValidatedPlatformRequest(PlatformRequest request);

    PlatformRequest request_;
};

}  // namespace eve
