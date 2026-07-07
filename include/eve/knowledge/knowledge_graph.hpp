#pragma once

#include "eve/knowledge/knowledge_object.hpp"

#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace eve {

enum class KnowledgeGraphEdgeType {
    References,
    ReferencedBy,
    Previous,
    Next,
    Parent,
    Children,
    SameSeries,
    SamePart,
    SameRepository,
    RelatedHeader,
    RelatedSource,
    RelatedTest,
};

[[nodiscard]] std::string to_string(KnowledgeGraphEdgeType type);

struct KnowledgeGraphNode {
    KnowledgeObjectId id;
    std::string document_identifier;
    DocumentType document_type{DocumentType::Other};
    std::string file_path;
};

struct KnowledgeGraphEdge {
    KnowledgeObjectId source_id;
    std::string target;
    KnowledgeGraphEdgeType type{KnowledgeGraphEdgeType::References};

    friend bool operator==(const KnowledgeGraphEdge& lhs, const KnowledgeGraphEdge& rhs) noexcept {
        return lhs.source_id.value == rhs.source_id.value && lhs.target == rhs.target &&
               lhs.type == rhs.type;
    }
};

class KnowledgeGraph {
public:
    [[nodiscard]] std::size_t node_count() const noexcept { return nodes_.size(); }
    [[nodiscard]] std::size_t edge_count() const noexcept { return edges_.size(); }

    [[nodiscard]] const std::vector<KnowledgeGraphNode>& nodes() const noexcept { return nodes_; }
    [[nodiscard]] const std::vector<KnowledgeGraphEdge>& edges() const noexcept { return edges_; }
    [[nodiscard]] const std::vector<KnowledgeObject>& objects() const noexcept { return objects_; }

    [[nodiscard]] const KnowledgeObject* find_object_by_id(const KnowledgeObjectId& id) const;
    [[nodiscard]] const KnowledgeObject* find_object_by_identifier(std::string_view identifier) const;
    [[nodiscard]] const KnowledgeGraphNode* find_node_by_id(const KnowledgeObjectId& id) const;

private:
    friend class KnowledgeGraphBuilder;

    KnowledgeGraph(
        std::vector<KnowledgeObject> objects,
        std::vector<KnowledgeGraphNode> nodes,
        std::vector<KnowledgeGraphEdge> edges,
        std::map<std::string, std::size_t> lookup_by_id,
        std::map<std::string, std::size_t> lookup_by_identifier);

    std::vector<KnowledgeObject> objects_;
    std::vector<KnowledgeGraphNode> nodes_;
    std::vector<KnowledgeGraphEdge> edges_;
    std::map<std::string, std::size_t> lookup_by_id_;
    std::map<std::string, std::size_t> lookup_by_identifier_;
};

class KnowledgeGraphBuilder {
public:
    [[nodiscard]] static KnowledgeGraph build(const std::vector<KnowledgeObject>& objects);
};

}  // namespace eve
