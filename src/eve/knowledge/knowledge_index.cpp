#include "eve/knowledge/knowledge_index.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace eve {
namespace {

using ObjectIndexList = std::vector<std::size_t>;
using StringIndex = std::map<std::string, ObjectIndexList>;

std::string normalize_key(std::string_view value) {
    std::string normalized{value};
    std::transform(normalized.begin(), normalized.end(), normalized.begin(), [](unsigned char ch) {
        return static_cast<char>(std::toupper(ch));
    });
    return normalized;
}

void append_unique_sorted(ObjectIndexList& indices, std::size_t object_index) {
    if (std::ranges::find(indices, object_index) == indices.end()) {
        indices.push_back(object_index);
    }
}

void finalize_index_list(
    ObjectIndexList& indices,
    const std::vector<KnowledgeObject>& objects) {
    std::sort(indices.begin(), indices.end(), [&objects](std::size_t lhs, std::size_t rhs) {
        return objects[lhs].identity.id.value < objects[rhs].identity.id.value;
    });
    indices.erase(std::unique(indices.begin(), indices.end()), indices.end());
}

void finalize_string_index(
    StringIndex& index,
    const std::vector<KnowledgeObject>& objects) {
    for (auto& [_, indices] : index) {
        finalize_index_list(indices, objects);
    }
}

void index_string_key(
    StringIndex& index,
    std::string_view key,
    std::size_t object_index) {
    if (key.empty()) {
        return;
    }
    append_unique_sorted(index[normalize_key(key)], object_index);
}

void index_tokens(
    StringIndex& index,
    std::string_view text,
    std::size_t object_index) {
    std::string token;
    for (const unsigned char ch : text) {
        if (std::isalnum(ch) != 0) {
            token.push_back(static_cast<char>(ch));
            continue;
        }
        if (!token.empty()) {
            index_string_key(index, token, object_index);
            token.clear();
        }
    }
    if (!token.empty()) {
        index_string_key(index, token, object_index);
    }
}

void build_identity_indexes(
    const std::vector<KnowledgeObject>& objects,
    StringIndex& object_id_index,
    StringIndex& document_identifier_index) {
    for (std::size_t object_index = 0; object_index < objects.size(); ++object_index) {
        const auto& object = objects[object_index];
        index_string_key(object_id_index, object.identity.id.value, object_index);
        index_string_key(
            document_identifier_index,
            object.identity.document_identifier,
            object_index);
    }
}

void build_metadata_indexes(
    const std::vector<KnowledgeObject>& objects,
    StringIndex& title_index,
    StringIndex& keyword_index,
    StringIndex& tag_index,
    std::map<DocumentType, ObjectIndexList>& document_type_index,
    StringIndex& repository_index,
    StringIndex& series_index,
    StringIndex& part_index,
    StringIndex& status_index) {
    for (std::size_t object_index = 0; object_index < objects.size(); ++object_index) {
        const auto& object = objects[object_index];

        index_string_key(title_index, object.identity.title, object_index);
        for (const auto& keyword : object.search.keywords) {
            index_string_key(keyword_index, keyword, object_index);
        }
        for (const auto& tag : object.metadata.tags) {
            index_string_key(tag_index, tag, object_index);
        }
        append_unique_sorted(document_type_index[object.identity.document_type], object_index);
        index_string_key(repository_index, object.identity.repository, object_index);
        index_string_key(series_index, object.metadata.series, object_index);
        index_string_key(part_index, object.metadata.part, object_index);
        index_string_key(status_index, object.metadata.status, object_index);
    }
}

void build_content_indexes(
    const std::vector<KnowledgeObject>& objects,
    StringIndex& search_text_index,
    StringIndex& heading_index,
    StringIndex& section_title_index) {
    for (std::size_t object_index = 0; object_index < objects.size(); ++object_index) {
        const auto& object = objects[object_index];

        index_tokens(search_text_index, object.search.search_text, object_index);
        for (const auto& heading : object.content.headings) {
            index_string_key(heading_index, heading.text, object_index);
        }
        for (const auto& section : object.content.sections) {
            index_string_key(section_title_index, section.title, object_index);
        }
    }
}

bool is_indexed_relationship_type(KnowledgeGraphEdgeType type) {
    switch (type) {
        case KnowledgeGraphEdgeType::References:
        case KnowledgeGraphEdgeType::ReferencedBy:
        case KnowledgeGraphEdgeType::Previous:
        case KnowledgeGraphEdgeType::Next:
        case KnowledgeGraphEdgeType::Parent:
        case KnowledgeGraphEdgeType::Children:
            return true;
        default:
            return false;
    }
}

void build_relationship_index(
    const KnowledgeGraph& graph,
    std::map<KnowledgeGraphEdgeType, StringIndex>& relationship_index) {
    const auto& objects = graph.objects();
    for (const auto& edge : graph.edges()) {
        if (!is_indexed_relationship_type(edge.type)) {
            continue;
        }

        const auto source_iterator = std::ranges::find_if(objects, [&edge](const KnowledgeObject& object) {
            return object.identity.id.value == edge.source_id.value;
        });
        if (source_iterator == objects.end()) {
            continue;
        }

        const auto object_index = static_cast<std::size_t>(
            std::distance(objects.begin(), source_iterator));
        index_string_key(relationship_index[edge.type], edge.target, object_index);
    }
}

void finalize_document_type_index(
    std::map<DocumentType, ObjectIndexList>& document_type_index,
    const std::vector<KnowledgeObject>& objects) {
    for (auto& [_, indices] : document_type_index) {
        finalize_index_list(indices, objects);
    }
}

void finalize_relationship_index(
    std::map<KnowledgeGraphEdgeType, StringIndex>& relationship_index,
    const std::vector<KnowledgeObject>& objects) {
    for (auto& [_, index] : relationship_index) {
        finalize_string_index(index, objects);
    }
}

}  // namespace

