#pragma once

#include "eve/knowledge/knowledge_graph.hpp"

#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace eve {

class KnowledgeIndex {
public:
    [[nodiscard]] std::size_t object_count() const noexcept;
    [[nodiscard]] const KnowledgeGraph& graph() const noexcept;

    [[nodiscard]] std::vector<const KnowledgeObject*> lookup_by_object_id(
        std::string_view object_id) const;
    [[nodiscard]] std::vector<const KnowledgeObject*> lookup_by_document_identifier(
        std::string_view document_identifier) const;

    [[nodiscard]] std::vector<const KnowledgeObject*> lookup_by_title(std::string_view title) const;
    [[nodiscard]] std::vector<const KnowledgeObject*> lookup_by_keyword(
        std::string_view keyword) const;
    [[nodiscard]] std::vector<const KnowledgeObject*> lookup_by_tag(std::string_view tag) const;
    [[nodiscard]] std::vector<const KnowledgeObject*> lookup_by_document_type(
        DocumentType document_type) const;
    [[nodiscard]] std::vector<const KnowledgeObject*> lookup_by_repository(
        std::string_view repository) const;
    [[nodiscard]] std::vector<const KnowledgeObject*> lookup_by_series(std::string_view series) const;
    [[nodiscard]] std::vector<const KnowledgeObject*> lookup_by_part(std::string_view part) const;
    [[nodiscard]] std::vector<const KnowledgeObject*> lookup_by_status(std::string_view status) const;

    [[nodiscard]] std::vector<const KnowledgeObject*> lookup_by_search_text_token(
        std::string_view token) const;
    [[nodiscard]] std::vector<const KnowledgeObject*> lookup_by_heading(
        std::string_view heading) const;
    [[nodiscard]] std::vector<const KnowledgeObject*> lookup_by_section_title(
        std::string_view section_title) const;

    [[nodiscard]] std::vector<const KnowledgeObject*> lookup_by_relationship(
        KnowledgeGraphEdgeType relationship_type,
        std::string_view target) const;

private:
    friend class KnowledgeIndexBuilder;

    using ObjectIndexList = std::vector<std::size_t>;
    using StringIndex = std::map<std::string, ObjectIndexList>;

    KnowledgeIndex(
        KnowledgeGraph graph,
        StringIndex object_id_index,
        StringIndex document_identifier_index,
        StringIndex title_index,
        StringIndex keyword_index,
        StringIndex tag_index,
        std::map<DocumentType, ObjectIndexList> document_type_index,
        StringIndex repository_index,
        StringIndex series_index,
        StringIndex part_index,
        StringIndex status_index,
        StringIndex search_text_index,
        StringIndex heading_index,
        StringIndex section_title_index,
        std::map<KnowledgeGraphEdgeType, StringIndex> relationship_index);

    [[nodiscard]] std::vector<const KnowledgeObject*> resolve_indices(
        const ObjectIndexList& indices) const;
    [[nodiscard]] std::vector<const KnowledgeObject*> lookup_in_index(
        const StringIndex& index,
        std::string_view key) const;

    KnowledgeGraph graph_;
    StringIndex object_id_index_;
    StringIndex document_identifier_index_;
    StringIndex title_index_;
    StringIndex keyword_index_;
    StringIndex tag_index_;
    std::map<DocumentType, ObjectIndexList> document_type_index_;
    StringIndex repository_index_;
    StringIndex series_index_;
    StringIndex part_index_;
    StringIndex status_index_;
    StringIndex search_text_index_;
    StringIndex heading_index_;
    StringIndex section_title_index_;
    std::map<KnowledgeGraphEdgeType, StringIndex> relationship_index_;
};

class KnowledgeIndexBuilder {
public:
    [[nodiscard]] static KnowledgeIndex build(const KnowledgeGraph& graph);
};

}  // namespace eve
