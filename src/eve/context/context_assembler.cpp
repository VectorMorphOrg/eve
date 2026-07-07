#include "eve/context/context_assembler.hpp"

#include <algorithm>
#include <unordered_set>

namespace eve::context {
namespace {

const CitationBundle* find_bundle(const CitationResultSet& citations, std::string_view object_id) {
    for (const auto& bundle : citations.bundles) {
        if (bundle.ranked_result.candidate.object_identifier == object_id) {
            return &bundle;
        }
    }
    return nullptr;
}

const KnowledgeObject* resolve_object(
    const KnowledgeIndex& index,
    const KnowledgeGraph& graph,
    std::string_view object_id) {
    const auto by_id = index.lookup_by_object_id(object_id);
    if (!by_id.empty()) {
        return by_id.front();
    }

    const auto by_identifier = index.lookup_by_document_identifier(object_id);
    if (!by_identifier.empty()) {
        return by_identifier.front();
    }

    return graph.find_object_by_id(KnowledgeObjectId{std::string{object_id}});
}

Citation to_context_citation(const eve::Citation& citation) {
    context::Citation mapped{
        .identifier = citation.document_identifier.empty() ? citation.object_identifier
                                                            : citation.document_identifier,
        .title = citation.title,
    };
    if (!citation.supporting_section_titles.empty()) {
        mapped.section = citation.supporting_section_titles.front();
    }
    return mapped;
}

AssembledDocumentMetadata metadata_from_object(const KnowledgeObject& object) {
    return AssembledDocumentMetadata{
        .object_identifier = object.identity.id.value,
        .repository = object.identity.repository,
        .series = object.metadata.series,
        .part = object.metadata.part,
        .status = object.metadata.status,
        .document_type = object.identity.document_type,
    };
}

NavigationContextEntry navigation_from_object(const KnowledgeObject& object) {
    return NavigationContextEntry{
        .object_identifier = object.identity.id.value,
        .previous = object.relationships.previous,
        .next = object.relationships.next,
        .parent = object.relationships.parent,
        .children = object.relationships.children,
    };
}

RepositoryMetadata repository_from_object(const KnowledgeObject& object) {
    return RepositoryMetadata{
        .name = object.identity.repository,
        .last_updated = object.source.last_modified,
    };
}

class BudgetTracker {
public:
    explicit BudgetTracker(ContextBudget limits) : limits_(limits) {}

    [[nodiscard]] bool truncated() const noexcept { return utilization_.budget_truncated; }
    [[nodiscard]] const ContextBudgetUtilization& utilization() const noexcept {
        return utilization_;
    }

    [[nodiscard]] bool can_add_document() const {
        return utilization_.selected_documents < limits_.max_documents;
    }

    [[nodiscard]] bool can_add_section() const {
        return utilization_.selected_sections < limits_.max_sections;
    }

    [[nodiscard]] bool can_add_heading() const {
        return utilization_.selected_headings < limits_.max_headings;
    }

    [[nodiscard]] bool can_add_implementation_file() const {
        return utilization_.selected_implementation_files < limits_.max_implementation_files;
    }

    [[nodiscard]] bool can_add_test_file() const {
        return utilization_.selected_test_files < limits_.max_test_files;
    }

    [[nodiscard]] bool can_add_reference() const {
        return utilization_.selected_references < limits_.max_references;
    }

    [[nodiscard]] bool try_add_tokens(std::size_t tokens) {
        if (utilization_.estimated_tokens + tokens > limits_.max_estimated_tokens) {
            mark_truncated();
            return false;
        }
        utilization_.estimated_tokens += tokens;
        return true;
    }

    [[nodiscard]] bool try_add_document(const KnowledgeObject& object) {
        if (!can_add_document()) {
            mark_truncated();
            return false;
        }

        const std::string_view token_source = object.search.excerpt.empty()
            ? std::string_view{object.identity.title}
            : std::string_view{object.search.excerpt};
        if (!try_add_tokens(estimate_tokens(token_source))) {
            return false;
        }

        ++utilization_.selected_documents;
        return true;
    }

    [[nodiscard]] bool try_add_section(const SelectedSection& section) {
        if (!can_add_section()) {
            mark_truncated();
            return false;
        }

        const auto tokens = estimate_tokens(section.title) + estimate_tokens(section.content);
        if (!try_add_tokens(tokens)) {
            return false;
        }

        ++utilization_.selected_sections;
        return true;
    }