std::size_t KnowledgeIndex::object_count() const noexcept {
    return graph_.objects().size();
}

const KnowledgeGraph& KnowledgeIndex::graph() const noexcept {
    return graph_;
}

KnowledgeIndex::KnowledgeIndex(
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
    std::map<KnowledgeGraphEdgeType, StringIndex> relationship_index)
    : graph_(std::move(graph)),
      object_id_index_(std::move(object_id_index)),
      document_identifier_index_(std::move(document_identifier_index)),
      title_index_(std::move(title_index)),
      keyword_index_(std::move(keyword_index)),
      tag_index_(std::move(tag_index)),
      document_type_index_(std::move(document_type_index)),
      repository_index_(std::move(repository_index)),
      series_index_(std::move(series_index)),
      part_index_(std::move(part_index)),
      status_index_(std::move(status_index)),
      search_text_index_(std::move(search_text_index)),
      heading_index_(std::move(heading_index)),
      section_title_index_(std::move(section_title_index)),
      relationship_index_(std::move(relationship_index)) {}

std::vector<const KnowledgeObject*> KnowledgeIndex::resolve_indices(
    const ObjectIndexList& indices) const {
    std::vector<const KnowledgeObject*> results;
    results.reserve(indices.size());
    for (const auto object_index : indices) {
        results.push_back(&graph_.objects().at(object_index));
    }
    return results;
}

std::vector<const KnowledgeObject*> KnowledgeIndex::lookup_in_index(
    const StringIndex& index,
    std::string_view key) const {
    if (key.empty()) {
        return {};
    }
    const auto iterator = index.find(normalize_key(key));
    if (iterator == index.end()) {
        return {};
    }
    return resolve_indices(iterator->second);
}

std::vector<const KnowledgeObject*> KnowledgeIndex::lookup_by_object_id(
    std::string_view object_id) const {
    return lookup_in_index(object_id_index_, object_id);
}

std::vector<const KnowledgeObject*> KnowledgeIndex::lookup_by_document_identifier(
    std::string_view document_identifier) const {
    return lookup_in_index(document_identifier_index_, document_identifier);
}

std::vector<const KnowledgeObject*> KnowledgeIndex::lookup_by_title(std::string_view title) const {
    return lookup_in_index(title_index_, title);
}

std::vector<const KnowledgeObject*> KnowledgeIndex::lookup_by_keyword(
    std::string_view keyword) const {
    return lookup_in_index(keyword_index_, keyword);
}

