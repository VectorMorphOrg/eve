#include "eve/knowledge/knowledge_graph.hpp"

#include <algorithm>
#include <cctype>
#include <map>

namespace eve {
namespace {

std::string to_upper(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::toupper(ch));
    });
    return value;
}

std::string normalize_lookup_key(std::string_view value) {
    return to_upper(std::string{value});
}

std::string path_string(const std::filesystem::path& path) {
    if (path.empty()) {
        return {};
    }
    return path.generic_string();
}

void append_edge(
    std::vector<KnowledgeGraphEdge>& edges,
    KnowledgeObjectId source_id,
    std::string target,
    KnowledgeGraphEdgeType type) {
    if (target.empty()) {
        return;
    }
    edges.push_back(KnowledgeGraphEdge{
        .source_id = std::move(source_id),
        .target = std::move(target),
        .type = type,
    });
}

void append_edges(
    std::vector<KnowledgeGraphEdge>& edges,
    KnowledgeObjectId source_id,
    const std::vector<std::string>& targets,
    KnowledgeGraphEdgeType type) {
    for (const auto& target : targets) {
        append_edge(edges, source_id, target, type);
    }
}

KnowledgeGraphNode make_node(const KnowledgeObject& object) {
    return KnowledgeGraphNode{
        .id = object.identity.id,
        .document_identifier = object.identity.document_identifier,
        .document_type = object.identity.document_type,
        .file_path = path_string(object.source.file_path),
    };
}

void sort_nodes(std::vector<KnowledgeGraphNode>& nodes) {
    std::sort(nodes.begin(), nodes.end(), [](const KnowledgeGraphNode& lhs, const KnowledgeGraphNode& rhs) {
        return lhs.id.value < rhs.id.value;
    });
}

void sort_edges(std::vector<KnowledgeGraphEdge>& edges) {
    std::sort(edges.begin(), edges.end(), [](const KnowledgeGraphEdge& lhs, const KnowledgeGraphEdge& rhs) {
        if (lhs.source_id.value != rhs.source_id.value) {
            return lhs.source_id.value < rhs.source_id.value;
        }
        if (lhs.type != rhs.type) {
            return lhs.type < rhs.type;
        }
        return lhs.target < rhs.target;
    });
}

std::vector<KnowledgeGraphEdge> build_edges(const std::vector<KnowledgeObject>& objects) {
    std::vector<KnowledgeGraphEdge> edges;
    edges.reserve(objects.size() * 4U);

    for (const auto& object : objects) {
        const auto source_id = object.identity.id;

        append_edges(edges, source_id, object.relationships.references, KnowledgeGraphEdgeType::References);
        append_edges(
            edges,
            source_id,
            object.relationships.referenced_by,
            KnowledgeGraphEdgeType::ReferencedBy);
        append_edges(edges, source_id, object.relationships.children, KnowledgeGraphEdgeType::Children);
        append_edges(
            edges,
            source_id,
            object.relationships.same_series,
            KnowledgeGraphEdgeType::SameSeries);
        append_edges(edges, source_id, object.relationships.same_part, KnowledgeGraphEdgeType::SamePart);
        append_edges(
            edges,
            source_id,
            object.relationships.same_repository,
            KnowledgeGraphEdgeType::SameRepository);
        append_edges(
            edges,
            source_id,
            object.relationships.related_headers,
            KnowledgeGraphEdgeType::RelatedHeader);
        append_edges(
            edges,
            source_id,
            object.relationships.related_sources,
            KnowledgeGraphEdgeType::RelatedSource);
        append_edges(
            edges,
            source_id,
            object.relationships.related_tests,
            KnowledgeGraphEdgeType::RelatedTest);

        if (object.relationships.previous) {
            append_edge(edges, source_id, *object.relationships.previous, KnowledgeGraphEdgeType::Previous);
        }
        if (object.relationships.next) {
            append_edge(edges, source_id, *object.relationships.next, KnowledgeGraphEdgeType::Next);
        }
        if (object.relationships.parent) {
            append_edge(edges, source_id, *object.relationships.parent, KnowledgeGraphEdgeType::Parent);
        }
    }

    sort_edges(edges);
    return edges;
}

std::map<std::string, std::size_t> build_id_lookup(const std::vector<KnowledgeObject>& objects) {
    std::map<std::string, std::size_t> lookup;
    for (std::size_t index = 0; index < objects.size(); ++index) {
        const auto& id = objects[index].identity.id.value;
        if (!id.empty()) {
            lookup.emplace(normalize_lookup_key(id), index);
        }
    }
    return lookup;
}

