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

struct RankFixture {
    KnowledgeIndex index;
    SearchEngine search_engine;
    RankingEngine ranking_engine;

    explicit RankFixture(std::vector<KnowledgeObject> objects)
        : index(KnowledgeIndexBuilder::build(
              KnowledgeGraphBuilder::build(RelationshipBuilder::build(std::move(objects))))),
          search_engine(index),
          ranking_engine(index) {}

    RankedSearchResultSet rank(SearchQuery query) const {
        return ranking_engine.rank(search_engine.search(query));
    }
};

std::vector<std::string> ranked_ids(const RankedSearchResultSet& result_set) {
    std::vector<std::string> ids;
    ids.reserve(result_set.results.size());
    for (const auto& result : result_set.results) {
        ids.push_back(result.candidate.object_identifier);
    }
    return ids;
}

SearchQuery make_query(std::string text, SearchMode mode = SearchMode::Auto) {
    SearchQuery query;
    query.text = std::move(text);
    query.mode = mode;
    return query;
}

TEST(RankingEngineTest, RanksExactIdentifierAboveKeywordMatch) {
    auto spec = make_object(
        "EVE-0010",
        "EVE-0010",
        "docs/0000-core-platform-specifications/eve-0010-platform-request-specification.md",
        "Platform Request Specification",
        "0000",
        "Part I");
    spec.search.keywords = {"EVE-0010"};

    auto other = make_object(
        "EVE-0011",
        "EVE-0011",
        "docs/0000-core-platform-specifications/eve-0011-platform-response-specification.md",
        "Platform Response Specification",
        "0000",
        "Part I");
    other.search.keywords = {"EVE-0010"};

    RankFixture fixture({spec, other});

    const auto ranked = fixture.rank(make_query("EVE-0010", SearchMode::Auto));

    ASSERT_EQ(ranked.results.size(), 2U);
    EXPECT_EQ(ranked.results.front().candidate.object_identifier, "EVE-0010");
    EXPECT_GT(ranked.results.front().rank_score, ranked.results.back().rank_score);
}

TEST(RankingEngineTest, RanksSpecificationAboveOtherDocumentTypes) {
    auto specification = make_object(
        "EVE-0010",
        "EVE-0010",
        "docs/0000-core-platform-specifications/eve-0010-platform-request-specification.md",
        "Shared Topic",
        "0000",
        "Part I",
        "eve",
        DocumentType::Specification);
    specification.search.keywords = {"shared-topic"};

    auto other = make_object(
        "DOC-0001",
        "DOC-0001",
        "docs/misc/other.md",
        "Shared Topic",
        "9999",
        "Part IX",
        "eve",
        DocumentType::Other);
    other.search.keywords = {"shared-topic"};

    RankFixture fixture({specification, other});

    const auto ranked = fixture.rank(make_query("shared-topic", SearchMode::Keyword));

    ASSERT_EQ(ranked.results.size(), 2U);
    EXPECT_EQ(ranked.results.front().candidate.object_identifier, "EVE-0010");
    EXPECT_EQ(ranked.results.front().candidate.document_type, DocumentType::Specification);
}

TEST(RankingEngineTest, RanksExactTitleAbovePartialFullTextMatch) {
    auto exact = make_object(
        "EVE-0010",
        "EVE-0010",
        "docs/0000-core-platform-specifications/eve-0010-platform-request-specification.md",
        "Validation Specification",
        "0000",
        "Part I");
    exact.search.search_text = "validation specification";

    auto partial = make_object(
        "EVE-0011",
        "EVE-0011",
        "docs/0000-core-platform-specifications/eve-0011-platform-response-specification.md",
        "Platform Response Specification",
        "0000",
        "Part I");
    partial.search.search_text = "validation platform response";

    RankFixture fixture({exact, partial});

    const auto ranked = fixture.rank(make_query("Validation Specification", SearchMode::Auto));

    ASSERT_GE(ranked.results.size(), 1U);
    EXPECT_EQ(ranked.results.front().candidate.object_identifier, "EVE-0010");
}

