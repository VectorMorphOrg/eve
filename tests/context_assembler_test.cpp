#include "eve/context/context_assembler.hpp"
#include "eve/core/platform_request.hpp"
#include "eve/knowledge/citation_engine.hpp"
#include "eve/knowledge/knowledge_graph.hpp"
#include "eve/knowledge/knowledge_index.hpp"
#include "eve/knowledge/ranking_engine.hpp"
#include "eve/knowledge/relationship_builder.hpp"
#include "eve/knowledge/search_engine.hpp"

#include <gtest/gtest.h>

#include <atomic>
#include <thread>
#include <vector>

namespace eve {
namespace {

KnowledgeObject make_object(
    std::string id,
    std::string document_identifier,
    std::string file_path,
    std::string title = {},
    std::string series = {},
    std::string part = {},
    std::string repository = "eve",
    DocumentType type = DocumentType::Specification) {
    KnowledgeObject object;
    object.identity.id = KnowledgeObjectId{std::move(id)};
    object.identity.document_identifier = std::move(document_identifier);
    object.identity.title = std::move(title);
    object.identity.document_type = type;
    object.identity.repository = std::move(repository);
    object.metadata.series = std::move(series);
    object.metadata.part = std::move(part);
    object.metadata.status = "Draft";
    object.source.file_path = std::move(file_path);
    object.search.priority_rank = priority_rank_for_document_type(type);
    object.search.excerpt = object.identity.title + " excerpt";
    return object;
}

SearchQuery make_query(std::string text, SearchMode mode = SearchMode::Auto) {
    SearchQuery query;
    query.text = std::move(text);
    query.mode = mode;
    return query;
}

struct AssemblerFixture {
    KnowledgeIndex index;
    KnowledgeGraph graph;
    SearchEngine search_engine;
    RankingEngine ranking_engine;
    CitationEngine citation_engine;
    context::ContextAssembler assembler;

    explicit AssemblerFixture(std::vector<KnowledgeObject> objects)
        : index(KnowledgeIndexBuilder::build(
              KnowledgeGraphBuilder::build(RelationshipBuilder::build(std::move(objects))))),
          graph(index.graph()),
          search_engine(index),
          ranking_engine(index),
          citation_engine(index, graph),
          assembler(index, graph) {}

