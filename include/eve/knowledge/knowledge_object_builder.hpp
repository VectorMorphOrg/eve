#pragma once

#include "eve/knowledge/document_metadata.hpp"
#include "eve/knowledge/document_model.hpp"
#include "eve/knowledge/knowledge_object.hpp"

namespace eve {

class KnowledgeObjectBuilder {
public:
    [[nodiscard]] static KnowledgeObject build(
        const DocumentModel& model,
        const DocumentMetadata& metadata);
};

}  // namespace eve
