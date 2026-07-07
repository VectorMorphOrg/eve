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
    return object;
}

struct CitationFixture {
    KnowledgeIndex index;
    KnowledgeGraph graph;
    SearchEngine search_engine;
    RankingEngine ranking_engine;
    CitationEngine citation_engine;

    explicit CitationFixture(std::vector<KnowledgeObject> objects)
        : index(KnowledgeIndexBuilder::build(
              KnowledgeGraphBuilder::build(RelationshipBuilder::build(std::move(objects))))),
          graph(index.graph()),
          search_engine(index),
          ranking_engine(index),
          citation_engine(index, graph) {}

    CitationResultSet cite(SearchQuery query) const {
        return citation_engine.cite(ranking_engine.rank(search_engine.search(query)));
    }
};

SearchQuery make_query(std::string text, SearchMode mode = SearchMode::Auto) {
    SearchQuery query;
    query.text = std::move(text);
    query.mode = mode;
    return query;
}

TEST(CitationEngineTest, BuildsPrimarySpecificationCitation) {
    auto spec = make_object(
        "EVE-0012",
        "EVE-0012",
        "docs/0000-core-platform-specifications/eve-0012-validation-specification.md",
        "Validation Specification",
        "0000",
        "Part I");
    spec.content.sections.push_back(KnowledgeSection{.title = "Requirements"});
    spec.content.headings.push_back(DocumentHeading{.level = 1, .text = "Overview"});

    CitationFixture fixture({spec});
    const auto result_set = fixture.cite(make_query("EVE-0012", SearchMode::Identifier));

    ASSERT_EQ(result_set.bundle_count, 1U);
    EXPECT_EQ(result_set.bundles.front().primary.citation_type, CitationType::PrimarySpecification);
    EXPECT_EQ(result_set.bundles.front().primary.document_identifier, "EVE-0012");
    EXPECT_GE(result_set.bundles.front().primary.citation_confidence, 50);
    EXPECT_EQ(result_set.bundles.front().primary.supporting_section_titles, std::vector<std::string>({"Requirements"}));
}

TEST(CitationEngineTest, CollectsImplementationSourceCitations) {
    auto spec = make_object(
        "EVE-0012",
        "EVE-0012",
        "docs/0000-core-platform-specifications/eve-0012-validation-specification.md",
        "Validation Specification",
        "0000",
        "Part I");
    spec.relationships.related_headers = {"include/eve/validation/validation_engine.hpp"};
    spec.relationships.related_sources = {"src/eve/validation/validation_engine.cpp"};

    CitationFixture fixture({spec});
    const auto result_set = fixture.cite(make_query("EVE-0012", SearchMode::Identifier));

    ASSERT_EQ(result_set.bundles.front().implementation.size(), 2U);
    EXPECT_EQ(
        result_set.bundles.front().implementation[0].citation_type,
        CitationType::ImplementationHeader);
    EXPECT_EQ(
        result_set.bundles.front().implementation[1].citation_type,
        CitationType::ImplementationSource);
    EXPECT_FALSE(result_set.bundles.front().implementation.front().supporting_source_files.empty());
}

TEST(CitationEngineTest, CollectsValidationTestCitations) {
    auto spec = make_object(
        "EVE-0012",
        "EVE-0012",
        "docs/0000-core-platform-specifications/eve-0012-validation-specification.md",
        "Validation Specification",
        "0000",
        "Part I");
    spec.relationships.related_tests = {"tests/validation_engine_test.cpp"};

    CitationFixture fixture({spec});
    const auto result_set = fixture.cite(make_query("EVE-0012", SearchMode::Identifier));

    ASSERT_EQ(result_set.bundles.front().validation.size(), 1U);
    EXPECT_EQ(result_set.bundles.front().validation.front().citation_type, CitationType::ValidationTest);
    EXPECT_EQ(
        result_set.bundles.front().validation.front().supporting_test_files,
        std::vector<std::string>({"tests/validation_engine_test.cpp"}));
}

