#pragma once

#include "eve/knowledge/document_model.hpp"
#include "eve/knowledge/repository_index.hpp"

#include <string>

namespace eve {

class MarkdownParser {
public:
    [[nodiscard]] static DocumentModel parse(std::string markdown);
    [[nodiscard]] static DocumentModel parse(
        const RepositoryResource& resource,
        std::string markdown);
    [[nodiscard]] static DocumentModel parse_file(const RepositoryResource& resource);
};

}  // namespace eve
