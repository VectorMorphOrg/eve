#pragma once

#include "eve/core/types.hpp"

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

struct KnowledgeObject {
    KnowledgeObjectId identifier;
    std::string repository;
    DocumentType document_type{DocumentType::Other};
    std::string title;
    std::string version;
    std::string author;
    std::string last_modified;
    std::vector<std::string> tags;
    std::vector<KnowledgeSection> sections;
    std::vector<std::string> references;
    std::filesystem::path source_path;
    std::string excerpt;
    int priority_rank{100};
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

}  // namespace eve
