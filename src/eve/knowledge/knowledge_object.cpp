#include "eve/knowledge/knowledge_object.hpp"

namespace eve {

int priority_rank_for_document_type(DocumentType type) {
    switch (type) {
        case DocumentType::Specification:
            return 10;
        case DocumentType::Rfc:
            return 20;
        case DocumentType::ReferenceCatalog:
            return 30;
        case DocumentType::Whitepaper:
            return 40;
        case DocumentType::Guide:
            return 50;
        case DocumentType::Readme:
            return 60;
        case DocumentType::Changelog:
            return 70;
        case DocumentType::Other:
            return 100;
    }
    return 100;
}

}  // namespace eve