    [[nodiscard]] bool try_add_heading(const SelectedHeading& heading) {
        if (!can_add_heading()) {
            mark_truncated();
            return false;
        }

        if (!try_add_tokens(estimate_tokens(heading.text))) {
            return false;
        }

        ++utilization_.selected_headings;
        return true;
    }

    [[nodiscard]] bool try_add_implementation_header(std::string_view path) {
        if (!can_add_implementation_file()) {
            mark_truncated();
            return false;
        }

        if (!try_add_tokens(estimate_tokens(path))) {
            return false;
        }

        ++utilization_.selected_implementation_files;
        return true;
    }

    [[nodiscard]] bool try_add_implementation_source(std::string_view path) {
        if (!can_add_implementation_file()) {
            mark_truncated();
            return false;
        }

        if (!try_add_tokens(estimate_tokens(path))) {
            return false;
        }

        ++utilization_.selected_implementation_files;
        return true;
    }

    [[nodiscard]] bool try_add_test_file(std::string_view path) {
        if (!can_add_test_file()) {
            mark_truncated();
            return false;
        }

        if (!try_add_tokens(estimate_tokens(path))) {
            return false;
        }

        ++utilization_.selected_test_files;
        return true;
    }

    [[nodiscard]] bool try_add_reference(std::string_view reference) {
        if (!can_add_reference()) {
            mark_truncated();
            return false;
        }

        if (!try_add_tokens(estimate_tokens(reference))) {
            return false;
        }

        ++utilization_.selected_references;
        return true;
    }

