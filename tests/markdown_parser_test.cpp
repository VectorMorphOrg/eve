#include "eve/knowledge/markdown_parser.hpp"

#include <gtest/gtest.h>

#ifndef EVE_PROJECT_ROOT
#define EVE_PROJECT_ROOT "."
#endif

namespace eve {
namespace {

DocumentModel parse(std::string markdown) {
    return MarkdownParser::parse(std::move(markdown));
}

const DocumentBlock* find_block(const DocumentModel& model, DocumentBlockKind kind, std::size_t occurrence = 0) {
    std::size_t seen = 0;
    for (const auto& block : model.blocks) {
        if (block.kind != kind) {
            continue;
        }
        if (seen++ == occurrence) {
            return &block;
        }
    }
    return nullptr;
}

TEST(MarkdownParserTest, ParsesHeadings) {
    const auto model = parse("# Title\n\n## Section\n\n### Subsection\n");
    ASSERT_GE(model.headings.size(), 3U);
    EXPECT_EQ(model.headings[0].text, "Title");
    EXPECT_EQ(model.headings[0].level, 1);
    EXPECT_EQ(model.headings[1].text, "Section");
    EXPECT_EQ(model.headings[1].level, 2);
    EXPECT_EQ(model.headings[2].text, "Subsection");
    EXPECT_EQ(model.headings[2].level, 3);
}

TEST(MarkdownParserTest, ParsesNestedHeadingSections) {
    const auto model = parse(
        "# Root\n\nIntro paragraph.\n\n## Child\n\nChild paragraph.\n\n# Sibling\n\nSibling paragraph.\n");

    ASSERT_EQ(model.sections.size(), 3U);
    EXPECT_EQ(model.sections[0].title, "Root");
    EXPECT_EQ(model.sections[0].level, 1);
    EXPECT_EQ(model.sections[1].title, "Child");
    EXPECT_EQ(model.sections[1].level, 2);
    EXPECT_EQ(model.sections[2].title, "Sibling");
    EXPECT_EQ(model.sections[2].level, 1);
    EXPECT_LT(model.sections[0].end_block_index, model.sections[2].start_block_index);
}

TEST(MarkdownParserTest, ParsesEveSpecMetadataBlockAndTitle) {
    const auto model = parse(
        "# EVE-0010\n\n"
        "# Platform Request Specification\n\n"
        "**Project:** Evolutionary Virtual Engineer (E.V.E.)\n\n"
        "**Status:** Draft\n\n"
        "---\n\n"
        "# Purpose\n\n"
        "Purpose text.\n");

    EXPECT_EQ(model.title, "Platform Request Specification");
    ASSERT_GE(model.metadata.entries.size(), 2U);
    EXPECT_EQ(model.metadata.entries[0].key, "Project");
    EXPECT_EQ(model.metadata.entries[1].key, "Status");

    const auto* metadata_block = find_block(model, DocumentBlockKind::Metadata);
    ASSERT_NE(metadata_block, nullptr);
    EXPECT_EQ(metadata_block->metadata.entries.size(), model.metadata.entries.size());
}

TEST(MarkdownParserTest, ParsesLists) {
    const auto model = parse(
        "- Alpha\n"
        "- Beta\n\n"
        "1. First\n"
        "2. Second\n");

    const auto* unordered = find_block(model, DocumentBlockKind::List, 0);
    const auto* ordered = find_block(model, DocumentBlockKind::List, 1);
    ASSERT_NE(unordered, nullptr);
    ASSERT_NE(ordered, nullptr);
    ASSERT_EQ(unordered->list.items.size(), 2U);
    EXPECT_EQ(unordered->list.items[0].text, "Alpha");
    EXPECT_FALSE(unordered->list.ordered);
    ASSERT_EQ(ordered->list.items.size(), 2U);
    EXPECT_TRUE(ordered->list.ordered);
    EXPECT_EQ(ordered->list.items[1].number, 2);
}

TEST(MarkdownParserTest, ParsesNestedListIndentation) {
    const auto model = parse("- Parent\n  - Child\n");
    const auto* list = find_block(model, DocumentBlockKind::List);
    ASSERT_NE(list, nullptr);
    ASSERT_EQ(list->list.items.size(), 2U);
    EXPECT_EQ(list->list.items[0].indent, 0);
    EXPECT_EQ(list->list.items[1].indent, 1);
}

TEST(MarkdownParserTest, ParsesCodeBlocks) {
    const auto model = parse(
        "```text\n"
        "Interface\n\n"
        "↓\n\n"
        "Platform Request\n"
        "```\n");
    const auto* code = find_block(model, DocumentBlockKind::CodeBlock);
    ASSERT_NE(code, nullptr);
    EXPECT_EQ(code->code_block.language, "text");
    EXPECT_NE(code->code_block.content.find("Platform Request"), std::string::npos);
}

TEST(MarkdownParserTest, ParsesTables) {
    const auto model = parse(
        "| Status | Meaning |\n"
        "|--------|---------|\n"
        "| Active | Enabled |\n");
    const auto* table = find_block(model, DocumentBlockKind::Table);
    ASSERT_NE(table, nullptr);
    ASSERT_EQ(table->table.rows.size(), 2U);
    EXPECT_TRUE(table->table.rows[0].header);
    EXPECT_EQ(table->table.rows[0].cells[0], "Status");
    EXPECT_FALSE(table->table.rows[1].header);
    EXPECT_EQ(table->table.rows[1].cells[1], "Enabled");
}

TEST(MarkdownParserTest, ParsesLinks) {
    const auto model = parse("See [EVE-SPEC](../README.md) for details.\n");
    ASSERT_EQ(model.links.size(), 1U);
    EXPECT_EQ(model.links[0].text, "EVE-SPEC");
    EXPECT_EQ(model.links[0].destination, "../README.md");
}

TEST(MarkdownParserTest, PreservesDocumentOrder) {
    const auto model = parse(
        "# Title\n\n"
        "Paragraph one.\n\n"
        "- Item\n\n"
        "```\n"
        "code\n"
        "```\n\n"
        "| A | B |\n"
        "| - | - |\n"
        "| 1 | 2 |\n");

    std::vector<DocumentBlockKind> kinds;
    for (const auto& block : model.blocks) {
        kinds.push_back(block.kind);
    }

    ASSERT_GE(kinds.size(), 5U);
    EXPECT_EQ(kinds[0], DocumentBlockKind::Heading);
    EXPECT_EQ(kinds[1], DocumentBlockKind::Paragraph);
    EXPECT_EQ(kinds[2], DocumentBlockKind::List);
    EXPECT_EQ(kinds[3], DocumentBlockKind::CodeBlock);
    EXPECT_EQ(kinds[4], DocumentBlockKind::Table);
}

TEST(MarkdownParserTest, ParsesEmptyDocument) {
    const auto model = parse("");
    EXPECT_TRUE(model.title.empty());
    EXPECT_TRUE(model.blocks.empty());
    EXPECT_TRUE(model.headings.empty());
    EXPECT_TRUE(model.raw_markdown.empty());
}

TEST(MarkdownParserTest, HandlesMalformedMarkdownDeterministically) {
    const auto model = parse(
        "#Missing space heading\n\n"
        "```\n"
        "line 1\n"
        "line 2\n\n"
        "Still a paragraph.\n");

    const auto* paragraph = find_block(model, DocumentBlockKind::Paragraph, 0);
    const auto* code = find_block(model, DocumentBlockKind::CodeBlock);
    ASSERT_NE(paragraph, nullptr);
    ASSERT_NE(code, nullptr);
    EXPECT_EQ(paragraph->paragraph.text, "#Missing space heading");
    EXPECT_NE(code->code_block.content.find("line 2"), std::string::npos);
}

TEST(MarkdownParserTest, ParsesRepositoryResourceFile) {
    RepositoryResource resource{
        .absolute_path = std::filesystem::path{EVE_PROJECT_ROOT} /
                         "docs/0000-core-platform-specifications/eve-0012-validation-specification.md",
        .relative_path = "docs/0000-core-platform-specifications/eve-0012-validation-specification.md",
        .file_type = RepositoryFileType::Markdown,
        .repository_root = std::filesystem::path{EVE_PROJECT_ROOT},
    };

    const auto model = MarkdownParser::parse_file(resource);
    EXPECT_TRUE(model.has_source);
    EXPECT_EQ(model.title, "Validation Specification");
    EXPECT_FALSE(model.raw_markdown.empty());
    EXPECT_FALSE(model.headings.empty());
    EXPECT_FALSE(model.blocks.empty());
}

}  // namespace
}  // namespace eve