TEST(RankingEngineTest, AssignsDeterministicRankPositions) {
    RankFixture fixture({
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

    const auto first = fixture.rank(make_query("0000", SearchMode::Series));
    const auto second = fixture.rank(make_query("0000", SearchMode::Series));

    EXPECT_EQ(ranked_ids(first), ranked_ids(second));
    ASSERT_EQ(first.results.size(), 2U);
    EXPECT_EQ(first.results[0].rank_position, 1U);
    EXPECT_EQ(first.results[1].rank_position, 2U);
}

TEST(RankingEngineTest, ReturnsEmptyRankedSetForEmptyCandidates) {
    RankFixture fixture({
        make_object(
            "EVE-0010",
            "EVE-0010",
            "docs/0000-core-platform-specifications/eve-0010-platform-request-specification.md",
            "Platform Request Specification",
            "0000",
            "Part I"),
    });

    const auto ranked = fixture.rank(make_query(""));

    EXPECT_TRUE(ranked.results.empty());
    EXPECT_EQ(ranked.result_count, 0U);
}

TEST(RankingEngineTest, UsesObjectIdentifierTieBreakerForEqualScores) {
    auto second = make_object(
        "EVE-0011",
        "EVE-0011",
        "docs/0000-core-platform-specifications/eve-0011-platform-response-specification.md",
        "Shared Title",
        "0000",
        "Part I");
    auto first = make_object(
        "EVE-0010",
        "EVE-0010",
        "docs/0000-core-platform-specifications/eve-0010-platform-request-specification.md",
        "Shared Title",
        "0000",
        "Part I");

    RankFixture fixture({second, first});

    const auto ranked = fixture.rank(make_query("Shared Title", SearchMode::Title));

    ASSERT_EQ(ranked.results.size(), 2U);
    EXPECT_EQ(ranked.results.front().candidate.object_identifier, "EVE-0010");
    EXPECT_EQ(ranked.results.back().candidate.object_identifier, "EVE-0011");
}

TEST(RankingEngineTest, BoostsRelationshipMatches) {
    auto source = make_object(
        "EVE-0010",
        "EVE-0010",
        "docs/0000-core-platform-specifications/eve-0010-platform-request-specification.md",
        "Platform Request Specification",
        "0000",
        "Part I");
    source.content.raw_markdown = "Depends on EVE-0011.";
    source.search.search_text = source.content.raw_markdown;

    auto target = make_object(
        "EVE-0011",
        "EVE-0011",
        "docs/0000-core-platform-specifications/eve-0011-platform-response-specification.md",
        "Platform Response Specification",
        "0000",
        "Part I",
        "eve",
        DocumentType::Other);
    target.search.priority_rank = priority_rank_for_document_type(DocumentType::Other);

    RankFixture fixture({source, target});

    SearchQuery query = make_query("EVE-0011", SearchMode::Relationship);
    query.relationship_type = KnowledgeGraphEdgeType::References;
    const auto ranked = fixture.ranking_engine.rank(fixture.search_engine.search(query));

    ASSERT_EQ(ranked.results.size(), 1U);
    EXPECT_EQ(ranked.results.front().candidate.match_type, SearchMatchType::RelationshipTarget);
    EXPECT_GE(ranked.results.front().rank_score, 650);
}

TEST(RankingEngineTest, RemainsImmutableAfterRanking) {
    RankFixture fixture({
        make_object(
            "EVE-0010",
            "EVE-0010",
            "docs/0000-core-platform-specifications/eve-0010-platform-request-specification.md",
            "Platform Request Specification",
            "0000",
            "Part I"),
    });

    const auto object_count = fixture.index.object_count();
    const auto first = fixture.rank(make_query("EVE-0010", SearchMode::Identifier));
    const auto second = fixture.rank(make_query("EVE-0010", SearchMode::Identifier));

    EXPECT_EQ(fixture.index.object_count(), object_count);
    EXPECT_EQ(first.results.front().rank_score, second.results.front().rank_score);
}

TEST(RankingEngineTest, SupportsConcurrentReadOnlyRanking) {
    RankFixture fixture({
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
            const auto ranked = fixture.rank(make_query("EVE-0010", SearchMode::Identifier));
            if (ranked.results.size() == 1U && ranked.results.front().rank_position == 1U) {
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
