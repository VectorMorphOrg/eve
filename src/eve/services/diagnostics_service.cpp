#include "eve/services/service_implementations.hpp"

namespace eve::services {

DiagnosticsService::DiagnosticsService(const IKnowledgeStore& knowledge_store)
    : knowledge_store_(knowledge_store) {}

std::vector<DiagnosticMessage> DiagnosticsService::collect() const {
    std::vector<DiagnosticMessage> messages;
    if (knowledge_store_.document_count() == 0) {
        messages.push_back(DiagnosticMessage{
            "warning",
            "No documents are currently indexed in the Knowledge Layer.",
        });
    } else {
        messages.push_back(DiagnosticMessage{
            "info",
            std::format("{} documents indexed.", knowledge_store_.document_count()),
        });
    }
    return messages;
}

}  // namespace eve::services
