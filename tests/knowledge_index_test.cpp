#include "eve/knowledge/knowledge_graph.hpp"
#include "eve/knowledge/knowledge_index.hpp"
#include "eve/knowledge/relationship_builder.hpp"

#include <gtest/gtest.h>

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

KnowledgeIndex build_index(std::vector<KnowledgeObject> objects) {
    const auto with_relationships = RelationshipBuilder::build(std::move(objects));
    const auto graph = KnowledgeGraphBuilder::build(with_relationships);
    return KnowledgeIndexBuilder::build(graph);
}

std::vector<std::string> object_ids(const std::vector<const KnowledgeObject*>& objects) {
    std::vector<std::string> ids;
    ids.reserve(objects.size());
    for (const auto* object : objects) {
        ids.push_back(object->identity.id.value);
    }
    return ids;
}

TEST(KnowledgeIndexTest, LooksUpByObjectIdentifier) {
    auto first = make_object(
        "EVE-0010",
        "EVE-0010",
        "docs/0000-core-platform-specifications/eve-0010-platform-request-specification.md",
        "Platform Request Specification",
        "0000",
        "Part I");
    auto second = make_object(
        "EVE-0011",
        "EVE-0011",
        "docs/0000-core-platform-specifications/eve-0011-platform-response-specification.md",
        "Platform Response Specification",
        "0000",
        "Part I");

    const auto index = build_index({first, second});

    const auto by_id = index.lookup_by_object_id("EVE-0010");
    const auto by_identifier = index.lookup_by_document_identifier("eve-0010");

    ASSERT_EQ(by_id.size(), 1U);
    ASSERT_EQ(by_identifier.size(), 1U);
    EXPECT_EQ(by_id.front()->identity.title, "Platform Request Specification");
    EXPECT_EQ(by_id.front(), by_identifier.front());
}

TEST(KnowledgeIndexTest, LooksUpByTitleKeywordAndTag) {
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

    const auto index = build_index({first, second});

    EXPECT_EQ(index.lookup_by_title("platform request specification").size(), 1U);
    EXPECT_EQ(object_ids(index.lookup_by_keyword("platform")), (std::vector<std::string>{"EVE-0010", "EVE-0011"}));
    EXPECT_EQ(
        object_ids(index.lookup_by_tag("specification")),
        (std::vector<std::string>{"EVE-0010", "EVE-0011"}));
}

TEST(KnowledgeIndexTest, LooksUpByRepositorySeriesAndPart) {
    auto first = make_object(
        "EVE-0010",
        "EVE-0010",
        "docs/0000-core-platform-specifications/eve-0010-platform-request-specification.md",
        "Platform Request Specification",
        "0000",
        "Part I");
    auto second = make_object(
        "EVE-0011",
        "EVE-0011",
        "docs/0000-core-platform-specifications/eve-0011-platform-response-specification.md",
        "Platform Response Specification",
        "0000",
        "Part I");
    auto other = make_object(
        "EVE-0600",
        "EVE-0600",
        "docs/0600-reference-catalogs/eve-0600-discord-command-catalog.md",
        "Discord Command Catalog",
        "0600",
        "Part VII");

    const auto index = build_index({first, second, other});

    EXPECT_EQ(object_ids(index.lookup_by_repository("eve")), (std::vector<std::string>{"EVE-0010", "EVE-0011", "EVE-0600"}));
    EXPECT_EQ(object_ids(index.lookup_by_series("0000")), (std::vector<std::string>{"EVE-0010", "EVE-0011"}));
    EXPECT_EQ(object_ids(index.lookup_by_part("Part I")), (std::vector<std::string>{"EVE-0010", "EVE-0011"}));
    EXPECT_EQ(index.lookup_by_document_type(DocumentType::Specification).size(), 3U);
}

TEST(KnowledgeIndexTest, LooksUpByRelationshipTarget) {
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
    second.content.raw_markdown = "References EVE-0010.";
    second.search.search_text = second.content.raw_markdown;

    const auto index = build_index({first, second});

    EXPECT_EQ(
        object_ids(index.lookup_by_relationship(KnowledgeGraphEdgeType::References, "EVE-0011")),
        (std::vector<std::string>{"EVE-0010"}));
    EXPECT_EQ(
        object_ids(index.lookup_by_relationship(KnowledgeGraphEdgeType::References, "EVE-0010")),
        (std::vector<std::string>{"EVE-0011"}));
    EXPECT_EQ(
        object_ids(index.lookup_by_relationship(KnowledgeGraphEdgeType::ReferencedBy, "EVE-0010")),
        (std::vector<std::string>{"EVE-0011"}));
}

