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

TEST(RelationshipBuilderTest, DiscoversReferenceIdentifiers) {
    const auto identifiers = RelationshipBuilder::discover_reference_identifiers(
        "See EVE-0001 and CAP-0102 plus SVC-0005 and DOC-0609.");
    ASSERT_EQ(identifiers.size(), 4U);
    EXPECT_EQ(identifiers[0], "CAP-0102");
    EXPECT_EQ(identifiers[1], "DOC-0609");
    EXPECT_EQ(identifiers[2], "EVE-0001");
    EXPECT_EQ(identifiers[3], "SVC-0005");
}

TEST(RelationshipBuilderTest, BuildsReciprocalReferences) {
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

    auto objects = RelationshipBuilder::build({first, second});

    EXPECT_EQ(objects[0].relationships.references, std::vector<std::string>({"EVE-0011"}));
    EXPECT_EQ(objects[1].relationships.references, std::vector<std::string>({"EVE-0010"}));
    EXPECT_EQ(objects[0].relationships.referenced_by, std::vector<std::string>({"EVE-0011"}));
    EXPECT_EQ(objects[1].relationships.referenced_by, std::vector<std::string>({"EVE-0010"}));
}

TEST(RelationshipBuilderTest, PopulatesNavigationRelationships) {
    auto readme = make_object(
        "README-0000",
        "README-0000",
        "docs/0000-core-platform-specifications/README.md",
        "0000",
        "Part I",
        "eve",
        DocumentType::Readme);

    auto first = make_object(
        "EVE-0010",
        "EVE-0010",
        "docs/0000-core-platform-specifications/eve-0010-platform-request-specification.md",
        "0000",
        "Part I");
    first.relationships.previous = "EVE-0009";
    first.relationships.next = "EVE-0011 — Platform Response Specification";

    auto second = make_object(
        "EVE-0011",
        "EVE-0011",
        "docs/0000-core-platform-specifications/eve-0011-platform-response-specification.md",
        "0000",
        "Part I");

    auto objects = RelationshipBuilder::build({readme, first, second});

    EXPECT_EQ(objects[1].relationships.parent, "README-0000");
    EXPECT_EQ(objects[0].relationships.children, std::vector<std::string>({"EVE-0010", "EVE-0011"}));
    EXPECT_EQ(objects[1].relationships.next, "EVE-0011");
}

TEST(RelationshipBuilderTest, PopulatesRepositoryRelationships) {
    auto first = make_object(
        "EVE-0010",
        "EVE-0010",
        "docs/0000-core-platform-specifications/eve-0010-platform-request-specification.md",
        "0000",
        "Part I");
    auto second = make_object(
        "EVE-0011",
        "EVE-0011",
        "docs/0000-core-platform-specifications/eve-0011-platform-response-specification.md",
        "0000",
        "Part I");
    auto other = make_object(
        "EVE-0600",
        "EVE-0600",
        "docs/0600-reference-catalogs/eve-0600-discord-command-catalog.md",
        "0600",
        "Part VII");

    auto objects = RelationshipBuilder::build({first, second, other});

    EXPECT_EQ(objects[0].relationships.same_series, std::vector<std::string>({"EVE-0011"}));
    EXPECT_EQ(objects[0].relationships.same_part, std::vector<std::string>({"EVE-0011"}));
    EXPECT_EQ(objects[0].relationships.same_repository, std::vector<std::string>({"EVE-0011", "EVE-0600"}));
    EXPECT_TRUE(objects[2].relationships.same_series.empty());
}

TEST(RelationshipBuilderTest, DiscoversImplementationRelationships) {
    auto spec = make_object(
        "EVE-0012",
        "EVE-0012",
        "docs/0000-core-platform-specifications/eve-0012-validation-specification.md",
        "0000",
        "Part I");

    KnowledgeObject header;
    header.identity.id = KnowledgeObjectId{"include-validation-engine-hpp"};
    header.source.file_path = "include/eve/validation/validation_engine.hpp";

    KnowledgeObject source;
    source.identity.id = KnowledgeObjectId{"src-validation-engine-cpp"};
    source.source.file_path = "src/eve/validation/validation_engine.cpp";

    KnowledgeObject test;
    test.identity.id = KnowledgeObjectId{"tests-validation-engine-test-cpp"};
    test.source.file_path = "tests/validation_engine_test.cpp";

    auto objects = RelationshipBuilder::build({spec, header, source, test});

    EXPECT_EQ(
        objects[0].relationships.related_headers,
        std::vector<std::string>({"include/eve/validation/validation_engine.hpp"}));
    EXPECT_EQ(
        objects[0].relationships.related_sources,
        std::vector<std::string>({"src/eve/validation/validation_engine.cpp"}));
    EXPECT_EQ(
        objects[0].relationships.related_tests,
        std::vector<std::string>({"tests/validation_engine_test.cpp"}));
}

TEST(RelationshipBuilderTest, KeepsMissingReferencesWithoutReciprocalTarget) {
    auto object = make_object(
        "EVE-0010",
        "EVE-0010",
        "docs/0000-core-platform-specifications/eve-0010-platform-request-specification.md",
        "0000",
        "Part I");
    set_content(object, "Future work references EVE-9999.");

    auto objects = RelationshipBuilder::build({object});

    ASSERT_EQ(objects.front().relationships.references.size(), 1U);
    EXPECT_EQ(objects.front().relationships.references.front(), "EVE-9999");
    EXPECT_TRUE(objects.front().relationships.referenced_by.empty());
}

TEST(RelationshipBuilderTest, HandlesCyclicReferencesDeterministically) {
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

    const auto first_pass = RelationshipBuilder::build({first, second});
    const auto second_pass = RelationshipBuilder::build({first, second});

    EXPECT_EQ(first_pass[0].relationships.references, second_pass[0].relationships.references);
    EXPECT_EQ(first_pass[0].relationships.referenced_by, second_pass[0].relationships.referenced_by);
    EXPECT_EQ(first_pass[1].relationships.referenced_by, std::vector<std::string>({"EVE-0010"}));
}

TEST(RelationshipBuilderTest, DoesNotRequireDocumentModelOrMetadataInputs) {
    auto object = make_object(
        "EVE-0010",
        "EVE-0010",
        "docs/0000-core-platform-specifications/eve-0010-platform-request-specification.md",
        "0000",
        "Part I");
    set_content(object, "References CAP-0001.");

    const auto before = object;
    auto objects = RelationshipBuilder::build({object});

    EXPECT_EQ(before.identity.id.value, objects.front().identity.id.value);
    EXPECT_EQ(before.metadata.series, objects.front().metadata.series);
    EXPECT_FALSE(objects.front().relationships.references.empty());
}

}  // namespace
}  // namespace eve
