#pragma once

#include "eve/knowledge/document_metadata.hpp"
#include "eve/knowledge/document_model.hpp"

#include <filesystem>
#include <string>
#include <vector>

namespace eve {

struct MetadataExtractionContext {
    std::string repository_name;
    std::vector<std::filesystem::path> series_documents;
};

class MetadataExtractor {
public:
    [[nodiscard]] static DocumentMetadata extract(const DocumentModel& model);

    [[nodiscard]] static DocumentMetadata extract(
        const DocumentModel& model,
        const MetadataExtractionContext& context);
};

}  // namespace eve