    void mark_truncated() { utilization_.budget_truncated = true; }

private:
    ContextBudget limits_;
    ContextBudgetUtilization utilization_{};
};

bool add_unique_document(
    const KnowledgeObject& object,
    BudgetTracker& tracker,
    std::vector<KnowledgeObject>& selected_objects,
    std::unordered_set<std::string>& selected_doc_ids,
    std::vector<AssembledDocumentMetadata>& metadata) {
    if (selected_doc_ids.contains(object.identity.id.value)) {
        return true;
    }

    if (!tracker.try_add_document(object)) {
        return false;
    }

    selected_doc_ids.insert(object.identity.id.value);
    metadata.push_back(metadata_from_object(object));
    selected_objects.push_back(object);
    return true;
}

void append_unique(std::vector<std::string>& values, std::unordered_set<std::string>& seen, std::string value) {
    if (!seen.insert(value).second) {
        return;
    }
    values.push_back(std::move(value));
}

void append_package_citation(
    std::vector<Citation>& citations,
    std::unordered_set<std::string>& seen,
    const eve::Citation& citation) {
    const auto identifier = citation.document_identifier.empty() ? citation.object_identifier
                                                                   : citation.document_identifier;
    if (!seen.insert(identifier).second) {
        return;
    }
    citations.push_back(to_context_citation(citation));
}

std::string implementation_path(const eve::Citation& citation) {
    if (!citation.supporting_source_files.empty()) {
        return citation.supporting_source_files.front();
    }
    return citation.object_identifier;
}

}  // namespace

std::string to_string(ContextSelectionPolicy policy) {
    switch (policy) {
        case ContextSelectionPolicy::Full:
            return "Full";
        case ContextSelectionPolicy::Balanced:
            return "Balanced";
        case ContextSelectionPolicy::Minimal:
            return "Minimal";
    }
    return "Unknown";
}

ContextBudget effective_budget_for_policy(ContextBudget budget, ContextSelectionPolicy policy) {
    if (policy == ContextSelectionPolicy::Minimal) {
        return ContextBudget{
            .max_documents = std::min(budget.max_documents, std::size_t{1}),
            .max_sections = std::min(budget.max_sections, std::size_t{1}),
            .max_headings = std::min(budget.max_headings, std::size_t{1}),
            .max_implementation_files = 0,
            .max_test_files = 0,
            .max_references = std::min(budget.max_references, std::size_t{3}),
            .max_estimated_tokens = budget.max_estimated_tokens,
        };
    }

    if (policy == ContextSelectionPolicy::Balanced) {
        auto balanced = budget;
        balanced.max_implementation_files =
            std::min(budget.max_implementation_files, budget.max_documents);
        balanced.max_test_files = std::min(budget.max_test_files, budget.max_documents);
        return balanced;
    }

    return budget;
}

std::size_t estimate_tokens(std::string_view text) {
    return (text.size() + 3) / 4;
}

ContextAssembler::ContextAssembler(const KnowledgeIndex& index, const KnowledgeGraph& graph)
    : index_(&index), graph_(&graph) {}

const KnowledgeIndex& ContextAssembler::index() const noexcept {
    return *index_;
}

const KnowledgeGraph& ContextAssembler::graph() const noexcept {
    return *graph_;
}

ContextAssemblyResult ContextAssembler::assemble(const ContextAssemblyRequest& request) const {
    const auto effective_budget = effective_budget_for_policy(request.budget, request.policy);
    BudgetTracker tracker(effective_budget);

    ContextAssemblyEvidence evidence{
        .identity =
            ContextAssemblyIdentity{
                .request_id = request.platform_request.metadata().request_id,
                .correlation_id = request.platform_request.metadata().correlation_id,
            },
    };

    std::vector<KnowledgeObject> selected_objects;
    std::unordered_set<std::string> selected_doc_ids;
    std::unordered_set<std::string> seen_sections;
    std::unordered_set<std::string> seen_headings;
    std::unordered_set<std::string> seen_impl_headers;
    std::unordered_set<std::string> seen_impl_sources;
    std::unordered_set<std::string> seen_tests;
    std::unordered_set<std::string> seen_references;
    std::unordered_set<std::string> seen_navigation;
    std::unordered_set<std::string> seen_citations;
    std::unordered_set<std::string> seen_repositories;

    std::vector<Citation> package_citations;
    std::vector<RepositoryMetadata> repositories;
    PackageDiagnostics diagnostics;

    struct DeferredImplementation {
        const CitationBundle* bundle{nullptr};
    };
    std::vector<DeferredImplementation> deferred_implementation;

    std::size_t processed_ranked_results = 0;

    for (const auto& ranked_result : request.ranked_results.results) {
        if (request.policy == ContextSelectionPolicy::Minimal && processed_ranked_results >= 1) {
            diagnostics.builder_notes.push_back(
                "Minimal policy skipped lower-ranked search results.");
            break;
        }

        const auto& object_id = ranked_result.candidate.object_identifier;
        const auto* bundle = find_bundle(request.citations, object_id);
        if (bundle == nullptr) {
            diagnostics.missing_documentation.push_back(std::string(object_id));
            continue;
        }

        const auto* object = resolve_object(*index_, *graph_, object_id);
        if (object == nullptr) {
            diagnostics.missing_documentation.push_back(std::string(object_id));
            continue;
        }

        if (!tracker.can_add_document()) {
            tracker.mark_truncated();
            diagnostics.builder_notes.push_back(
                "Budget truncation: document limit reached before " + std::string(object_id) + ".");
            break;
        }

        if (!add_unique_document(
                *object,
                tracker,
                selected_objects,
                selected_doc_ids,
                evidence.document_metadata)) {
            diagnostics.builder_notes.push_back("Budget truncation: skipped document " + std::string(object_id) + ".");
            continue;
        }

        evidence.ranked_results.push_back(ranked_result);
        evidence.citation_bundles.push_back(*bundle);
        append_package_citation(package_citations, seen_citations, bundle->primary);
        ++processed_ranked_results;

        if (const auto repo = repository_from_object(*object);
            seen_repositories.insert(repo.name).second) {
            repositories.push_back(std::move(repo));
        }

        for (const auto& related : bundle->related_documents) {
            if (related.document_type != DocumentType::Specification) {
                continue;
            }

            const auto* supporting = resolve_object(*index_, *graph_, related.object_identifier);
            if (supporting == nullptr) {
                diagnostics.missing_documentation.push_back(related.object_identifier);
                continue;
            }

            if (!add_unique_document(
                    *supporting,
                    tracker,
                    selected_objects,
                    selected_doc_ids,
                    evidence.document_metadata)) {
                tracker.mark_truncated();
                break;
            }

            append_package_citation(package_citations, seen_citations, related);
        }

        for (const auto& section : object->content.sections) {
            if (!tracker.can_add_section()) {
                tracker.mark_truncated();
                break;
            }

            const std::string section_key = object_id + "|" + section.title;
            if (seen_sections.contains(section_key)) {
                continue;
            }

            SelectedSection selected{
                .object_identifier = std::string(object_id),
                .title = section.title,
                .content = section.content,
            };
            if (!tracker.try_add_section(selected)) {
                break;
            }

            seen_sections.insert(section_key);
            evidence.sections.push_back(std::move(selected));
        }

        for (const auto& heading : object->content.headings) {
            if (!tracker.can_add_heading()) {
                tracker.mark_truncated();
                break;
            }

            const std::string heading_key = object_id + "|" + heading.text;
            if (seen_headings.contains(heading_key)) {
                continue;
            }

            SelectedHeading selected{
                .object_identifier = std::string(object_id),
                .text = heading.text,
            };
            if (!tracker.try_add_heading(selected)) {
                break;
            }

            seen_headings.insert(heading_key);
            evidence.headings.push_back(std::move(selected));
        }

        if (request.policy == ContextSelectionPolicy::Balanced) {
            deferred_implementation.push_back(DeferredImplementation{.bundle = bundle});
        } else if (request.policy != ContextSelectionPolicy::Minimal) {
            for (const auto& implementation : bundle->implementation) {
                if (implementation.citation_type == CitationType::ImplementationHeader) {
                    const auto path = implementation_path(implementation);
                    if (!tracker.try_add_implementation_header(path)) {
                        break;
                    }
                    append_unique(evidence.implementation_headers, seen_impl_headers, path);
                } else if (implementation.citation_type == CitationType::ImplementationSource) {
                    const auto path = implementation_path(implementation);
                    if (!tracker.try_add_implementation_source(path)) {
                        break;
                    }
                    append_unique(evidence.implementation_sources, seen_impl_sources, path);
                }
            }

            for (const auto& validation : bundle->validation) {
                const auto path = validation.supporting_test_files.empty()
                    ? validation.object_identifier
                    : validation.supporting_test_files.front();
                if (!tracker.try_add_test_file(path)) {
                    break;
                }
                append_unique(evidence.validation_tests, seen_tests, path);
            }
        }

        if (seen_navigation.insert(std::string(object_id)).second) {
            evidence.navigation.push_back(navigation_from_object(*object));
        }

        for (const auto& navigation : bundle->navigation) {
            append_package_citation(package_citations, seen_citations, navigation);
        }

        for (const auto& reference : bundle->supporting_references) {
            for (const auto& related_ref : reference.related_references) {
                if (!tracker.try_add_reference(related_ref)) {
                    break;
                }
                append_unique(evidence.related_references, seen_references, related_ref);
            }
            append_package_citation(package_citations, seen_citations, reference);
        }

        for (const auto& related : bundle->related_documents) {
            append_package_citation(package_citations, seen_citations, related);
        }
    }

    for (const auto& deferred : deferred_implementation) {
        if (deferred.bundle == nullptr) {
            continue;
        }

        for (const auto& implementation : deferred.bundle->implementation) {
            if (implementation.citation_type == CitationType::ImplementationHeader) {
                const auto path = implementation_path(implementation);
                if (!tracker.try_add_implementation_header(path)) {
                    break;
                }
                append_unique(evidence.implementation_headers, seen_impl_headers, path);
            } else if (implementation.citation_type == CitationType::ImplementationSource) {
                const auto path = implementation_path(implementation);
                if (!tracker.try_add_implementation_source(path)) {
                    break;
                }
                append_unique(evidence.implementation_sources, seen_impl_sources, path);
            }
        }

        for (const auto& validation : deferred.bundle->validation) {
            const auto path = validation.supporting_test_files.empty()
                ? validation.object_identifier
                : validation.supporting_test_files.front();
            if (!tracker.try_add_test_file(path)) {
                break;
            }
            append_unique(evidence.validation_tests, seen_tests, path);
        }
    }

    evidence.budget = tracker.utilization();

    if (evidence.budget.budget_truncated) {
        diagnostics.builder_notes.push_back("Context budget truncation occurred during assembly.");
        diagnostics.warnings.push_back(DiagnosticMessage{
            .level = "warning",
            .message = "Context assembly stopped early because a configured budget limit was reached.",
        });
    }

    if (selected_objects.empty() && diagnostics.missing_documentation.empty()) {
        diagnostics.missing_documentation.push_back(
            "No documented knowledge was available for this request.");
    }

    auto constraints = request.constraints;
    if (!constraints.response_length.has_value()) {
        constraints.response_length = effective_budget.max_estimated_tokens * 4;
    }

    diagnostics.builder_notes.push_back(
        "Assembled by ContextAssembler using policy " + to_string(request.policy) + ".");

    ContextAssemblyResult result{
        .package = ContextPackage::create(
            request.platform_request,
            std::move(selected_objects),
            std::move(repositories),
            std::move(package_citations),
            std::move(constraints),
            request.system,
            std::move(diagnostics)),
        .evidence = std::move(evidence),
        .budget = effective_budget,
        .policy = request.policy,
    };

    return result;
}

}  // namespace eve::context
