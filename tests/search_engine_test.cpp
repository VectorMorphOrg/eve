#include "eve/knowledge/knowledge_graph.hpp"
#include "eve/knowledge/knowledge_index.hpp"
#include "eve/knowledge/relationship_builder.hpp"
#include "eve/knowledge/search_engine.hpp"

#include <gtest/gtest.h>

#include <atomic>
#include <string>
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
    return object;
}

struct SearchFixture {
    KnowledgeIndex index;
    SearchEngine engine;

    explicit SearchFixture(std::vector<KnowledgeObject> objects)
        : index(KnowledgeIndexBuilder::build(
              KnowledgeGraphBuilder::build(RelationshipBuilder::build(std::move(objects))))),
          engine(index) {}
};

std::vector<std::string> result_ids(const SearchEngineResultSet& result_set) {
    std::vector<std::string> ids;
    ids.reserve(result_set.results.size());
    for (const auto& result : result_set.results) {
        ids.push_back(result.object_identifier);
    }
    return ids;
}

SearchQuery make_query(std::string text, SearchMode mode = SearchMode::Auto) {
    SearchQuery query;
    query.text = std::move(text);
    query.mode = mode;
    return query;
}

TEST(SearchEngineTest, LooksUpByObjectIdentifier) {
    SearchFixture fixture({
        make_object(
            "EVE-0010",
            "EVE-0010",
            "docs/0000-core-platform-specifications/eve-0010-platform-request-specification.md",
            "Platform Request Specification",
            "0000",
            "Part I"),
    });

    const auto result_set = fixture.engine.search(make_query("EVE-0010", SearchMode::Identifier));

    ASSERT_EQ(result_set.results.size(), 1U);
    EXPECT_EQ(result_set.results.front().object_identifier, "EVE-0010");
    EXPECT_EQ(result_set.results.front().match_type, SearchMatchType::ObjectId);
}

TEST(SearchEngineTest, LooksUpByDocumentIdentifier) {
    SearchFixture fixture({
        make_object(
            "EVE-0010",
            "EVE-0010",
            "docs/0000-core-platform-specifications/eve-0010-platform-request-specification.md",
            "Platform Request Specification",
            "0000",
            "Part I"),
    });

    SearchQuery query = make_query("eve-0010", SearchMode::Identifier);
    const auto result_set = fixture.engine.search(query);

    ASSERT_EQ(result_set.results.size(), 1U);
    EXPECT_EQ(result_set.results.front().document_identifier, "EVE-0010");
}

TEST(SearchEngineTest, LooksUpByTitleKeywordAndTag) {
    auto first = make_object(
        "EVE-0010",
        "EVE-0010",
        "docs/0000-core-platform-specifications/eve-0010-platform-request-specification.md",
        "Platform Request Specification",
        "0000",
        "Part I");
    first.search.keywords = {"platform", "request"};
    first.metadata.tags = {"core", "specification"};

    auto second = make_object(
        "EVE-0011",
        "EVE-0011",
        "docs/0000-core-platform-specifications/eve-0011-platform-response-specification.md",
        "Platform Response Specification",
        "0000",
        "Part I");
    second.search.keywords = {"platform", "response"};
    second.metadata.tags = {"core", "specification"};

    SearchFixture fixture({first, second});

    EXPECT_EQ(
        result_ids(fixture.engine.search(make_query("Platform Request Specification", SearchMode::Title))),
        (std::vector<std::string>{"EVE-0010"}));
    EXPECT_EQ(
        result_ids(fixture.engine.search(make_query("platform", SearchMode::Keyword))),
        (std::vector<std::string>{"EVE-0010", "EVE-0011"}));
    EXPECT_EQ(
        result_ids(fixture.engine.search(make_query("specification", SearchMode::Tag))),
        (std::vector<std::string>{"EVE-0010", "EVE-0011"}));
}

