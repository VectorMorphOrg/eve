#include "eve/capability/handlers/capability_handlers.hpp"

#include <format>
#include <sstream>

namespace eve::capability::handlers {
namespace {

class DocumentationSearchHandler final : public ICapabilityHandler {
public:
    DocumentationSearchHandler(
        const services::ISearchService& search,
        const context::ContextBuilder& context_builder,
        std::shared_ptr<const ai::ProviderManager> provider_manager)
        : search_(search),
          context_builder_(context_builder),
          provider_manager_(std::move(provider_manager)) {}

    [[nodiscard]] CapabilityId id() const override { return CapabilityId{"CAP-0102"}; }
    [[nodiscard]] std::string name() const override { return "Documentation Search"; }

    [[nodiscard]] std::expected<PlatformResponse, CapabilityError> execute(
        const PlatformRequest& request) const override {
        const auto query = request.parameter("query");
        if (!query || query->empty()) {
            return std::unexpected(CapabilityError{
                .capability_id = id().value,
                .message = "Missing required parameter 'query'.",
            });
        }

        const auto repository = request.execution_context().repository_selection;
        const auto search_results = search_.search(*query, repository);
        if (!search_results) {
            return std::unexpected(CapabilityError{
                .capability_id = id().value,
                .message = search_results.error().description,
            });
        }

        std::vector<KnowledgeObject> knowledge_objects;
        knowledge_objects.reserve(search_results->size());
        for (const auto& result : *search_results) {
            knowledge_objects.push_back(result.object);
        }

        context::SystemInstructions system{
            .ai_provider = provider_manager_->active_provider()
                               ? provider_manager_->active_provider()->id().value
                               : "AI-0000",
        };
        const auto package = context_builder_.build(request, knowledge_objects, system);
        if (const auto validation_error = validate_context_package(package)) {
            return std::unexpected(CapabilityError{
                .capability_id = id().value,
                .message = validation_error->message,
            });
        }

        const auto ai_response = provider_manager_->generate(package);
        if (!ai_response) {
            return std::unexpected(CapabilityError{
                .capability_id = id().value,
                .message = ai_response.error().message,
            });
        }

        std::ostringstream stream;
        stream << "Search results for '" << *query << "':\n";
        for (const auto& result : *search_results) {
            stream << "- " << result.object.identity.title << " ("
                   << result.object.identity.id.value << ")\n";
        }
        stream << "\nAI Provider:\n" << ai_response->generated_text;

        std::vector<Reference> references;
        references.reserve(search_results->size());
        for (const auto& result : *search_results) {
            references.push_back(Reference{
                .kind = "document",
                .identifier = result.object.identity.id.value,
                .title = result.object.identity.title,
            });
        }

        ResponseContent content{
            .primary = stream.str(),
        };
        content.structured["result_count"] = std::to_string(search_results->size());
        content.structured["context_package_id"] = package.metadata().package_id.value;

        std::vector<DiagnosticMessage> diagnostics = ai_response->warnings;
        if (diagnostics.empty()) {
            return PlatformResponse::success(request, std::move(content), std::move(references));
        }
        return PlatformResponse::partial_success(
            request,
            std::move(content),
            std::move(diagnostics),
            std::move(references));
    }

private:
    const services::ISearchService& search_;
    const context::ContextBuilder& context_builder_;
    std::shared_ptr<const ai::ProviderManager> provider_manager_;
};

class StatusReportingHandler final : public ICapabilityHandler {
public:
    explicit StatusReportingHandler(const services::IStatusService& status)
        : status_(status) {}

    [[nodiscard]] CapabilityId id() const override { return CapabilityId{"CAP-0002-STATUS"}; }
    [[nodiscard]] std::string name() const override { return "Status Reporting"; }

    [[nodiscard]] std::expected<PlatformResponse, CapabilityError> execute(
        const PlatformRequest& request) const override {
        return status_.report_status(request);
    }

private:
    const services::IStatusService& status_;
};

}  // namespace

void register_documentation_handlers(
    CapabilityRegistry& registry,
    const services::IDocumentationService& documentation,
    const services::ISearchService& search,
    const services::IStatusService& status,
    const context::ContextBuilder& context_builder,
    std::shared_ptr<const ai::ProviderManager> provider_manager) {
    (void)documentation;
    registry.register_handler(std::make_shared<DocumentationSearchHandler>(
        search,
        context_builder,
        provider_manager));
    registry.register_handler(std::make_shared<StatusReportingHandler>(status));
}

}  // namespace eve::capability::handlers
