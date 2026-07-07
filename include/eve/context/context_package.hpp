#pragma once

#include "eve/core/platform_request.hpp"
#include "eve/knowledge/knowledge_object.hpp"

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace eve::context {

struct PackageMetadata {
    PackageId package_id;
    RequestId request_id;
    TimePoint timestamp{};
    std::string specification_version{"1.1.0"};
    std::string builder_version{"0.1.0"};
};

struct RepositoryMetadata {
    std::string name;
    std::string branch{"main"};
    std::optional<std::string> commit;
    std::optional<std::string> version;
    std::optional<std::string> last_updated;
};

struct Citation {
    std::string identifier;
    std::string title;
    std::optional<std::string> section;
};

struct ConversationContext {
    std::optional<std::string> session_summary;
    std::vector<std::string> recent_messages;
};

struct ContextConstraints {
    bool documentation_first{true};
    bool preserve_citations{true};
    bool do_not_invent_information{true};
    std::optional<std::size_t> response_length;
    std::optional<std::string> output_format;
};

struct SystemInstructions {
    std::optional<std::string> personality_profile;
    std::optional<std::string> language;
    std::optional<std::string> ai_provider;
    std::map<std::string, std::string> provider_options;
};

struct PackageDiagnostics {
    std::vector<DiagnosticMessage> warnings;
    std::vector<std::string> missing_documentation;
    std::vector<std::string> builder_notes;
};

class ContextPackage {
public:
    [[nodiscard]] static ContextPackage create(
        const PlatformRequest& request,
        std::vector<KnowledgeObject> knowledge_objects,
        std::vector<RepositoryMetadata> repositories,
        std::vector<Citation> citations,
        ContextConstraints constraints,
        SystemInstructions system,
        PackageDiagnostics diagnostics = {});

    [[nodiscard]] const PackageMetadata& metadata() const noexcept { return metadata_; }
    [[nodiscard]] const PlatformRequest& request() const noexcept { return request_; }
    [[nodiscard]] CapabilityId capability() const noexcept { return request_.capability(); }
    [[nodiscard]] const std::vector<RepositoryMetadata>& repositories() const noexcept {
        return repositories_;
    }
    [[nodiscard]] const std::vector<KnowledgeObject>& knowledge_objects() const noexcept {
        return knowledge_objects_;
    }
    [[nodiscard]] const std::vector<Citation>& citations() const noexcept { return citations_; }
    [[nodiscard]] const std::optional<ConversationContext>& conversation() const noexcept {
        return conversation_;
    }
    [[nodiscard]] const ContextConstraints& constraints() const noexcept { return constraints_; }
    [[nodiscard]] const SystemInstructions& system() const noexcept { return system_; }
    [[nodiscard]] const PackageDiagnostics& diagnostics() const noexcept { return diagnostics_; }

    [[nodiscard]] std::string debug_string() const;

private:
    ContextPackage(
        PackageMetadata metadata,
        PlatformRequest request,
        std::vector<KnowledgeObject> knowledge_objects,
        std::vector<RepositoryMetadata> repositories,
        std::vector<Citation> citations,
        std::optional<ConversationContext> conversation,
        ContextConstraints constraints,
        SystemInstructions system,
        PackageDiagnostics diagnostics);

    PackageMetadata metadata_{};
    PlatformRequest request_;
    std::vector<RepositoryMetadata> repositories_{};
    std::vector<KnowledgeObject> knowledge_objects_{};
    std::vector<Citation> citations_{};
    std::optional<ConversationContext> conversation_{};
    ContextConstraints constraints_{};
    SystemInstructions system_{};
    PackageDiagnostics diagnostics_{};
};

struct ContextValidationError {
    std::string message;
};

[[nodiscard]] std::optional<ContextValidationError> validate_context_package(
    const ContextPackage& package);

class ContextBuilder {
public:
    explicit ContextBuilder(std::size_t context_limit_chars = 32000);

    [[nodiscard]] ContextPackage build(
        const PlatformRequest& request,
        const std::vector<KnowledgeObject>& knowledge_objects,
        SystemInstructions system = {}) const;

private:
    std::size_t context_limit_chars_;

    [[nodiscard]] std::vector<KnowledgeObject> rank_and_limit(
        std::vector<KnowledgeObject> objects) const;
    [[nodiscard]] std::vector<Citation> build_citations(
        const std::vector<KnowledgeObject>& objects) const;
    [[nodiscard]] std::vector<RepositoryMetadata> build_repositories(
        const std::vector<KnowledgeObject>& objects) const;
};

}  // namespace eve::context
