#pragma once

#include "eve/core/types.hpp"
#include "eve/knowledge/document_model.hpp"

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace eve {

struct DocumentMetadata {
    std::string title;

    std::optional<std::string> identifier;
    std::optional<std::string> version;
    std::optional<std::string> authors;
    std::optional<std::string> status;
    std::optional<std::string> published;
    std::optional<std::string> project;
    std::optional<std::string> specification;

    std::optional<std::string> part;
    std::optional<std::string> series;
    std::optional<std::string> repository;
    DocumentType document_type{DocumentType::Other};
    std::optional<std::filesystem::path> file_path;

    std::optional<std::string> previous_document;
    std::optional<std::string> next_document;

    std::vector<MetadataEntry> explicit_entries;
};

}  // namespace eve