std::vector<const KnowledgeObject*> KnowledgeIndex::lookup_by_tag(std::string_view tag) const {
    return lookup_in_index(tag_index_, tag);
}

std::vector<const KnowledgeObject*> KnowledgeIndex::lookup_by_document_type(
    DocumentType document_type) const {
    const auto iterator = document_type_index_.find(document_type);
    if (iterator == document_type_index_.end()) {
        return {};
    }
    return resolve_indices(iterator->second);
}

std::vector<const KnowledgeObject*> KnowledgeIndex::lookup_by_repository(
    std::string_view repository) const {
    return lookup_in_index(repository_index_, repository);
}

std::vector<const KnowledgeObject*> KnowledgeIndex::lookup_by_series(std::string_view series) const {
    return lookup_in_index(series_index_, series);
}

std::vector<const KnowledgeObject*> KnowledgeIndex::lookup_by_part(std::string_view part) const {
    return lookup_in_index(part_index_, part);
}

std::vector<const KnowledgeObject*> KnowledgeIndex::lookup_by_status(std::string_view status) const {
    return lookup_in_index(status_index_, status);
}

std::vector<const KnowledgeObject*> KnowledgeIndex::lookup_by_search_text_token(
    std::string_view token) const {
    return lookup_in_index(search_text_index_, token);
}

std::vector<const KnowledgeObject*> KnowledgeIndex::lookup_by_heading(
    std::string_view heading) const {
    return lookup_in_index(heading_index_, heading);
}

std::vector<const KnowledgeObject*> KnowledgeIndex::lookup_by_section_title(
    std::string_view section_title) const {
    return lookup_in_index(section_title_index_, section_title);
}

std::vector<const KnowledgeObject*> KnowledgeIndex::lookup_by_relationship(
    KnowledgeGraphEdgeType relationship_type,
    std::string_view target) const {
    const auto type_iterator = relationship_index_.find(relationship_type);
    if (type_iterator == relationship_index_.end()) {
        return {};
    }
    return lookup_in_index(type_iterator->second, target);
}

KnowledgeIndex KnowledgeIndexBuilder::build(const KnowledgeGraph& graph) {
    StringIndex object_id_index;
    StringIndex document_identifier_index;
    StringIndex title_index;
    StringIndex keyword_index;
    StringIndex tag_index;
    std::map<DocumentType, ObjectIndexList> document_type_index;
    StringIndex repository_index;
    StringIndex series_index;
    StringIndex part_index;
    StringIndex status_index;
    StringIndex search_text_index;
    StringIndex heading_index;
    StringIndex section_title_index;
    std::map<KnowledgeGraphEdgeType, StringIndex> relationship_index;

    const auto& objects = graph.objects();

    build_identity_indexes(objects, object_id_index, document_identifier_index);
    build_metadata_indexes(
        objects,
        title_index,
        keyword_index,
        tag_index,
        document_type_index,
        repository_index,
        series_index,
        part_index,
        status_index);
    build_content_indexes(objects, search_text_index, heading_index, section_title_index);
    build_relationship_index(graph, relationship_index);

    finalize_string_index(object_id_index, objects);
    finalize_string_index(document_identifier_index, objects);
    finalize_string_index(title_index, objects);
    finalize_string_index(keyword_index, objects);
    finalize_string_index(tag_index, objects);
    finalize_document_type_index(document_type_index, objects);
    finalize_string_index(repository_index, objects);
    finalize_string_index(series_index, objects);
    finalize_string_index(part_index, objects);
    finalize_string_index(status_index, objects);
    finalize_string_index(search_text_index, objects);
    finalize_string_index(heading_index, objects);
    finalize_string_index(section_title_index, objects);
    finalize_relationship_index(relationship_index, objects);

    return KnowledgeIndex{
        graph,
        std::move(object_id_index),
        std::move(document_identifier_index),
        std::move(title_index),
        std::move(keyword_index),
        std::move(tag_index),
        std::move(document_type_index),
        std::move(repository_index),
        std::move(series_index),
        std::move(part_index),
        std::move(status_index),
        std::move(search_text_index),
        std::move(heading_index),
        std::move(section_title_index),
        std::move(relationship_index),
    };
}

}  // namespace eve
