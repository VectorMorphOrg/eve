#include "eve/core/core_platform.hpp"

#include "eve/ai/provider_manager.hpp"
#include "eve/capability/handlers/capability_handlers.hpp"
#include "eve/knowledge/knowledge_object.hpp"
#include "eve/services/service_implementations.hpp"
#include "eve/validation/validation_engine.hpp"

#include <format>
#include <memory>

namespace eve {

CorePlatform PlatformBootstrap::create(
    const std::filesystem::path& project_root,
    const std::filesystem::path& config_path) {
    const auto resolved_config = project_root / config_path;

    auto configuration = std::make_shared<services::ConfigurationService>(resolved_config);
    auto logging = std::make_shared<services::LoggingService>();
    auto knowledge_store = std::make_shared<InMemoryKnowledgeStore>();

    std::vector<KnowledgeObject> indexed_objects;
    for (const auto& docs_path : configuration->documentation_paths()) {
        const auto absolute_docs = project_root / docs_path;
        auto objects = DocumentIndexer::index_directory(
            absolute_docs,
            configuration->default_repository());
        indexed_objects.insert(
            indexed_objects.end(),
            std::make_move_iterator(objects.begin()),
            std::make_move_iterator(objects.end()));
    }
    knowledge_store->load(std::move(indexed_objects));
    logging->info(std::format("Indexed {} knowledge objects.", knowledge_store->document_count()));

    auto documentation = std::make_shared<services::DocumentationService>(knowledge_store);
    auto search = std::make_shared<services::SearchService>(knowledge_store);
    auto diagnostics = std::make_shared<services::DiagnosticsService>(*knowledge_store);

    auto provider_manager = std::make_shared<ai::ProviderManager>();
    provider_manager->register_provider(std::make_shared<ai::NullProvider>());
    provider_manager->set_active_provider(ProviderId{configuration->active_ai_provider()});

    auto status = std::make_shared<services::StatusService>(
        *configuration,
        *knowledge_store,
        provider_manager);

    const context::ContextBuilder context_builder(configuration->context_limit_chars());

    auto registry = std::make_shared<capability::CapabilityRegistry>();
    capability::handlers::register_core_handlers(
        *registry,
        *configuration,
        *status,
        *diagnostics,
        context_builder);
    capability::handlers::register_documentation_handlers(
        *registry,
        *documentation,
        *search,
        *status,
        context_builder,
        provider_manager);

    registry->register_alias("search", CapabilityId{"CAP-0102"});
    registry->register_alias("status", CapabilityId{"CAP-0002-STATUS"});

    const capability::CapabilityEngine capability_engine(registry);
    auto validation_engine =
        std::make_shared<validation::ValidationEngine>(capability_engine.registry());
    const dispatcher::CommandDispatcher dispatcher(
        validation_engine,
        capability_engine,
        logging);

    CorePlatformDependencies dependencies{
        .configuration = configuration,
        .logging = logging,
        .documentation = documentation,
        .search = search,
        .status = status,
        .diagnostics = diagnostics,
        .knowledge_store = knowledge_store,
        .context_builder = context_builder,
        .provider_manager = provider_manager,
        .validation_engine = validation_engine,
        .capability_engine = capability_engine,
        .dispatcher = dispatcher,
    };

    return CorePlatform(std::move(dependencies));
}

}  // namespace eve