    context::ContextAssemblyResult assemble(
        SearchQuery query,
        context::ContextSelectionPolicy policy = context::ContextSelectionPolicy::Full,
        context::ContextBudget budget = {}) const {
        const auto ranked = ranking_engine.rank(search_engine.search(query));
        const auto citations = citation_engine.cite(ranked);

        const auto request = PlatformRequest::create(CapabilityId{"CAP-0201"}, InterfaceType::Internal);
        context::ContextAssemblyRequest assembly_request{
            .platform_request = request,
            .search_query = query,
            .ranked_results = ranked,
            .citations = citations,
            .budget = budget,
            .policy = policy,
        };
        return assembler.assemble(assembly_request);
    }
};

TEST(ContextAssemblerTest, FullAssemblyCollectsEvidence) {
    auto primary = make_object(
        "EVE-0012",
        "EVE-0012",
        "docs/0000-core-platform-specifications/eve-0012-validation-specification.md",
        "Validation Specification",
        "0000",
        "Part I");
    primary.content.sections.push_back(
        KnowledgeSection{.title = "Requirements", .content = "Validation requirements content."});
    primary.content.headings.push_back(DocumentHeading{.level = 1, .text = "Overview"});
    primary.relationships.related_headers = {"include/eve/validation/validation_engine.hpp"};
    primary.relationships.related_sources = {"src/eve/validation/validation_engine.cpp"};
    primary.relationships.related_tests = {"tests/validation_engine_test.cpp"};
    primary.relationships.previous = "EVE-0011";
    primary.relationships.next = "EVE-0013";
    primary.relationships.parent = "EVE-0000";
    primary.relationships.children = {"EVE-0012-A"};

    auto supporting = make_object(
        "EVE-0011",
        "EVE-0011",
        "docs/0000-core-platform-specifications/eve-0011-platform-response-specification.md",
        "Platform Response Specification",
        "0000",
        "Part I");
    primary.relationships.references = {"EVE-0011"};

    AssemblerFixture fixture({primary, supporting});
    const auto result = fixture.assemble(make_query("EVE-0012", SearchMode::Identifier));

    EXPECT_EQ(result.policy, context::ContextSelectionPolicy::Full);
    EXPECT_FALSE(result.package.knowledge_objects().empty());
    EXPECT_FALSE(result.evidence.ranked_results.empty());
    EXPECT_FALSE(result.evidence.citation_bundles.empty());
    EXPECT_FALSE(result.evidence.sections.empty());
    EXPECT_FALSE(result.evidence.headings.empty());
    EXPECT_FALSE(result.evidence.implementation_headers.empty());
    EXPECT_FALSE(result.evidence.implementation_sources.empty());
    EXPECT_FALSE(result.evidence.validation_tests.empty());
    EXPECT_FALSE(result.evidence.navigation.empty());
    EXPECT_FALSE(result.package.citations().empty());
    EXPECT_EQ(result.evidence.identity.request_id, result.package.metadata().request_id);
}

TEST(ContextAssemblerTest, MinimalPolicyLimitsEvidence) {
    auto primary = make_object(
        "EVE-0012",
        "EVE-0012",
        "docs/0000-core-platform-specifications/eve-0012-validation-specification.md",
        "Validation Specification",
        "0000",
        "Part I");
    primary.content.sections.push_back(KnowledgeSection{.title = "Requirements", .content = "A"});
    primary.content.sections.push_back(KnowledgeSection{.title = "Details", .content = "B"});
    primary.content.headings.push_back(DocumentHeading{.level = 1, .text = "Overview"});
    primary.content.headings.push_back(DocumentHeading{.level = 2, .text = "Scope"});
    primary.relationships.related_headers = {"include/eve/validation/validation_engine.hpp"};
    primary.relationships.related_sources = {"src/eve/validation/validation_engine.cpp"};
    primary.relationships.related_tests = {"tests/validation_engine_test.cpp"};

    auto secondary = make_object(
        "EVE-0011",
        "EVE-0011",
        "docs/0000-core-platform-specifications/eve-0011-platform-response-specification.md",
        "Platform Response Specification",
        "0000",
        "Part I");

    AssemblerFixture fixture({primary, secondary});
    const auto result = fixture.assemble(
        make_query("EVE", SearchMode::FullText),
        context::ContextSelectionPolicy::Minimal,
        context::ContextBudget{
            .max_documents = 5,
            .max_sections = 5,
            .max_headings = 5,
            .max_implementation_files = 5,
            .max_test_files = 5,
            .max_references = 5,
            .max_estimated_tokens = 32000,
        });

    EXPECT_EQ(result.policy, context::ContextSelectionPolicy::Minimal);
    EXPECT_LE(result.evidence.ranked_results.size(), 1U);
    EXPECT_LE(result.evidence.sections.size(), 1U);
    EXPECT_LE(result.evidence.headings.size(), 1U);
    EXPECT_TRUE(result.evidence.implementation_headers.empty());
    EXPECT_TRUE(result.evidence.implementation_sources.empty());
    EXPECT_TRUE(result.evidence.validation_tests.empty());
}

TEST(ContextAssemblerTest, BalancedPolicyDefersImplementationUntilAfterSpecifications) {
    auto primary = make_object(
        "EVE-0012",
        "EVE-0012",
        "docs/0000-core-platform-specifications/eve-0012-validation-specification.md",
        "Validation Specification",
        "0000",
        "Part I");
    primary.content.sections.push_back(KnowledgeSection{.title = "Requirements", .content = "A"});
    primary.relationships.related_headers = {"include/eve/validation/validation_engine.hpp"};
    primary.relationships.related_sources = {"src/eve/validation/validation_engine.cpp"};

    auto secondary = make_object(
        "EVE-0011",
        "EVE-0011",
        "docs/0000-core-platform-specifications/eve-0011-platform-response-specification.md",
        "Platform Response Specification",
        "0000",
        "Part I");
    primary.relationships.references = {"EVE-0011"};

    AssemblerFixture fixture({primary, secondary});
    const auto result = fixture.assemble(
        make_query("EVE-0012", SearchMode::Identifier),
        context::ContextSelectionPolicy::Balanced,
        context::ContextBudget{
            .max_documents = 2,
            .max_sections = 2,
            .max_headings = 2,
            .max_implementation_files = 2,
            .max_test_files = 2,
            .max_references = 5,
            .max_estimated_tokens = 32000,
        });

    EXPECT_EQ(result.policy, context::ContextSelectionPolicy::Balanced);
    EXPECT_FALSE(result.evidence.sections.empty());
    EXPECT_FALSE(result.evidence.implementation_headers.empty());
    EXPECT_LE(result.evidence.implementation_headers.size(), result.budget.max_implementation_files);
}

TEST(ContextAssemblerTest, BudgetEnforcementStopsAtDocumentLimit) {
    auto first = make_object(
        "EVE-0001",
        "EVE-0001",
        "docs/0000-core-platform-specifications/eve-0001-system-architecture.md",
        "System Architecture",
        "0000",
        "Part I");
    auto second = make_object(
        "EVE-0002",
        "EVE-0002",
        "docs/0000-core-platform-specifications/eve-0002-capability-specification.md",
        "Capability Specification",
        "0000",
        "Part I");

    AssemblerFixture fixture({first, second});
    const auto result = fixture.assemble(
        make_query("eve", SearchMode::Repository),
        context::ContextSelectionPolicy::Full,
        context::ContextBudget{
            .max_documents = 1,
            .max_sections = 50,
            .max_headings = 50,
            .max_implementation_files = 20,
            .max_test_files = 20,
            .max_references = 50,
            .max_estimated_tokens = 32000,
        });

    EXPECT_EQ(result.evidence.budget.selected_documents, 1U);
    EXPECT_TRUE(result.evidence.budget.budget_truncated);
    EXPECT_FALSE(result.package.diagnostics().builder_notes.empty());
}

TEST(ContextAssemblerTest, BudgetEnforcementStopsAtTokenLimit) {
    auto primary = make_object(
        "EVE-0012",
        "EVE-0012",
        "docs/0000-core-platform-specifications/eve-0012-validation-specification.md",
        "Validation Specification",
        "0000",
        "Part I");
    primary.search.excerpt = std::string(400, 'x');
    primary.content.sections.push_back(
        KnowledgeSection{.title = "Requirements", .content = std::string(400, 'y')});

    AssemblerFixture fixture({primary});
    const auto result = fixture.assemble(
        make_query("EVE-0012", SearchMode::Identifier),
        context::ContextSelectionPolicy::Full,
        context::ContextBudget{
            .max_documents = 5,
            .max_sections = 5,
            .max_headings = 5,
            .max_implementation_files = 5,
            .max_test_files = 5,
            .max_references = 5,
            .max_estimated_tokens = 50,
        });

    EXPECT_TRUE(result.evidence.budget.budget_truncated);
    EXPECT_LE(result.evidence.budget.estimated_tokens, 50U);
}

TEST(ContextAssemblerTest, SectionSelectionPreservesOrder) {
    auto primary = make_object(
        "EVE-0012",
        "EVE-0012",
        "docs/0000-core-platform-specifications/eve-0012-validation-specification.md",
        "Validation Specification",
        "0000",
        "Part I");
    primary.content.sections.push_back(KnowledgeSection{.title = "First", .content = "one"});
    primary.content.sections.push_back(KnowledgeSection{.title = "Second", .content = "two"});

    AssemblerFixture fixture({primary});
    const auto result = fixture.assemble(make_query("EVE-0012", SearchMode::Identifier));

    ASSERT_EQ(result.evidence.sections.size(), 2U);
    EXPECT_EQ(result.evidence.sections[0].title, "First");
    EXPECT_EQ(result.evidence.sections[1].title, "Second");
}

TEST(ContextAssemblerTest, HeadingSelectionPreservesOrder) {
    auto primary = make_object(
        "EVE-0012",
        "EVE-0012",
        "docs/0000-core-platform-specifications/eve-0012-validation-specification.md",
        "Validation Specification",
        "0000",
        "Part I");
    primary.content.headings.push_back(DocumentHeading{.level = 1, .text = "Alpha"});
    primary.content.headings.push_back(DocumentHeading{.level = 2, .text = "Beta"});

    AssemblerFixture fixture({primary});
    const auto result = fixture.assemble(make_query("EVE-0012", SearchMode::Identifier));

    ASSERT_EQ(result.evidence.headings.size(), 2U);
    EXPECT_EQ(result.evidence.headings[0].text, "Alpha");
    EXPECT_EQ(result.evidence.headings[1].text, "Beta");
}

TEST(ContextAssemblerTest, CitationPropagationMapsToContextPackage) {
    auto primary = make_object(
        "EVE-0012",
        "EVE-0012",
        "docs/0000-core-platform-specifications/eve-0012-validation-specification.md",
        "Validation Specification",
        "0000",
        "Part I");
    primary.content.sections.push_back(KnowledgeSection{.title = "Requirements", .content = "A"});

    AssemblerFixture fixture({primary});
    const auto result = fixture.assemble(make_query("EVE-0012", SearchMode::Identifier));

    ASSERT_FALSE(result.package.citations().empty());
    EXPECT_EQ(result.package.citations().front().identifier, "EVE-0012");
    EXPECT_EQ(result.package.citations().front().title, "Validation Specification");
    ASSERT_TRUE(result.package.citations().front().section.has_value());
    EXPECT_EQ(*result.package.citations().front().section, "Requirements");
}

TEST(ContextAssemblerTest, ImplementationFileSelectionIncludesHeadersAndSources) {
    auto primary = make_object(
        "EVE-0012",
        "EVE-0012",
        "docs/0000-core-platform-specifications/eve-0012-validation-specification.md",
        "Validation Specification",
        "0000",
        "Part I");
    primary.relationships.related_headers = {"include/eve/validation/validation_engine.hpp"};
    primary.relationships.related_sources = {"src/eve/validation/validation_engine.cpp"};

    AssemblerFixture fixture({primary});
    const auto result = fixture.assemble(make_query("EVE-0012", SearchMode::Identifier));

    EXPECT_EQ(result.evidence.implementation_headers.size(), 1U);
    EXPECT_EQ(result.evidence.implementation_sources.size(), 1U);
    EXPECT_EQ(result.evidence.implementation_headers.front(), "include/eve/validation/validation_engine.hpp");
    EXPECT_EQ(result.evidence.implementation_sources.front(), "src/eve/validation/validation_engine.cpp");
}

TEST(ContextAssemblerTest, NavigationPropagationIncludesRelationships) {
    auto primary = make_object(
        "EVE-0012",
        "EVE-0012",
        "docs/0000-core-platform-specifications/eve-0012-validation-specification.md",
        "Validation Specification",
        "0000",
        "Part I");
    primary.relationships.previous = "EVE-0011";
    primary.relationships.next = "EVE-0013";
    primary.relationships.parent = "EVE-0000";
    primary.relationships.children = {"EVE-0012-A", "EVE-0012-B"};

    AssemblerFixture fixture({primary});
    const auto result = fixture.assemble(make_query("EVE-0012", SearchMode::Identifier));

    ASSERT_EQ(result.evidence.navigation.size(), 1U);
    EXPECT_EQ(result.evidence.navigation.front().previous, "EVE-0011");
    EXPECT_EQ(result.evidence.navigation.front().next, "EVE-0013");
    EXPECT_EQ(result.evidence.navigation.front().parent, "EVE-0000");
    EXPECT_EQ(result.evidence.navigation.front().children, std::vector<std::string>({"EVE-0012-A", "EVE-0012-B"}));
}

TEST(ContextAssemblerTest, MetadataPropagationIncludesRepositorySeriesPartStatus) {
    auto primary = make_object(
        "EVE-0012",
        "EVE-0012",
        "docs/0000-core-platform-specifications/eve-0012-validation-specification.md",
        "Validation Specification",
        "0000",
        "Part I");

    AssemblerFixture fixture({primary});
    const auto result = fixture.assemble(make_query("EVE-0012", SearchMode::Identifier));

    ASSERT_EQ(result.evidence.document_metadata.size(), 1U);
    EXPECT_EQ(result.evidence.document_metadata.front().repository, "eve");
    EXPECT_EQ(result.evidence.document_metadata.front().series, "0000");
    EXPECT_EQ(result.evidence.document_metadata.front().part, "Part I");
    EXPECT_EQ(result.evidence.document_metadata.front().status, "Draft");
    EXPECT_EQ(result.evidence.document_metadata.front().document_type, DocumentType::Specification);
}

TEST(ContextAssemblerTest, DiagnosticsReportMissingEvidence) {
    RankedSearchResultSet ranked{
        .query = make_query("MISSING", SearchMode::Identifier),
        .resolved_mode = SearchMode::Identifier,
        .result_count = 1,
        .results =
            {RankedSearchResult{
                .candidate =
                    SearchEngineResult{
                        .object_identifier = "MISSING",
                        .document_identifier = "MISSING",
                        .title = "Missing",
                    },
                .rank_score = 10,
                .rank_position = 0,
            }},
    };
    CitationResultSet citations{
        .query = ranked.query,
        .resolved_mode = ranked.resolved_mode,
        .bundle_count = 0,
    };

    KnowledgeObject placeholder = make_object(
        "EVE-0001",
        "EVE-0001",
        "docs/0000-core-platform-specifications/eve-0001-system-architecture.md",
        "System Architecture");
    AssemblerFixture fixture({placeholder});

    const auto request = PlatformRequest::create(CapabilityId{"CAP-0201"}, InterfaceType::Internal);
    context::ContextAssemblyRequest assembly_request{
        .platform_request = request,
        .search_query = ranked.query,
        .ranked_results = ranked,
        .citations = citations,
    };
    const auto result = fixture.assembler.assemble(assembly_request);

    EXPECT_FALSE(result.package.diagnostics().missing_documentation.empty());
    EXPECT_TRUE(result.package.diagnostics().missing_documentation.front().find("MISSING") != std::string::npos);
}

TEST(ContextAssemblerTest, EmptyInputsProduceDiagnostics) {
    AssemblerFixture fixture({make_object(
        "EVE-0001",
        "EVE-0001",
        "docs/0000-core-platform-specifications/eve-0001-system-architecture.md",
        "System Architecture")});

    const auto request = PlatformRequest::create(CapabilityId{"CAP-0201"}, InterfaceType::Internal);
    context::ContextAssemblyRequest assembly_request{
        .platform_request = request,
        .search_query = make_query("", SearchMode::Identifier),
    };
    const auto result = fixture.assembler.assemble(assembly_request);

    EXPECT_TRUE(result.evidence.ranked_results.empty());
    EXPECT_FALSE(result.package.diagnostics().missing_documentation.empty());
}

TEST(ContextAssemblerTest, StableOrderingMatchesRankedResults) {
    auto first = make_object(
        "EVE-0001",
        "EVE-0001",
        "docs/0000-core-platform-specifications/eve-0001-system-architecture.md",
        "System Architecture",
        "0000",
        "Part I");
    auto second = make_object(
        "EVE-0002",
        "EVE-0002",
        "docs/0000-core-platform-specifications/eve-0002-capability-specification.md",
        "Capability Specification",
        "0000",
        "Part I");

    AssemblerFixture fixture({first, second});
    const auto result = fixture.assemble(
        make_query("eve", SearchMode::Repository),
        context::ContextSelectionPolicy::Full,
        context::ContextBudget{
            .max_documents = 2,
            .max_sections = 50,
            .max_headings = 50,
            .max_implementation_files = 20,
            .max_test_files = 20,
            .max_references = 50,
            .max_estimated_tokens = 32000,
        });

    ASSERT_EQ(result.evidence.ranked_results.size(), 2U);
    EXPECT_LE(
        result.evidence.ranked_results[0].rank_position,
        result.evidence.ranked_results[1].rank_position);
}

TEST(ContextAssemblerTest, DeterministicOutputForIdenticalRequests) {
    auto primary = make_object(
        "EVE-0012",
        "EVE-0012",
        "docs/0000-core-platform-specifications/eve-0012-validation-specification.md",
        "Validation Specification",
        "0000",
        "Part I");
    primary.content.sections.push_back(KnowledgeSection{.title = "Requirements", .content = "A"});
    primary.relationships.related_headers = {"include/eve/validation/validation_engine.hpp"};

    AssemblerFixture fixture({primary});
    const auto query = make_query("EVE-0012", SearchMode::Identifier);
    const auto first = fixture.assemble(query);
    const auto second = fixture.assemble(query);

    EXPECT_EQ(first.evidence.sections.size(), second.evidence.sections.size());
    EXPECT_EQ(first.evidence.headings.size(), second.evidence.headings.size());
    EXPECT_EQ(first.evidence.implementation_headers, second.evidence.implementation_headers);
    EXPECT_EQ(first.package.knowledge_objects().size(), second.package.knowledge_objects().size());
    EXPECT_EQ(first.evidence.budget.estimated_tokens, second.evidence.budget.estimated_tokens);
}

TEST(ContextAssemblerTest, ConcurrentReadSafetyReturnsConsistentResults) {
    auto primary = make_object(
        "EVE-0012",
        "EVE-0012",
        "docs/0000-core-platform-specifications/eve-0012-validation-specification.md",
        "Validation Specification",
        "0000",
        "Part I");
    primary.content.sections.push_back(KnowledgeSection{.title = "Requirements", .content = "A"});

    AssemblerFixture fixture({primary});
    const auto query = make_query("EVE-0012", SearchMode::Identifier);
    const auto expected = fixture.assemble(query);

    std::atomic<int> mismatches{0};
    std::vector<std::thread> threads;
    for (int i = 0; i < 8; ++i) {
        threads.emplace_back([&]() {
            const auto result = fixture.assemble(query);
            if (result.evidence.sections.size() != expected.evidence.sections.size() ||
                result.evidence.budget.estimated_tokens != expected.evidence.budget.estimated_tokens) {
                mismatches.fetch_add(1);
            }
        });
    }
    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(mismatches.load(), 0);
}

TEST(ContextAssemblerTest, ImmutabilityPreservesResultAfterInputMutation) {
    auto primary = make_object(
        "EVE-0012",
        "EVE-0012",
        "docs/0000-core-platform-specifications/eve-0012-validation-specification.md",
        "Validation Specification",
        "0000",
        "Part I");
    primary.content.sections.push_back(KnowledgeSection{.title = "Requirements", .content = "A"});

    AssemblerFixture fixture({primary});
    const auto request = PlatformRequest::create(CapabilityId{"CAP-0201"}, InterfaceType::Internal);
    const auto ranked = fixture.ranking_engine.rank(fixture.search_engine.search(make_query("EVE-0012", SearchMode::Identifier)));
    auto citations = fixture.citation_engine.cite(ranked);

    context::ContextAssemblyRequest assembly_request{
        .platform_request = request,
        .search_query = make_query("EVE-0012", SearchMode::Identifier),
        .ranked_results = ranked,
        .citations = citations,
    };
    const auto result = fixture.assembler.assemble(assembly_request);
    const auto section_count = result.evidence.sections.size();
    const auto token_count = result.evidence.budget.estimated_tokens;

    assembly_request.ranked_results.results.clear();
    assembly_request.citations.bundles.clear();
    citations.bundles.clear();

    EXPECT_EQ(result.evidence.sections.size(), section_count);
    EXPECT_EQ(result.evidence.budget.estimated_tokens, token_count);
}

TEST(ContextAssemblerTest, EffectiveBudgetForPolicyAppliesMinimalLimits) {
    context::ContextBudget budget{
        .max_documents = 10,
        .max_sections = 10,
        .max_headings = 10,
        .max_implementation_files = 10,
        .max_test_files = 10,
        .max_references = 10,
        .max_estimated_tokens = 32000,
    };

    const auto minimal = context::effective_budget_for_policy(budget, context::ContextSelectionPolicy::Minimal);
    EXPECT_EQ(minimal.max_documents, 1U);
    EXPECT_EQ(minimal.max_implementation_files, 0U);
    EXPECT_EQ(minimal.max_test_files, 0U);
}

}  // namespace
}  // namespace eve