TEST(SearchEngineTest, LooksUpByRepositorySeriesPartAndStatus) {
    SearchFixture fixture({
        make_object(
            "EVE-0010",
            "EVE-0010",
            "docs/0000-core-platform-specifications/eve-0010-platform-request-specification.md",
            "Platform Request Specification",
            "0000",
            "Part I"),
        make_object(
            "EVE-0011",
            "EVE-0011",
            "docs/0000-core-platform-specifications/eve-0011-platform-response-specification.md",
            "Platform Response Specification",
            "0000",
            "Part I"),
    });

    EXPECT_EQ(result_ids(fixture.engine.search(make_query("eve", SearchMode::Repository))).size(), 2U);
    EXPECT_EQ(
        result_ids(fixture.engine.search(make_query("0000", SearchMode::Series))),
        (std::vector<std::string>{"EVE-0010", "EVE-0011"}));
    EXPECT_EQ(
        result_ids(fixture.engine.search(make_query("Part I", SearchMode::Part))),
        (std::vector<std::string>{"EVE-0010", "EVE-0011"}));
    EXPECT_EQ(
        result_ids(fixture.engine.search(make_query("Draft", SearchMode::Status))),
        (std::vector<std::string>{"EVE-0010", "EVE-0011"}));
}

TEST(SearchEngineTest, LooksUpByDocumentTypeHeadingAndSection) {
    auto object = make_object(
        "EVE-0010",
        "EVE-0010",
        "docs/0000-core-platform-specifications/eve-0010-platform-request-specification.md",
        "Platform Request Specification",
        "0000",
        "Part I");
    object.content.headings.push_back(DocumentHeading{.level = 1, .text = "Overview Section"});
    object.content.sections.push_back(KnowledgeSection{.title = "Requirements Section"});

    SearchFixture fixture({object});

    EXPECT_EQ(result_ids(fixture.engine.search(make_query("Specification", SearchMode::DocumentType))).size(), 1U);
    EXPECT_EQ(result_ids(fixture.engine.search(make_query("Overview Section", SearchMode::Heading))).size(), 1U);
    EXPECT_EQ(result_ids(fixture.engine.search(make_query("Requirements Section", SearchMode::Section))).size(), 1U);
}

TEST(SearchEngineTest, LooksUpByRelationshipTargetAndFullText) {
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
    second.content.raw_markdown = "References EVE-0010 and validation platform request.";
    second.search.search_text = second.content.raw_markdown;

    SearchFixture fixture({first, second});

    SearchQuery relationship_query = make_query("EVE-0010", SearchMode::Relationship);
    relationship_query.relationship_type = KnowledgeGraphEdgeType::References;
    EXPECT_EQ(result_ids(fixture.engine.search(relationship_query)), (std::vector<std::string>{"EVE-0011"}));

    EXPECT_EQ(result_ids(fixture.engine.search(make_query("validation", SearchMode::FullText))), (std::vector<std::string>{"EVE-0011"}));
}

TEST(SearchEngineTest, AutoModeDetectsIdentifiersAndMetadata) {
    auto first = make_object(
        "EVE-0010",
        "EVE-0010",
        "docs/0000-core-platform-specifications/eve-0010-platform-request-specification.md",
        "Platform Request Specification",
        "0000",
        "Part I");
    first.search.keywords = {"platform"};

    auto second = make_object(
        "EVE-0011",
        "EVE-0011",
        "docs/0000-core-platform-specifications/eve-0011-platform-response-specification.md",
        "Platform Response Specification",
        "0000",
        "Part I");

    SearchFixture fixture({first, second});

    const auto identifier_results = fixture.engine.search(make_query("EVE-0010"));
    ASSERT_EQ(identifier_results.results.size(), 1U);
    EXPECT_EQ(identifier_results.results.front().object_identifier, "EVE-0010");

    const auto title_results = fixture.engine.search(make_query("Platform Response Specification"));
    ASSERT_EQ(title_results.results.size(), 1U);
    EXPECT_EQ(title_results.results.front().object_identifier, "EVE-0011");
}

TEST(SearchEngineTest, SupportsExactAndPartialMatching) {
    auto object = make_object(
        "EVE-0010",
        "EVE-0010",
        "docs/0000-core-platform-specifications/eve-0010-platform-request-specification.md",
        "Platform Request Specification",
        "0000",
        "Part I");
    object.search.search_text = "validation platform request";

    SearchFixture fixture({object});

    SearchQuery exact_query = make_query("Platform Request Specification", SearchMode::Title);
    exact_query.exact_match = true;
    exact_query.partial_match = false;
    EXPECT_EQ(result_ids(fixture.engine.search(exact_query)).size(), 1U);

    SearchQuery partial_query = make_query("request", SearchMode::FullText);
    partial_query.exact_match = false;
    partial_query.partial_match = true;
    EXPECT_EQ(result_ids(fixture.engine.search(partial_query)), (std::vector<std::string>{"EVE-0010"}));
}

