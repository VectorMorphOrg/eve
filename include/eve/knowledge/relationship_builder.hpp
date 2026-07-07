#pragma once

#include "eve/knowledge/knowledge_object.hpp"

#include <string>
#include <vector>

namespace eve {

class RelationshipBuilder {
public:
    [[nodiscard]] static std::vector<KnowledgeObject> build(std::vector<KnowledgeObject> objects);

    [[nodiscard]] static std::vector<std::string> discover_reference_identifiers(
        std::string_view text);

    [[nodiscard]] static std::string normalize_reference_identifier(std::string reference);
};

}  // namespace eve
