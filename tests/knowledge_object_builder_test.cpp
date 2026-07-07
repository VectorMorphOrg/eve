#include "eve/knowledge/knowledge_object_builder.hpp"
#include "eve/knowledge/markdown_parser.hpp"
#include "eve/knowledge/metadata_extractor.hpp"

#include <gtest/gtest.h>

#ifndef EVE_PROJECT_ROOT
#define EVE_PROJECT_ROOT "."
#endif

namespace eve {
namespace {

DocumentMetadata extract(const DocumentModel& model) {
    return MetadataExtractor::extract(model);
}

DocumentMetadata extract(
    const DocumentModel& model,
    const MetadataExtractionContext& context) {
    return MetadataExtractor::extract(model, context);
}

RepositoryResource make_source(std::string relative_path) {
    const auto root = std::filesystem::path{EVE_PROJECT_ROOT};
    return RepositoryResource{
        .absolute_path = root / relative_path,
        .relative_path = relative_path,
        .file_type = RepositoryFileType::Markdown,
        .repository_root = root,
    };
}

KnowledgeObject build(const DocumentModel& model, const DocumentMetadata& metadata) {
    return KnowledgeObjectBuilder::build(model, metadata);
}

TEST(KnowledgeObjectBuilderTest, BuildsFromSimpleSpecification) {
    const auto model = MarkdownParser::parse(
        "# EVE-0099\n\n"
        "# Sample Specification\n\n"
        "**Document Version:** 1.0.0\n\n"
        "**Authors:** VMRI\n\n"
        "**Status:** Draft\n\n"
        "---\n\n"
        "# Purpose\n\n"
        "Purpose paragraph.\n\n"
        "## Details\n\n"
        "- Alpha\n"
        "- Beta\n\n"
        "```text\n"
        "Diagram\n"
        "```\n\n"
        "See [EVE-SPEC](../README.md).\n");

    DocumentMetadata metadata = extract(model);
    metadata.repository = "eve-docs";
    metadata.identifier = "EVE-0099";
    metadata.version = "1.0.0";
    metadata.authors = "VMRI";
    metadata.status = "Draft";

    const auto object = build(model, metadata);

    EXPECT_EQ(object.identity.id.value, "EVE-0099");
    EXPECT_EQ(object.identity.title, "Sample Specification");
    EXPECT_EQ(object.identity.document_type, DocumentType::Specification);
    EXPECT_EQ(object.metadata.authors, "VMRI");
    EXPECT_GE(object.content.headings.size(), 3U);
    EXPECT_FALSE(object.content.paragraphs.empty());
    EXPECT_FALSE(object.content.lists.empty());
    EXPECT_FALSE(object.content.code_blocks.empty());
    EXPECT_FALSE(object.content.links.empty());
    EXPECT_FALSE(object.content.sections.empty());
    EXPECT_FALSE(object.search.search_text.empty());
    EXPECT_EQ(object.search.priority_rank, priority_rank_for_document_type(object.identity.document_type));
}

TEST(KnowledgeObjectBuilderTest, BuildsFromRealEveDocument) {
    const auto source = make_source(
        "docs/0000-core-platform-specifications/eve-0012-validation-specification.md");
    const auto model = MarkdownParser::parse_file(source);
    const auto metadata = extract(
        model,
        MetadataExtractionContext{.repository_name = "eve-docs"});

    const auto object = build(model, metadata);

    EXPECT_EQ(object.identity.document_identifier, "EVE-0012");
    EXPECT_EQ(object.identity.title, "Validation Specification");
    EXPECT_EQ(object.identity.document_type, DocumentType::Specification);
    EXPECT_EQ(object.identity.repository, "eve-docs");
    EXPECT_FALSE(object.metadata.status.empty());
    EXPECT_EQ(object.metadata.status, "Draft");
    EXPECT_FALSE(object.content.raw_markdown.empty());
    EXPECT_FALSE(object.content.headings.empty());
    EXPECT_GT(object.search.priority_rank, 0);
}

TEST(KnowledgeObjectBuilderTest, HandlesEmptyDocument) {
    const auto model = MarkdownParser::parse("");
    const auto metadata = extract(model);
    const auto object = build(model, metadata);

    EXPECT_TRUE(object.identity.title.empty());
    EXPECT_TRUE(object.content.paragraphs.empty());
    EXPECT_TRUE(object.content.headings.empty());
    EXPECT_TRUE(object.content.sections.empty());
    EXPECT_TRUE(object.relationships.references.empty());
    EXPECT_TRUE(object.relationships.referenced_by.empty());
    EXPECT_TRUE(object.relationships.children.empty());
}

TEST(KnowledgeObjectBuilderTest, HandlesMissingMetadata) {
    const auto model = MarkdownParser::parse("# Untitled\n\nBody text.\n");
    DocumentMetadata metadata;
    metadata.title = "Untitled";

    const auto object = build(model, metadata);

    EXPECT_EQ(object.identity.title, "Untitled");
    EXPECT_TRUE(object.identity.version.empty());
    EXPECT_TRUE(object.metadata.status.empty());
    EXPECT_TRUE(object.metadata.authors.empty());
    EXPECT_TRUE(object.relationships.previous.has_value() == false);
    EXPECT_TRUE(object.relationships.next.has_value() == false);
}

TEST(KnowledgeObjectBuilderTest, PreservesHeadings) {
    const auto model = MarkdownParser::parse("# One\n\n## Two\n\n### Three\n");
    const auto object = build(model, extract(model));

    ASSERT_EQ(object.content.headings.size(), 3U);
    EXPECT_EQ(object.content.headings[0].text, "One");
    EXPECT_EQ(object.content.headings[2].level, 3);
}

TEST(KnowledgeObjectBuilderTest, PreservesCodeBlocks) {
    const auto model = MarkdownParser::parse("```cpp\nint main(){}\n```\n");
    const auto object = build(model, extract(model));

    ASSERT_EQ(object.content.code_blocks.size(), 1U);
    EXPECT_EQ(object.content.code_blocks.front().language, "cpp");
    EXPECT_NE(object.content.code_blocks.front().content.find("main"), std::string::npos);
}

TEST(KnowledgeObjectBuilderTest, PreservesLinks) {
    const auto model = MarkdownParser::parse("[Spec](../docs/spec.md)\n");
    const auto object = build(model, extract(model));

    ASSERT_EQ(object.content.links.size(), 1U);
    EXPECT_EQ(object.content.links.front().text, "Spec");
    EXPECT_EQ(object.content.links.front().destination, "../docs/spec.md");
    EXPECT_TRUE(object.relationships.references.empty());
}

TEST(KnowledgeObjectBuilderTest, PreservesSectionsAndNavigationRelationships) {
    const auto model = MarkdownParser::parse(
        "# Root\n\n"
        "Intro.\n\n"
        "## Child\n\n"
        "Child body.\n\n"
        "## Navigation\n\n"
        "**Previous**\n\n"
        "EVE-0011 — Platform Response Specification\n\n"
        "**Next**\n\n"
        "Reserved\n");

    DocumentMetadata metadata = extract(model);
    metadata.previous_document = "EVE-0011 — Platform Response Specification";

    const auto object = build(model, metadata);

    ASSERT_GE(object.content.sections.size(), 2U);
    EXPECT_EQ(object.content.sections.front().title, "Root");
    ASSERT_TRUE(object.relationships.previous.has_value());
    EXPECT_EQ(*object.relationships.previous, "EVE-0011 — Platform Response Specification");
    EXPECT_FALSE(object.relationships.next.has_value());
}

TEST(KnowledgeObjectBuilderTest, DoesNotModifyInputs) {
    const auto model = MarkdownParser::parse("# Title\n\nBody\n");
    auto metadata = extract(model);
    const auto heading_count = model.headings.size();
    const auto metadata_title = metadata.title;

    const auto object = build(model, metadata);

    EXPECT_EQ(model.headings.size(), heading_count);
    EXPECT_EQ(metadata.title, metadata_title);
    EXPECT_EQ(object.identity.title, "Title");
}

}  // namespace
}  // namespace eve
