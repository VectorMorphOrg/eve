#include "eve/knowledge/knowledge_graph.hpp"
#include "eve/knowledge/relationship_builder.hpp"

#include <gtest/gtest.h>

namespace eve {
namespace {

KnowledgeObject make_object(
    std::string id,
    std::string document_identifier,
    std::string file_path,
    std::string series = {},
    std::string part = {},
    std::string repository = "eve",
    DocumentType type = DocumentType::Specification) {
    KnowledgeObject object;
    object.identity.id = KnowledgeObjectId{std::move(id)};
    object.identity.document_identifier = std::move(document_identifier);
    object.identity.document_type = type;
    object.identity.repository = std::move(repository);
    object.metadata.series = std::move(series);
    object.metadata.part = std::move(part);
    object.source.file_path = std::move(file_path);
    return object;
}

void set_content(KnowledgeObject& object, std::string text) {
    object.content.raw_markdown = text;
    object.search.search_text = text;
}


bool has_edge(
    const KnowledgeGraph& graph,
    std::string_view source_id,
    std::string_view target,
    KnowledgeGraphEdgeType type) {
    for (const auto& edge : graph.edges()) {
        if (edge.source_id.value == source_id && edge.target == target && edge.type == type) {
            return true;
        }
    }
    return false;
}

TEST(KnowledgeGraphTest, CreatesNodeForEveryKnowledgeObject) {
    auto first = make_object(
        "EVE-0011",
        "EVE-0011",
        "docs/0000-core-platform-specifications/eve-0011-platform-response-specification.md",
        "0000",
        "Part I");
    auto second = make_object(
        "EVE-0010",
        "EVE-0010",
        "docs/0000-core-platform-specifications/eve-0010-platform-request-specification.md",
        "0000",
        "Part I");
    auto isolated = make_object(
        "EVE-0600",
        "EVE-0600",
        "docs/0600-reference-catalogs/eve-0600-discord-command-catalog.md",
        "0600",
        "Part VII");

    const auto graph = KnowledgeGraphBuilder::build({first, second, isolated});

    EXPECT_EQ(graph.node_count(), 3U);
    EXPECT_EQ(graph.nodes().front().id.value, "EVE-0010");
    EXPECT_EQ(graph.nodes().back().id.value, "EVE-0600");
    EXPECT_NE(graph.find_node_by_id(KnowledgeObjectId{"EVE-0010"}), nullptr);
    EXPECT_NE(graph.find_node_by_id(KnowledgeObjectId{"EVE-0600"}), nullptr);
}

TEST(KnowledgeGraphTest, CreatesDirectedEdgesFromRelationshipFields) {
    auto parent = make_object(
        "README-0000",
        "README-0000",
        "docs/0000-core-platform-specifications/README.md",
        "0000",
        "Part I",
        "eve",
        DocumentType::Readme);
    auto object = make_object(
        "EVE-0012",
        "EVE-0012",
        "docs/0000-core-platform-specifications/eve-0012-validation-specification.md",
        "0000",
        "Part I");
    object.relationships.previous = "EVE-0011";
    object.relationships.next = "EVE-0013";
    object.relationships.parent = "README-0000";
    object.relationships.children = {"EVE-0013"};
    object.relationships.references = {"EVE-0010", "CAP-0001"};
    object.relationships.referenced_by = {"EVE-0011"};
    object.relationships.same_series = {"EVE-0011"};
    object.relationships.same_part = {"EVE-0011"};
    object.relationships.same_repository = {"EVE-0011"};
    object.relationships.related_headers = {"include/eve/validation/validation_engine.hpp"};
    object.relationships.related_sources = {"src/eve/validation/validation_engine.cpp"};
    object.relationships.related_tests = {"tests/validation_engine_test.cpp"};

    const auto graph = KnowledgeGraphBuilder::build({parent, object});

    EXPECT_TRUE(has_edge(graph, "EVE-0012", "EVE-0011", KnowledgeGraphEdgeType::Previous));
    EXPECT_TRUE(has_edge(graph, "EVE-0012", "EVE-0013", KnowledgeGraphEdgeType::Next));
    EXPECT_TRUE(has_edge(graph, "EVE-0012", "README-0000", KnowledgeGraphEdgeType::Parent));
    EXPECT_TRUE(has_edge(graph, "EVE-0012", "EVE-0013", KnowledgeGraphEdgeType::Children));
    EXPECT_TRUE(has_edge(graph, "EVE-0012", "EVE-0010", KnowledgeGraphEdgeType::References));
    EXPECT_TRUE(has_edge(graph, "EVE-0012", "CAP-0001", KnowledgeGraphEdgeType::References));
    EXPECT_TRUE(has_edge(graph, "EVE-0012", "EVE-0011", KnowledgeGraphEdgeType::ReferencedBy));
    EXPECT_TRUE(has_edge(graph, "EVE-0012", "EVE-0011", KnowledgeGraphEdgeType::SameSeries));
    EXPECT_TRUE(has_edge(
        graph,
        "EVE-0012",
        "include/eve/validation/validation_engine.hpp",
        KnowledgeGraphEdgeType::RelatedHeader));
    EXPECT_TRUE(has_edge(
        graph,
        "EVE-0012",
        "src/eve/validation/validation_engine.cpp",
        KnowledgeGraphEdgeType::RelatedSource));
    EXPECT_TRUE(has_edge(
        graph,
        "EVE-0012",
        "tests/validation_engine_test.cpp",
        KnowledgeGraphEdgeType::RelatedTest));
}

TEST(KnowledgeGraphTest, PreservesReciprocalReferenceEdges) {
    auto first = make_object(
        "EVE-0010",
        "EVE-0010",
        "docs/0000-core-platform-specifications/eve-0010-platform-request-specification.md",
        "0000",
        "Part I");
    set_content(first, "Depends on EVE-0011.");

    auto second = make_object(
        "EVE-0011",
        "EVE-0011",
        "docs/0000-core-platform-specifications/eve-0011-platform-response-specification.md",
        "0000",
        "Part I");
    set_content(second, "References EVE-0010.");

    const auto objects = RelationshipBuilder::build({first, second});
    const auto graph = KnowledgeGraphBuilder::build(objects);

    EXPECT_TRUE(has_edge(graph, "EVE-0010", "EVE-0011", KnowledgeGraphEdgeType::References));
    EXPECT_TRUE(has_edge(graph, "EVE-0011", "EVE-0010", KnowledgeGraphEdgeType::References));
    EXPECT_TRUE(has_edge(graph, "EVE-0010", "EVE-0011", KnowledgeGraphEdgeType::ReferencedBy));
    EXPECT_TRUE(has_edge(graph, "EVE-0011", "EVE-0010", KnowledgeGraphEdgeType::ReferencedBy));
}

TEST(KnowledgeGraphTest, HandlesCyclicGraphsDeterministically) {
    auto first = make_object(
        "EVE-0010",
        "EVE-0010",
        "docs/0000-core-platform-specifications/eve-0010-platform-request-specification.md",
        "0000",
        "Part I");
    set_content(first, "See EVE-0011.");

    auto second = make_object(
        "EVE-0011",
        "EVE-0011",
        "docs/0000-core-platform-specifications/eve-0011-platform-response-specification.md",
        "0000",
        "Part I");
    set_content(second, "See EVE-0010.");

    const auto objects = RelationshipBuilder::build({first, second});
    const auto first_graph = KnowledgeGraphBuilder::build(objects);
    const auto second_graph = KnowledgeGraphBuilder::build(objects);

    EXPECT_EQ(first_graph.edge_count(), second_graph.edge_count());
    EXPECT_EQ(first_graph.edges(), second_graph.edges());
    EXPECT_TRUE(has_edge(first_graph, "EVE-0010", "EVE-0011", KnowledgeGraphEdgeType::References));
    EXPECT_TRUE(has_edge(first_graph, "EVE-0011", "EVE-0010", KnowledgeGraphEdgeType::References));
    EXPECT_TRUE(has_edge(first_graph, "EVE-0010", "EVE-0011", KnowledgeGraphEdgeType::ReferencedBy));
    EXPECT_TRUE(has_edge(first_graph, "EVE-0011", "EVE-0010", KnowledgeGraphEdgeType::ReferencedBy));
}

TEST(KnowledgeGraphTest, SupportsDisconnectedGraphComponents) {
    auto connected = make_object(
        "EVE-0010",
        "EVE-0010",
        "docs/0000-core-platform-specifications/eve-0010-platform-request-specification.md",
        "0000",
        "Part I");
    connected.relationships.references = {"EVE-0011"};

    auto peer = make_object(
        "EVE-0011",
        "EVE-0011",
        "docs/0000-core-platform-specifications/eve-0011-platform-response-specification.md",
        "0000",
        "Part I");

    auto isolated = make_object(
        "EVE-0600",
        "EVE-0600",
        "docs/0600-reference-catalogs/eve-0600-discord-command-catalog.md",
        "0600",
        "Part VII");

    const auto graph = KnowledgeGraphBuilder::build({connected, peer, isolated});

    EXPECT_EQ(graph.node_count(), 3U);
    EXPECT_FALSE(has_edge(graph, "EVE-0600", "EVE-0010", KnowledgeGraphEdgeType::References));
    EXPECT_FALSE(has_edge(graph, "EVE-0600", "EVE-0011", KnowledgeGraphEdgeType::References));
    EXPECT_FALSE(has_edge(graph, "EVE-0010", "EVE-0600", KnowledgeGraphEdgeType::References));
    EXPECT_NE(graph.find_node_by_id(KnowledgeObjectId{"EVE-0600"}), nullptr);
}

TEST(KnowledgeGraphTest, LooksUpObjectsByIdentifier) {
    auto object = make_object(
        "EVE-0010",
        "EVE-0010",
        "docs/0000-core-platform-specifications/eve-0010-platform-request-specification.md",
        "0000",
        "Part I");

    const auto graph = KnowledgeGraphBuilder::build({object});

    const auto* by_id = graph.find_object_by_id(KnowledgeObjectId{"EVE-0010"});
    const auto* by_identifier = graph.find_object_by_identifier("eve-0010");
    const auto* missing = graph.find_object_by_identifier("EVE-9999");

    ASSERT_NE(by_id, nullptr);
    ASSERT_NE(by_identifier, nullptr);
    EXPECT_EQ(by_id, by_identifier);
    EXPECT_EQ(by_id->identity.document_identifier, "EVE-0010");
    EXPECT_EQ(missing, nullptr);
}

TEST(KnowledgeGraphTest, DoesNotMutateInputObjects) {
    auto object = make_object(
        "EVE-0010",
        "EVE-0010",
        "docs/0000-core-platform-specifications/eve-0010-platform-request-specification.md",
        "0000",
        "Part I");
    set_content(object, "References EVE-0011.");
    const auto id_before = object.identity.id.value;
    const auto references_before = object.relationships.references;

    const auto with_relationships = RelationshipBuilder::build({object});
    const auto edge_count_before = with_relationships.front().relationships.references.size();
    const auto graph = KnowledgeGraphBuilder::build(with_relationships);

    EXPECT_EQ(object.identity.id.value, id_before);
    EXPECT_TRUE(object.relationships.references.empty());
    EXPECT_EQ(references_before, object.relationships.references);
    EXPECT_EQ(with_relationships.front().relationships.references.size(), edge_count_before);
    EXPECT_EQ(graph.objects().front().identity.id.value, "EVE-0010");
}

}  // namespace
}  // namespace eve