std::map<std::string, std::size_t> build_identifier_lookup(const std::vector<KnowledgeObject>& objects) {
    std::map<std::string, std::size_t> lookup;
    for (std::size_t index = 0; index < objects.size(); ++index) {
        const auto& document_identifier = objects[index].identity.document_identifier;
        if (!document_identifier.empty()) {
            lookup.emplace(normalize_lookup_key(document_identifier), index);
        }
        const auto& id = objects[index].identity.id.value;
        if (!id.empty()) {
            lookup.emplace(normalize_lookup_key(id), index);
        }
    }
    return lookup;
}

const KnowledgeObject* lookup_object(
    std::string_view key,
    const std::vector<KnowledgeObject>& objects,
    const std::map<std::string, std::size_t>& lookup) {
    if (key.empty()) {
        return nullptr;
    }
    const auto iterator = lookup.find(normalize_lookup_key(key));
    if (iterator == lookup.end()) {
        return nullptr;
    }
    return &objects.at(iterator->second);
}

}  // namespace

std::string to_string(KnowledgeGraphEdgeType type) {
    switch (type) {
        case KnowledgeGraphEdgeType::References:
            return "References";
        case KnowledgeGraphEdgeType::ReferencedBy:
            return "Referenced By";
        case KnowledgeGraphEdgeType::Previous:
            return "Previous";
        case KnowledgeGraphEdgeType::Next:
            return "Next";
        case KnowledgeGraphEdgeType::Parent:
            return "Parent";
        case KnowledgeGraphEdgeType::Children:
            return "Children";
        case KnowledgeGraphEdgeType::SameSeries:
            return "Same Series";
        case KnowledgeGraphEdgeType::SamePart:
            return "Same Part";
        case KnowledgeGraphEdgeType::SameRepository:
            return "Same Repository";
        case KnowledgeGraphEdgeType::RelatedHeader:
            return "Related Header";
        case KnowledgeGraphEdgeType::RelatedSource:
            return "Related Source";
        case KnowledgeGraphEdgeType::RelatedTest:
            return "Related Test";
    }
    return "Unknown";
}

KnowledgeGraph::KnowledgeGraph(
    std::vector<KnowledgeObject> objects,
    std::vector<KnowledgeGraphNode> nodes,
    std::vector<KnowledgeGraphEdge> edges,
    std::map<std::string, std::size_t> lookup_by_id,
    std::map<std::string, std::size_t> lookup_by_identifier)
    : objects_(std::move(objects)),
      nodes_(std::move(nodes)),
      edges_(std::move(edges)),
      lookup_by_id_(std::move(lookup_by_id)),
      lookup_by_identifier_(std::move(lookup_by_identifier)) {}

const KnowledgeObject* KnowledgeGraph::find_object_by_id(const KnowledgeObjectId& id) const {
    return lookup_object(id.value, objects_, lookup_by_id_);
}

const KnowledgeObject* KnowledgeGraph::find_object_by_identifier(std::string_view identifier) const {
    return lookup_object(identifier, objects_, lookup_by_identifier_);
}

const KnowledgeGraphNode* KnowledgeGraph::find_node_by_id(const KnowledgeObjectId& id) const {
    const auto iterator = std::lower_bound(
        nodes_.begin(),
        nodes_.end(),
        id,
        [](const KnowledgeGraphNode& node, const KnowledgeObjectId& target) {
            return node.id.value < target.value;
        });
    if (iterator == nodes_.end() || iterator->id.value != id.value) {
        return nullptr;
    }
    return &(*iterator);
}

KnowledgeGraph KnowledgeGraphBuilder::build(const std::vector<KnowledgeObject>& objects) {
    const auto object_snapshot = objects;

    std::vector<KnowledgeGraphNode> nodes;
    nodes.reserve(object_snapshot.size());
    for (const auto& object : object_snapshot) {
        nodes.push_back(make_node(object));
    }
    sort_nodes(nodes);

    auto edges = build_edges(object_snapshot);
    auto lookup_by_id = build_id_lookup(object_snapshot);
    auto lookup_by_identifier = build_identifier_lookup(object_snapshot);

    return KnowledgeGraph{
        object_snapshot,
        std::move(nodes),
        std::move(edges),
        std::move(lookup_by_id),
        std::move(lookup_by_identifier),
    };
}

}  // namespace eve