TEST(CitationEngineTest, CollectsReferenceAndRelatedDocumentCitations) {
    auto first = make_object(
        "EVE-0010",
        "EVE-0010",
        "docs/0000-core-platform-specifications/eve-0010-platform-request-specification.md",
        "Platform Request Specification",
        "0000",
        "Part I");
    first.content.raw_markdown = "Depends on EVE-0011.";
    first.search.search_text = first.content.raw_markdown;

    auto second = make_object(
        "EVE-0011",
        "EVE-0011",
        "docs/0000-core-platform-specifications/eve-0011-platform-response-specification.md",
        "Platform Response Specification",
        "0000",
        "Part I");

    CitationFixture fixture({first, second});
    const auto result_set = fixture.cite(make_query("EVE-0010", SearchMode::Identifier));

    ASSERT_FALSE(result_set.bundles.front().supporting_references.empty());
    ASSERT_FALSE(result_set.bundles.front().related_documents.empty());
    EXPECT_EQ(result_set.bundles.front().related_documents.front().object_identifier, "EVE-0011");
}

TEST(CitationEngineTest, CollectsNavigationCitations) {
    auto readme = make_object(
        "README-0000",
        "README-0000",
        "docs/0000-core-platform-specifications/README.md",
        "Core Platform README",
        "0000",
        "Part I",
        "eve",
        DocumentType::Readme);
    auto spec = make_object(
        "EVE-0010",
        "EVE-0010",
        "docs/0000-core-platform-specifications/eve-0010-platform-request-specification.md",
        "Platform Request Specification",
        "0000",
        "Part I");
    spec.relationships.previous = "README-0000";
    spec.relationships.next = "EVE-0011";

    auto next = make_object(
        "EVE-0011",
        "EVE-0011",
        "docs/0000-core-platform-specifications/eve-0011-platform-response-specification.md",
        "Platform Response Specification",
        "0000",
        "Part I");

    CitationFixture fixture({readme, spec, next});
    const auto result_set = fixture.cite(make_query("EVE-0010", SearchMode::Identifier));

    ASSERT_GE(result_set.bundles.front().navigation.size(), 2U);
    EXPECT_EQ(result_set.bundles.front().navigation.front().citation_type, CitationType::Navigation);
}

TEST(CitationEngineTest, CalculatesDeterministicConfidenceFromEvidence) {
    auto rich = make_object(
        "EVE-0012",
        "EVE-0012",
        "docs/0000-core-platform-specifications/eve-0012-validation-specification.md",
        "Validation Specification",
        "0000",
        "Part I");
    rich.content.sections.push_back(KnowledgeSection{.title = "Requirements"});
    rich.content.headings.push_back(DocumentHeading{.level = 1, .text = "Overview"});
    rich.relationships.references = {"EVE-0010"};
    rich.relationships.related_sources = {"src/eve/validation/validation_engine.cpp"};
    rich.relationships.related_tests = {"tests/validation_engine_test.cpp"};

    auto sparse = make_object(
        "DOC-0001",
        "DOC-0001",
        "docs/misc/other.md",
        "Other Document",
        "9999",
        "Part IX",
        "eve",
        DocumentType::Other);

    CitationFixture fixture({rich, sparse});

    const auto rich_set = fixture.cite(make_query("EVE-0012", SearchMode::Identifier));
    const auto sparse_set = fixture.cite(make_query("DOC-0001", SearchMode::Identifier));

    EXPECT_GT(rich_set.bundles.front().primary.citation_confidence, sparse_set.bundles.front().primary.citation_confidence);
    const auto first_confidence = rich_set.bundles.front().primary.citation_confidence;
    const auto second_confidence = fixture.cite(make_query("EVE-0012", SearchMode::Identifier))
                                       .bundles.front()
                                       .primary.citation_confidence;
    EXPECT_EQ(first_confidence, second_confidence);
}

