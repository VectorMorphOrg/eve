#pragma once

#include "eve/core/types.hpp"
#include "eve/knowledge/document_model.hpp"

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace eve {

struct KnowledgeSection {
    std::string identifier;
    std::string title;
    std::string content;
    std::vector<std::string> references;
};

struct KnowledgeParagraph {
    std::string text;
    std::size_t block_index{0};
};

struct KnowledgeList {
    bool ordered{false};
    std::vector<DocumentListItem> items;
    std::size_t block_index{0};
};

struct KnowledgeTable {
    std::vector<DocumentTableRow> rows;
    std::size_t block_index{0};
};

struct KnowledgeCodeBlock {
    std::string language;
    std::string content;
    std::size_t block_index{0};
};

struct KnowledgeObjectIdentity {
    KnowledgeObjectId id;
    std::string document_identifier;
    std::string title;
    DocumentType document_type{DocumentType::Other};
    std::string repository;
    std::string version;
};

struct KnowledgeObjectMetadata {
    std::string status;
    std::string authors;
    std::string published;
    std::string part;
    std::string series;
    std::vector<std::string> tags;
};

struct KnowledgeObjectContent {
    std::vector<KnowledgeSection> sections;
    std::vector<DocumentHeading> headings;
    std::vector<KnowledgeParagraph> paragraphs;
    std::vector<KnowledgeList> lists;
    std::vector<KnowledgeTable> tables;
    std::vector<KnowledgeCodeBlock> code_blocks;
    std::vector<DocumentLink> links;
    std::string raw_markdown;
};

struct KnowledgeObjectRelationships {
    std::optional<std::string> previous;
    std::optional<std::string> next;
    std::optional<std::string> parent;
    std::vector<std::string> children;

    std::vector<std::string> references;
    std::vector<std::string> referenced_by;

    std::vector<std::string> same_series;
    std::vector<std::string> same_part;
    std::vector<std::string> same_repository;

    std::vector<std::string> related_headers;
    std::vector<std::string> related_sources;
    std::vector<std::string> related_tests;
};

struct KnowledgeObjectSearchData {
    std::string search_text;
    std::string excerpt;
    std::vector<std::string> keywords;
    int priority_rank{100};
};

struct KnowledgeObjectSource {
    std::filesystem::path repository_path;
    std::filesystem::path file_path;
    std::string last_modified;
    TimePoint last_modified_time{};
};

struct KnowledgeObject {
    KnowledgeObjectIdentity identity;
    KnowledgeObjectMetadata metadata;
    KnowledgeObjectContent content;
    KnowledgeObjectRelationships relationships;
    KnowledgeObjectSearchData search;
    KnowledgeObjectSource source;
};

struct SearchResult {
    KnowledgeObject object;
    double score{0.0};
};

class IKnowledgeStore {
public:
    virtual ~IKnowledgeStore() = default;

    [[nodiscard]] virtual std::optional<KnowledgeObject> get_by_id(
        const KnowledgeObjectId& id) const = 0;
    [[nodiscard]] virtual std::vector<KnowledgeObject> list_by_repository(
        std::string_view repository) const = 0;
    [[nodiscard]] virtual std::vector<SearchResult> search(
        std::string_view query,
        std::optional<std::string_view> repository = std::nullopt) const = 0;
    [[nodiscard]] virtual std::size_t document_count() const = 0;
};

class DocumentIndexer {
public:
    [[nodiscard]] static std::vector<KnowledgeObject> index_directory(
        const std::filesystem::path& root,
        std::string_view repository_name);

private:
    [[nodiscard]] static DocumentType classify_document(
        const std::filesystem::path& path,
        std::string_view title);
    [[nodiscard]] static int priority_for(DocumentType type);
    [[nodiscard]] static KnowledgeObject parse_markdown_file(
        const std::filesystem::path& path,
        std::string_view repository_name,
        std::uint32_t sequence);
};

[[nodiscard]] std::string knowledge_object_id_from_path(
    const std::filesystem::path& path,
    std::string_view repository_name);

[[nodiscard]] int priority_rank_for_document_type(DocumentType type);

}  // namespace eve
