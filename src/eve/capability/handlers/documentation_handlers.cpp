#include "eve/capability/handlers/capability_handlers.hpp"

#include "eve/context/context_assembler.hpp"
#include "eve/knowledge/reasoning_pipeline.hpp"

#include <format>
#include <sstream>

namespace eve::capability::handlers {
namespace {

ResponseTraceInformation make_pipeline_trace(
    const std::string& provider_id,
    const std::string& provider_name,
    std::size_t search_count,
    std::size_t ranked_count,
    std::size_t citation_count,
    std::size_t knowledge_object_count) {
    ResponseTraceInformation trace{
        .services_executed =
            {
                "SearchEngine",
                "RankingEngine",
                "CitationEngine",
                "ContextAssembler",
                "ProviderFormatter",
                provider_name,
            },
        .provider_selected = provider_id,
        .entries =
            {
                TraceEntry{
                    .component = "SearchEngine",
                    .detail = std::format("Search completed with {} candidate(s).", search_count),
                },
                TraceEntry{
                    .component = "RankingEngine",
                    .detail = std::format("Ranking completed with {} ranked result(s).", ranked_count),
                },
                TraceEntry{
                    .component = "CitationEngine",
                    .detail = std::format("Citation completed with {} bundle(s).", citation_count),
                },
                TraceEntry{
                    .component = "ContextAssembler",
                    .detail = std::format(
                        "Context assembled with {} knowledge object(s).",
                        knowledge_object_count),
                },
                TraceEntry{
                    .component = "ProviderFormatter",
                    .detail = std::format(
                        "Provider request formatted for {} with {} knowledge object(s).",
                        provider_name,
                        knowledge_object_count),
                },
                TraceEntry{
                    .component = provider_name,
                    .detail = "Provider invoked.",
                },
            },
    };
    return trace;
}

class DocumentationSearchHandler final : public ICapabilityHandler {
public:
    DocumentationSearchHandler(
        const ReasoningPipeline& reasoning_pipeline,
        std::shared_ptr<const ai::ProviderManager> provider_manager,
        std::size_t context_limit_chars)
        : reasoning_pipeline_(reasoning_pipeline),
          provider_manager_(std::move(provider_manager)),
          context_limit_chars_(context_limit_chars) {}

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

        SearchQuery search_query{
            .text = *query,
            .mode = SearchMode::Auto,
            .exact_match = false,
            .partial_match = true,
        };

        auto search_results = reasoning_pipeline_.search(search_query);
        const auto repository = request.execution_context().repository_selection;
        if (repository) {
            search_results =
                filter_search_results_by_repository(std::move(search_results), *repository);
        }

        if (search_results.results.empty()) {
            return std::unexpected(CapabilityError{
                .capability_id = id().value,
                .message = std::format("No search results for '{}'.", *query),
            });
        }

        const auto ranked_results = reasoning_pipeline_.rank(search_results);
        const auto citation_results = reasoning_pipeline_.cite(ranked_results);

        context::SystemInstructions system{
            .ai_provider = provider_manager_->active_provider()
                               ? provider_manager_->active_provider()->id().value
                               : "AI-0000",
        };

        context::ContextBudget budget{
            .max_estimated_tokens = std::max<std::size_t>(1, context_limit_chars_ / 4),
        };

        context::ContextAssemblyRequest assembly_request{
            .platform_request = request,
            .search_query = search_query,
            .ranked_results = ranked_results,
            .citations = citation_results,
            .budget = budget,
            .policy = context::ContextSelectionPolicy::Balanced,
            .system = system,
        };

        const auto assembly = reasoning_pipeline_.assemble(assembly_request);
        const auto& package = assembly.package;

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
        for (const auto& ranked : assembly.evidence.ranked_results) {
            stream << "- " << ranked.candidate.title << " (" << ranked.candidate.object_identifier
                   << ")\n";
        }
        stream << "\nAI Provider:\n" << ai_response->generated_text;

        std::vector<Reference> references;
        references.reserve(package.knowledge_objects().size());
        for (const auto& object : package.knowledge_objects()) {
            references.push_back(Reference{
                .kind = "document",
                .identifier = object.identity.id.value,
                .title = object.identity.title,
            });
        }

        ResponseContent content{
            .primary = stream.str(),
        };
        content.structured["result_count"] = std::to_string(assembly.evidence.ranked_results.size());
        content.structured["context_package_id"] = package.metadata().package_id.value;
        content.structured["reasoning_pipeline"] = "4.1";
        content.structured["context_assembly_policy"] = context::to_string(assembly.policy);
        content.structured["search_result_count"] =
            std::to_string(search_results.result_count);
        content.structured["ranked_result_count"] = std::to_string(ranked_results.result_count);
        content.structured["citation_bundle_count"] =
            std::to_string(citation_results.bundle_count);

        const auto active_provider = provider_manager_->active_provider();
        const auto provider_id = active_provider ? active_provider->id().value : "AI-0000";
        const auto provider_name = active_provider ? active_provider->name() : "NullProvider";

        auto trace = make_pipeline_trace(
            provider_id,
            provider_name,
            search_results.result_count,
            ranked_results.result_count,
            citation_results.bundle_count,
            package.knowledge_objects().size());

        std::vector<DiagnosticMessage> diagnostics = ai_response->warnings;
        if (diagnostics.empty()) {
            return PlatformResponse::success(request, std::move(content), std::move(references))
                .with_trace(std::move(trace));
        }
        return PlatformResponse::partial_success(
                   request,
                   std::move(content),
                   std::move(diagnostics),
                   std::move(references))
            .with_trace(std::move(trace));
    }

private:
    const ReasoningPipeline& reasoning_pipeline_;
    std::shared_ptr<const ai::ProviderManager> provider_manager_;
    std::size_t context_limit_chars_;
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
    const services::IStatusService& status,
    const ReasoningPipeline& reasoning_pipeline,
    std::shared_ptr<const ai::ProviderManager> provider_manager,
    std::size_t context_limit_chars) {
    (void)documentation;
    registry.register_handler(std::make_shared<DocumentationSearchHandler>(
        reasoning_pipeline,
        std::move(provider_manager),
        context_limit_chars));
    registry.register_handler(std::make_shared<StatusReportingHandler>(status));
}

}  // namespace eve::capability::handlers