TEST(KnowledgeIndexTest, HandlesDuplicateKeysWithStableOrdering) {
    auto first = make_object(
        "EVE-0010",
        "EVE-0010",
        "docs/0000-core-platform-specifications/eve-0010-platform-request-specification.md",
        "Shared Title",
        "0000",
        "Part I");
    auto second = make_object(
        "EVE-0011",
        "EVE-0011",
        "docs/0000-core-platform-specifications/eve-0011-platform-response-specification.md",
        "Shared Title",
        "0000",
        "Part I");

    const auto first_index = build_index({first, second});
    const auto second_index = build_index({second, first});

    const auto first_lookup = object_ids(first_index.lookup_by_title("Shared Title"));
    const auto second_lookup = object_ids(second_index.lookup_by_title("Shared Title"));

    EXPECT_EQ(first_lookup, (std::vector<std::string>{"EVE-0010", "EVE-0011"}));
    EXPECT_EQ(first_lookup, second_lookup);
}

TEST(KnowledgeIndexTest, ReturnsEmptyResultsForMissingKeys) {
    auto object = make_object(
        "EVE-0010",
        "EVE-0010",
        "docs/0000-core-platform-specifications/eve-0010-platform-request-specification.md",
        "Platform Request Specification",
        "0000",
        "Part I");

    const auto index = build_index({object});

    EXPECT_TRUE(index.lookup_by_object_id("EVE-9999").empty());
    EXPECT_TRUE(index.lookup_by_title("Missing Title").empty());
    EXPECT_TRUE(index.lookup_by_keyword("missing").empty());
    EXPECT_TRUE(index.lookup_by_tag("missing").empty());
    EXPECT_TRUE(index.lookup_by_series("9999").empty());
    EXPECT_TRUE(
        index.lookup_by_relationship(KnowledgeGraphEdgeType::References, "EVE-9999").empty());
}

TEST(KnowledgeIndexTest, PerformsCaseInsensitiveLookup) {
    auto object = make_object(
        "EVE-0010",
        "EVE-0010",
        "docs/0000-core-platform-specifications/eve-0010-platform-request-specification.md",
        "Platform Request Specification",
        "0000",
        "Part I");
    object.search.keywords = {"Validation"};
    object.metadata.tags = {"Core"};
    object.content.headings.push_back(DocumentHeading{.level = 1, .text = "Overview Section"});
    object.content.sections.push_back(KnowledgeSection{.title = "Requirements Section"});
    object.search.search_text = "Validation platform request";

    const auto index = build_index({object});

    EXPECT_EQ(index.lookup_by_document_identifier("eve-0010").size(), 1U);
    EXPECT_EQ(index.lookup_by_title("PLATFORM REQUEST SPECIFICATION").size(), 1U);
    EXPECT_EQ(index.lookup_by_keyword("validation").size(), 1U);
    EXPECT_EQ(index.lookup_by_tag("core").size(), 1U);
    EXPECT_EQ(index.lookup_by_search_text_token("VALIDATION").size(), 1U);
    EXPECT_EQ(index.lookup_by_heading("overview section").size(), 1U);
    EXPECT_EQ(index.lookup_by_section_title("requirements section").size(), 1U);
}

TEST(KnowledgeIndexTest, RemainsImmutableAfterConstruction) {
    auto object = make_object(
        "EVE-0010",
        "EVE-0010",
        "docs/0000-core-platform-specifications/eve-0010-platform-request-specification.md",
        "Platform Request Specification",
        "0000",
        "Part I");
    object.search.keywords = {"platform"};

    const auto graph = KnowledgeGraphBuilder::build(RelationshipBuilder::build({object}));
    const auto index = KnowledgeIndexBuilder::build(graph);

    const auto first_lookup = object_ids(index.lookup_by_keyword("platform"));
    const auto object_count = index.object_count();
    const auto graph_nodes = index.graph().node_count();

    EXPECT_EQ(first_lookup, (std::vector<std::string>{"EVE-0010"}));
    EXPECT_EQ(object_count, 1U);
    EXPECT_EQ(graph_nodes, 1U);
    EXPECT_EQ(object_ids(index.lookup_by_keyword("platform")), first_lookup);
}

}  // namespace
}  // namespace eve