TEST(CitationEngineTest, EliminatesDuplicateCitationsAndPreservesOrdering) {
    auto spec = make_object(
        "EVE-0012",
        "EVE-0012",
        "docs/0000-core-platform-specifications/eve-0012-validation-specification.md",
        "Validation Specification",
        "0000",
        "Part I");
    spec.relationships.related_sources = {
        "src/eve/validation/validation_engine.cpp",
        "src/eve/validation/validation_engine.cpp",
    };
    spec.relationships.references = {"EVE-0010", "EVE-0010"};

    CitationFixture fixture({spec});
    const auto result_set = fixture.cite(make_query("EVE-0012", SearchMode::Identifier));

    EXPECT_EQ(result_set.bundles.front().implementation.size(), 1U);
    EXPECT_LE(result_set.bundles.front().supporting_references.size(), 1U);
}

TEST(CitationEngineTest, ReturnsEmptyResultSetForEmptyRankedInput) {
    CitationFixture fixture({
        make_object(
            "EVE-0010",
            "EVE-0010",
            "docs/0000-core-platform-specifications/eve-0010-platform-request-specification.md",
            "Platform Request Specification",
            "0000",
            "Part I"),
    });

    const auto ranked = RankedSearchResultSet{};
    const auto result_set = fixture.citation_engine.cite(ranked);

    EXPECT_TRUE(result_set.bundles.empty());
    EXPECT_EQ(result_set.bundle_count, 0U);
}

TEST(CitationEngineTest, HandlesMissingSupportingEvidence) {
    auto spec = make_object(
        "EVE-0010",
        "EVE-0010",
        "docs/0000-core-platform-specifications/eve-0010-platform-request-specification.md",
        "Platform Request Specification",
        "0000",
        "Part I");
    spec.relationships.references = {"EVE-9999"};

    CitationFixture fixture({spec});
    const auto result_set = fixture.cite(make_query("EVE-0010", SearchMode::Identifier));

    EXPECT_TRUE(result_set.bundles.front().implementation.empty());
    EXPECT_TRUE(result_set.bundles.front().validation.empty());
    ASSERT_EQ(result_set.bundles.front().supporting_references.size(), 1U);
    EXPECT_EQ(result_set.bundles.front().supporting_references.front().object_identifier, "EVE-9999");
}

TEST(CitationEngineTest, RemainsImmutableAfterCitationGeneration) {
    CitationFixture fixture({
        make_object(
            "EVE-0010",
            "EVE-0010",
            "docs/0000-core-platform-specifications/eve-0010-platform-request-specification.md",
            "Platform Request Specification",
            "0000",
            "Part I"),
    });

    const auto object_count = fixture.index.object_count();
    const auto graph_nodes = fixture.graph.node_count();
    const auto first = fixture.cite(make_query("EVE-0010", SearchMode::Identifier));
    const auto second = fixture.cite(make_query("EVE-0010", SearchMode::Identifier));

    EXPECT_EQ(fixture.index.object_count(), object_count);
    EXPECT_EQ(fixture.graph.node_count(), graph_nodes);
    EXPECT_EQ(first.bundles.front().primary.citation_confidence, second.bundles.front().primary.citation_confidence);
}

TEST(CitationEngineTest, SupportsConcurrentReadOnlyCitationGeneration) {
    CitationFixture fixture({
        make_object(
            "EVE-0010",
            "EVE-0010",
            "docs/0000-core-platform-specifications/eve-0010-platform-request-specification.md",
            "Platform Request Specification",
            "0000",
            "Part I"),
    });

    std::atomic<int> success_count{0};
    std::vector<std::thread> workers;
    workers.reserve(8);
    for (int worker = 0; worker < 8; ++worker) {
        workers.emplace_back([&fixture, &success_count]() {
            const auto result_set = fixture.cite(make_query("EVE-0010", SearchMode::Identifier));
            if (result_set.bundle_count == 1U &&
                result_set.bundles.front().primary.object_identifier == "EVE-0010") {
                success_count.fetch_add(1);
            }
        });
    }
    for (auto& worker : workers) {
        worker.join();
    }

    EXPECT_EQ(success_count.load(), 8);
}

}  // namespace
}  // namespace eve
