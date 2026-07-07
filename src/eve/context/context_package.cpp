#include "eve/context/context_package.hpp"

#include <algorithm>
#include <format>
#include <unordered_set>

namespace eve::context {

ContextPackage::ContextPackage(
    PackageMetadata metadata,
    PlatformRequest request,
    std::vector<KnowledgeObject> knowledge_objects,
    std::vector<RepositoryMetadata> repositories,
    std::vector<Citation> citations,
    std::optional<ConversationContext> conversation,
    ContextConstraints constraints,
    SystemInstructions system,
    PackageDiagnostics diagnostics)
    : metadata_(std::move(metadata)),
      request_(std::move(request)),
      repositories_(std::move(repositories)),
      knowledge_objects_(std::move(knowledge_objects)),
      citations_(std::move(citations)),
      conversation_(std::move(conversation)),
      constraints_(std::move(constraints)),
      system_(std::move(system)),
      diagnostics_(std::move(diagnostics)) {}

ContextPackage ContextPackage::create(
    const PlatformRequest& request,
    std::vector<KnowledgeObject> knowledge_objects,
    std::vector<RepositoryMetadata> repositories,
    std::vector<Citation> citations,
    ContextConstraints constraints,
    SystemInstructions system,
    PackageDiagnostics diagnostics) {
    PackageMetadata metadata{
        .package_id = generate_package_id(),
        .request_id = request.metadata().request_id,
        .timestamp = Clock::now(),
    };
    return ContextPackage(
        std::move(metadata),
        request,
        std::move(knowledge_objects),
        std::move(repositories),
        std::move(citations),
        std::nullopt,
        std::move(constraints),
        std::move(system),
        std::move(diagnostics));
}

std::string ContextPackage::debug_string() const {
    return std::format(
        "ContextPackage{{id={}, capability={}, knowledge_objects={}}}",
        metadata_.package_id.value,
        request_.capability().value,
        knowledge_objects_.size());
}

std::optional<ContextValidationError> validate_context_package(const ContextPackage& package) {
    if (!package.capability()) {
        return ContextValidationError{"Context package is missing a capability."};
    }
    if (package.knowledge_objects().empty() && package.diagnostics().missing_documentation.empty()) {
        return ContextValidationError{"Context package contains no knowledge and no diagnostics."};
    }
    return std::nullopt;
}

ContextBuilder::ContextBuilder(std::size_t context_limit_chars)
    : context_limit_chars_(context_limit_chars) {}

std::vector<KnowledgeObject> ContextBuilder::rank_and_limit(
    std::vector<KnowledgeObject> objects) const {
    std::sort(objects.begin(), objects.end(), [](const KnowledgeObject& lhs, const KnowledgeObject& rhs) {
        if (lhs.search.priority_rank != rhs.search.priority_rank) {
            return lhs.search.priority_rank < rhs.search.priority_rank;
        }
        return lhs.identity.title < rhs.identity.title;
    });

    std::vector<KnowledgeObject> selected;
    std::unordered_set<std::string> seen_titles;
    std::size_t total_chars = 0;

    for (auto& object : objects) {
        if (seen_titles.contains(object.identity.title)) {
            continue;
        }

        const std::size_t object_size = object.search.excerpt.size();
        if (!selected.empty() && total_chars + object_size > context_limit_chars_) {
            break;
        }

        seen_titles.insert(object.identity.title);
        total_chars += object_size;
        selected.push_back(std::move(object));
    }

    return selected;
}

std::vector<Citation> ContextBuilder::build_citations(
    const std::vector<KnowledgeObject>& objects) const {
    std::vector<Citation> citations;
    for (const auto& object : objects) {
        citations.push_back(Citation{
            .identifier = object.identity.id.value,
            .title = object.identity.title,
            .section = std::nullopt,
        });
    }
    return citations;
}

std::vector<RepositoryMetadata> ContextBuilder::build_repositories(
    const std::vector<KnowledgeObject>& objects) const {
    std::vector<RepositoryMetadata> repositories;
    std::unordered_set<std::string> seen;

    for (const auto& object : objects) {
        if (!seen.insert(object.identity.repository).second) {
            continue;
        }
        repositories.push_back(RepositoryMetadata{
            .name = object.identity.repository,
            .last_updated = object.source.last_modified,
        });
    }
    return repositories;
}

ContextPackage ContextBuilder::build(
    const PlatformRequest& request,
    const std::vector<KnowledgeObject>& knowledge_objects,
    SystemInstructions system) const {
    auto ranked = rank_and_limit(knowledge_objects);
    PackageDiagnostics diagnostics;
    if (ranked.empty()) {
        diagnostics.missing_documentation.push_back(
            "No documented knowledge was available for this request.");
    }

    const auto repositories = build_repositories(knowledge_objects);
    const auto citations = build_citations(ranked);

    ContextConstraints constraints{
        .documentation_first = true,
        .preserve_citations = true,
        .do_not_invent_information = true,
        .response_length = context_limit_chars_,
    };

    return ContextPackage::create(
        request,
        std::move(ranked),
        repositories,
        citations,
        constraints,
        std::move(system),
        std::move(diagnostics));
}

}  // namespace eve::context
