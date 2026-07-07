#include "eve/knowledge/citation_engine.hpp"

#include <algorithm>
#include <set>

namespace eve {
namespace {

void append_unique_sorted(std::vector<std::string>& values, std::string value) {
    if (value.empty()) {
        return;
    }
    if (std::ranges::find(values, value) == values.end()) {
        values.push_back(std::move(value));
    }
}

void finalize_sorted_unique(std::vector<std::string>& values) {
    std::sort(values.begin(), values.end());
    values.erase(std::unique(values.begin(), values.end()), values.end());
}

const KnowledgeObject* resolve_object(
    const KnowledgeIndex& index,
    const KnowledgeGraph& graph,
    std::string_view object_identifier) {
    const auto by_id = index.lookup_by_object_id(object_identifier);
    if (!by_id.empty()) {
        return by_id.front();
    }
    const auto by_identifier = index.lookup_by_document_identifier(object_identifier);
    if (!by_identifier.empty()) {
        return by_identifier.front();
    }
    return graph.find_object_by_id(KnowledgeObjectId{std::string{object_identifier}});
}

std::vector<std::string> collect_section_titles(const KnowledgeObject& object) {
    std::vector<std::string> titles;
    for (const auto& section : object.content.sections) {
        append_unique_sorted(titles, section.title);
    }
    finalize_sorted_unique(titles);
    return titles;
}

std::vector<std::string> collect_headings(const KnowledgeObject& object) {
    std::vector<std::string> headings;
    for (const auto& heading : object.content.headings) {
        append_unique_sorted(headings, heading.text);
    }
    finalize_sorted_unique(headings);
    return headings;
}

int relationship_density_score(const KnowledgeObject& object) {
    int populated = 0;
    if (object.relationships.previous) {
        ++populated;
    }
    if (object.relationships.next) {
        ++populated;
    }
    if (object.relationships.parent) {
        ++populated;
    }
    populated += static_cast<int>(object.relationships.children.size());
    populated += static_cast<int>(object.relationships.references.size());
    populated += static_cast<int>(object.relationships.referenced_by.size());
    populated += static_cast<int>(object.relationships.related_headers.size());
    populated += static_cast<int>(object.relationships.related_sources.size());
    populated += static_cast<int>(object.relationships.related_tests.size());
    return std::min(populated * 2, 20);
}

int compute_confidence(const KnowledgeObject& object, CitationType citation_type) {
    int confidence = 0;

    switch (citation_type) {
        case CitationType::PrimarySpecification:
            if (object.identity.document_type == DocumentType::Specification) {
                confidence += 40;
            } else {
                confidence += 20;
            }
            if (!object.content.sections.empty()) {
                confidence += 10;
            }
            if (!object.content.headings.empty()) {
                confidence += 10;
            }
            break;
        case CitationType::ImplementationHeader:
        case CitationType::ImplementationSource:
            confidence += 30;
            break;
        case CitationType::ValidationTest:
            confidence += 35;
            break;
        case CitationType::RelatedDocument:
            confidence += 25;
            break;
        case CitationType::SupportingReference:
            confidence += 20;
            break;
        case CitationType::Navigation:
            confidence += 15;
            break;
    }

    if (!object.relationships.references.empty()) {
        confidence += 10;
    }
    if (!object.relationships.referenced_by.empty()) {
        confidence += 10;
    }
    confidence += relationship_density_score(object);
    return std::min(confidence, 100);
}

Citation build_citation_from_object(
    const KnowledgeObject& object,
    CitationType citation_type) {
    Citation citation{
        .object_identifier = object.identity.id.value,
        .document_identifier = object.identity.document_identifier,
        .title = object.identity.title,
        .document_type = object.identity.document_type,
        .repository = object.identity.repository,
        .supporting_section_titles = collect_section_titles(object),
        .supporting_headings = collect_headings(object),
        .supporting_source_files = object.relationships.related_sources,
        .supporting_test_files = object.relationships.related_tests,
        .related_references = object.relationships.references,
        .citation_type = citation_type,
        .citation_confidence = compute_confidence(object, citation_type),
    };
    finalize_sorted_unique(citation.supporting_source_files);
    finalize_sorted_unique(citation.supporting_test_files);
    finalize_sorted_unique(citation.related_references);
    for (const auto& header : object.relationships.related_headers) {
        append_unique_sorted(citation.supporting_source_files, header);
    }
    finalize_sorted_unique(citation.supporting_source_files);
    return citation;
}

Citation build_path_citation(
    const std::string& path,
    CitationType citation_type,
    const KnowledgeObject& context_object) {
    Citation citation;
    citation.object_identifier = path;
    citation.title = path;
    citation.repository = context_object.identity.repository;
    citation.citation_type = citation_type;
    citation.citation_confidence = compute_confidence(context_object, citation_type);
    if (citation_type == CitationType::ImplementationHeader ||
        citation_type == CitationType::ImplementationSource) {
        citation.supporting_source_files.push_back(path);
    } else if (citation_type == CitationType::ValidationTest) {
        citation.supporting_test_files.push_back(path);
    }
    return citation;
}

Citation build_reference_citation(
    const KnowledgeObject& source_object,
    const KnowledgeObject& referenced_object) {
    auto citation = build_citation_from_object(referenced_object, CitationType::SupportingReference);
    append_unique_sorted(citation.related_references, source_object.identity.document_identifier);
    finalize_sorted_unique(citation.related_references);
    citation.citation_confidence = compute_confidence(referenced_object, CitationType::SupportingReference);
    return citation;
}

std::vector<Citation> build_implementation_citations(const KnowledgeObject& object) {
    std::vector<Citation> citations;
    std::set<std::string> seen;

    for (const auto& header : object.relationships.related_headers) {
        if (!seen.insert(header).second) {
            continue;
        }
        citations.push_back(build_path_citation(header, CitationType::ImplementationHeader, object));
    }
    for (const auto& source : object.relationships.related_sources) {
        if (!seen.insert(source).second) {
            continue;
        }
        citations.push_back(build_path_citation(source, CitationType::ImplementationSource, object));
    }

    std::sort(citations.begin(), citations.end(), [](const Citation& lhs, const Citation& rhs) {
        return lhs.object_identifier < rhs.object_identifier;
    });
    return citations;
}

std::vector<Citation> build_validation_citations(const KnowledgeObject& object) {
    std::vector<Citation> citations;
    std::set<std::string> seen;

    for (const auto& test_path : object.relationships.related_tests) {
        if (!seen.insert(test_path).second) {
            continue;
        }
        citations.push_back(build_path_citation(test_path, CitationType::ValidationTest, object));
    }

    std::sort(citations.begin(), citations.end(), [](const Citation& lhs, const Citation& rhs) {
        return lhs.object_identifier < rhs.object_identifier;
    });
    return citations;
}

std::vector<Citation> build_related_document_citations(
    const KnowledgeObject& object,
    const KnowledgeIndex& index,
    const KnowledgeGraph& graph) {
    std::vector<Citation> citations;
    std::set<std::string> seen;

    for (const auto& reference : object.relationships.references) {
        if (const auto* related = resolve_object(index, graph, reference)) {
            if (!seen.insert(related->identity.id.value).second) {
                continue;
            }
            citations.push_back(build_citation_from_object(*related, CitationType::RelatedDocument));
        }
    }
    for (const auto& referencer_id : object.relationships.referenced_by) {
        if (const auto* related = resolve_object(index, graph, referencer_id)) {
            if (!seen.insert(related->identity.id.value).second) {
                continue;
            }
            citations.push_back(build_citation_from_object(*related, CitationType::RelatedDocument));
        }
    }

    std::sort(citations.begin(), citations.end(), [](const Citation& lhs, const Citation& rhs) {
        return lhs.object_identifier < rhs.object_identifier;
    });
    return citations;
}

std::vector<Citation> build_supporting_reference_citations(
    const KnowledgeObject& object,
    const KnowledgeIndex& index,
    const KnowledgeGraph& graph) {
    std::vector<Citation> citations;
    std::set<std::string> seen;

    for (const auto& reference : object.relationships.references) {
        if (const auto* related = resolve_object(index, graph, reference)) {
            if (!seen.insert(related->identity.id.value).second) {
                continue;
            }
            citations.push_back(build_reference_citation(object, *related));
            continue;
        }

        if (!seen.insert(reference).second) {
            continue;
        }
        Citation unresolved;
        unresolved.object_identifier = reference;
        unresolved.document_identifier = reference;
        unresolved.title = reference;
        unresolved.repository = object.identity.repository;
        unresolved.related_references = {reference};
        unresolved.citation_type = CitationType::SupportingReference;
        unresolved.citation_confidence = compute_confidence(object, CitationType::SupportingReference);
        citations.push_back(std::move(unresolved));
    }

    std::sort(citations.begin(), citations.end(), [](const Citation& lhs, const Citation& rhs) {
        return lhs.object_identifier < rhs.object_identifier;
    });
    return citations;
}

std::vector<Citation> build_navigation_citations(
    const KnowledgeObject& object,
    const KnowledgeIndex& index,
    const KnowledgeGraph& graph) {
    std::vector<Citation> citations;
    std::set<std::string> seen;

    auto add_navigation_target = [&](const std::optional<std::string>& target) {
        if (!target) {
            return;
        }
        if (const auto* related = resolve_object(index, graph, *target)) {
            if (!seen.insert(related->identity.id.value).second) {
                return;
            }
            citations.push_back(build_citation_from_object(*related, CitationType::Navigation));
            return;
        }
        if (!seen.insert(*target).second) {
            return;
        }
        Citation unresolved;
        unresolved.object_identifier = *target;
        unresolved.document_identifier = *target;
        unresolved.title = *target;
        unresolved.repository = object.identity.repository;
        unresolved.citation_type = CitationType::Navigation;
        unresolved.citation_confidence = compute_confidence(object, CitationType::Navigation);
        citations.push_back(std::move(unresolved));
    };

    add_navigation_target(object.relationships.previous);
    add_navigation_target(object.relationships.next);
    add_navigation_target(object.relationships.parent);
    for (const auto& child : object.relationships.children) {
        add_navigation_target(child);
    }

    std::sort(citations.begin(), citations.end(), [](const Citation& lhs, const Citation& rhs) {
        return lhs.object_identifier < rhs.object_identifier;
    });
    return citations;
}

CitationBundle build_bundle(
    const RankedSearchResult& ranked_result,
    const KnowledgeIndex& index,
    const KnowledgeGraph& graph) {
    CitationBundle bundle{
        .ranked_result = ranked_result,
    };

    const auto* object = resolve_object(index, graph, ranked_result.candidate.object_identifier);
    if (object == nullptr) {
        bundle.primary = Citation{
            .object_identifier = ranked_result.candidate.object_identifier,
            .document_identifier = ranked_result.candidate.document_identifier,
            .title = ranked_result.candidate.title,
            .document_type = ranked_result.candidate.document_type,
            .repository = ranked_result.candidate.repository,
            .citation_type = CitationType::PrimarySpecification,
            .citation_confidence = 0,
        };
        return bundle;
    }

    bundle.primary = build_citation_from_object(*object, CitationType::PrimarySpecification);
    bundle.implementation = build_implementation_citations(*object);
    bundle.validation = build_validation_citations(*object);
    bundle.related_documents = build_related_document_citations(*object, index, graph);
    bundle.supporting_references = build_supporting_reference_citations(*object, index, graph);
    bundle.navigation = build_navigation_citations(*object, index, graph);
    return bundle;
}

}  // namespace

std::string to_string(CitationType type) {
    switch (type) {
        case CitationType::PrimarySpecification:
            return "Primary Specification";
        case CitationType::ImplementationHeader:
            return "Implementation Header";
        case CitationType::ImplementationSource:
            return "Implementation Source";
        case CitationType::ValidationTest:
            return "Validation Test";
        case CitationType::RelatedDocument:
            return "Related Document";
        case CitationType::SupportingReference:
            return "Supporting Reference";
        case CitationType::Navigation:
            return "Navigation";
    }
    return "Unknown";
}

CitationEngine::CitationEngine(const KnowledgeIndex& index, const KnowledgeGraph& graph)
    : index_(&index), graph_(&graph) {}

const KnowledgeIndex& CitationEngine::index() const noexcept {
    return *index_;
}

const KnowledgeGraph& CitationEngine::graph() const noexcept {
    return *graph_;
}

CitationResultSet CitationEngine::cite(const RankedSearchResultSet& ranked_results) const {
    CitationResultSet result_set{
        .query = ranked_results.query,
        .resolved_mode = ranked_results.resolved_mode,
    };

    result_set.bundles.reserve(ranked_results.results.size());
    for (const auto& ranked_result : ranked_results.results) {
        result_set.bundles.push_back(build_bundle(ranked_result, *index_, *graph_));
    }

    result_set.bundle_count = result_set.bundles.size();
    return result_set;
}

}  // namespace eve