TEST(SearchEngineTest, EliminatesDuplicatesAndPreservesStableOrdering) {
    auto first = make_object(
        "EVE-0010",
        "EVE-0010",
        "docs/0000-core-platform-specifications/eve-0010-platform-request-specification.md",
        "Shared Title",
        "0000",
        "Part I");
    first.search.keywords = {"shared"};
    first.metadata.tags = {"shared"};

    auto second = make_object(
        "EVE-0011",
        "EVE-0011",
        "docs/0000-core-platform-specifications/eve-0011-platform-response-specification.md",
        "Shared Title",
        "0000",
        "Part I");
    second.search.keywords = {"shared"};
    second.metadata.tags = {"shared"};

    SearchFixture fixture({first, second});

    const auto first_pass = fixture.engine.search(make_query("shared", SearchMode::Auto));
    const auto second_pass = fixture.engine.search(make_query("shared", SearchMode::Auto));

    EXPECT_EQ(result_ids(first_pass), (std::vector<std::string>{"EVE-0010", "EVE-0011"}));
    EXPECT_EQ(result_ids(first_pass), result_ids(second_pass));
}

TEST(SearchEngineTest, ReturnsEmptyResultsForEmptyOrMissingQueries) {
    SearchFixture fixture({
        make_object(
            "EVE-0010",
            "EVE-0010",
            "docs/0000-core-platform-specifications/eve-0010-platform-request-specification.md",
            "Platform Request Specification",
            "0000",
            "Part I"),
    });

    EXPECT_TRUE(fixture.engine.search(make_query("")).results.empty());
    EXPECT_TRUE(fixture.engine.search(make_query("EVE-9999", SearchMode::Identifier)).results.empty());
}

TEST(SearchEngineTest, PerformsCaseInsensitiveLookup) {
    SearchFixture fixture({
        make_object(
            "EVE-0010",
            "EVE-0010",
            "docs/0000-core-platform-specifications/eve-0010-platform-request-specification.md",
            "Platform Request Specification",
            "0000",
            "Part I"),
    });

    SearchQuery query = make_query("platform request specification", SearchMode::Title);
    query.case_sensitive = false;
    EXPECT_EQ(result_ids(fixture.engine.search(query)).size(), 1U);
}

TEST(SearchEngineTest, AppliesMaximumResultLimit) {
    SearchFixture fixture({
        make_object(
            "EVE-0010",
            "EVE-0010",
            "docs/0000-core-platform-specifications/eve-0010-platform-request-specification.md",
            "Platform Request Specification",
            "0000",
            "Part I"),
        make_object(
            "EVE-0011",
            "EVE-0011",
            "docs/0000-core-platform-specifications/eve-0011-platform-response-specification.md",
            "Platform Response Specification",
            "0000",
            "Part I"),
    });

    SearchQuery query = make_query("0000", SearchMode::Series);
    query.max_results = 1;
    const auto result_set = fixture.engine.search(query);

    EXPECT_EQ(result_set.result_count, 1U);
    EXPECT_EQ(result_set.results.size(), 1U);
}

TEST(SearchEngineTest, SupportsConcurrentReadOnlySearch) {
    SearchFixture fixture({
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
            const auto result_set = fixture.engine.search(make_query("EVE-0010", SearchMode::Identifier));
            if (result_set.results.size() == 1U &&
                result_set.results.front().object_identifier == "EVE-0010") {
                success_count.fetch_add(1);
            }
        });
    }
    for (auto& worker : workers) {
        worker.join();
    }

    EXPECT_EQ(success_count.load(), 8);
}

TEST(SearchEngineTest, RemainsImmutableAfterConstruction) {
    auto object = make_object(
        "EVE-0010",
        "EVE-0010",
        "docs/0000-core-platform-specifications/eve-0010-platform-request-specification.md",
        "Platform Request Specification",
        "0000",
        "Part I");
    object.search.keywords = {"platform"};

    SearchFixture fixture({object});
    const auto before_count = fixture.index.object_count();

    const auto first = fixture.engine.search(make_query("platform", SearchMode::Keyword));
    const auto second = fixture.engine.search(make_query("platform", SearchMode::Keyword));

    EXPECT_EQ(before_count, fixture.index.object_count());
    EXPECT_EQ(result_ids(first), result_ids(second));
    EXPECT_EQ(&fixture.engine.index(), &fixture.index);
}

}  // namespace
}  // namespace eve
