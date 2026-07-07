#include "eve/knowledge/markdown_parser.hpp"
#include "eve/knowledge/metadata_extractor.hpp"

#include <gtest/gtest.h>

#ifndef EVE_PROJECT_ROOT
#define EVE_PROJECT_ROOT "."
#endif

namespace eve {
namespace {

DocumentModel make_model(
    DocumentMetadataBlock metadata,
    std::string title = {},
    RepositoryResource source = {},
    bool has_source = false) {
    DocumentModel model;
    model.metadata = std::move(metadata);
    model.title = std::move(title);
    model.source = std::move(source);
    model.has_source = has_source;
    return model;
}

RepositoryResource make_source(std::string relative_path, std::string repository_name = "eve") {
    const auto root = std::filesystem::path{EVE_PROJECT_ROOT};
    return RepositoryResource{
        .absolute_path = root / relative_path,
        .relative_path = relative_path,
        .file_type = RepositoryFileType::Markdown,
        .repository_root = root,
    };
}

TEST(MetadataExtractorTest, ExtractsExplicitMetadata) {
    DocumentModel model = make_model(
        DocumentMetadataBlock{
            .entries = {
                {.key = "Document Version", .value = "1.0.0"},
                {.key = "Authors", .value = "VectorMorph Research Initiative (VMRI)"},
                {.key = "Status", .value = "Draft"},
                {.key = "Published", .value = "July 2026"},
                {.key = "Project", .value = "Evolutionary Virtual Engineer (E.V.E.)"},
                {.key = "Specification", .value = "EVE-SPEC"},
            },
        },
        "Validation Specification");

    const auto metadata = MetadataExtractor::extract(model);

    EXPECT_EQ(metadata.version, "1.0.0");
    EXPECT_EQ(metadata.authors, "VectorMorph Research Initiative (VMRI)");
    EXPECT_EQ(metadata.status, "Draft");
    EXPECT_EQ(metadata.published, "July 2026");
    EXPECT_EQ(metadata.project, "Evolutionary Virtual Engineer (E.V.E.)");
    EXPECT_EQ(metadata.specification, "EVE-SPEC");
    EXPECT_EQ(metadata.explicit_entries.size(), 6U);
}

TEST(MetadataExtractorTest, InfersRepositoryMetadataFromPath) {
    const auto source = make_source(
        "docs/0000-core-platform-specifications/eve-0012-validation-specification.md");
    DocumentModel model = make_model({}, "Validation Specification", source, true);

    const auto metadata = MetadataExtractor::extract(
        model,
        MetadataExtractionContext{.repository_name = "eve-docs"});

    EXPECT_EQ(metadata.identifier, "EVE-0012");
    EXPECT_EQ(metadata.series, "0000");
    EXPECT_EQ(metadata.part, "I — Core Platform Specifications");
    EXPECT_EQ(metadata.repository, "eve-docs");
    EXPECT_EQ(metadata.file_path->generic_string(),
              "docs/0000-core-platform-specifications/eve-0012-validation-specification.md");
    EXPECT_EQ(metadata.document_type, DocumentType::Specification);
}

TEST(MetadataExtractorTest, HandlesMissingMetadata) {
    DocumentModel model = make_model({}, "Untitled Document");

    const auto metadata = MetadataExtractor::extract(model);

    EXPECT_EQ(metadata.title, "Untitled Document");
    EXPECT_FALSE(metadata.version.has_value());
    EXPECT_FALSE(metadata.authors.has_value());
    EXPECT_FALSE(metadata.identifier.has_value());
    EXPECT_FALSE(metadata.previous_document.has_value());
    EXPECT_FALSE(metadata.next_document.has_value());
}

TEST(MetadataExtractorTest, IgnoresMalformedExplicitMetadata) {
    DocumentModel model = make_model(
        DocumentMetadataBlock{
            .entries = {
                {.key = "", .value = "Ignored"},
                {.key = "Status", .value = ""},
                {.key = "   ", .value = "Also Ignored"},
                {.key = "Status", .value = "Draft"},
            },
        },
        "Document");

    const auto metadata = MetadataExtractor::extract(model);

    EXPECT_EQ(metadata.status, "Draft");
    EXPECT_FALSE(metadata.version.has_value());
}

TEST(MetadataExtractorTest, ExtractsNavigationMetadata) {
    const auto model = MarkdownParser::parse(
        "# EVE-0011\n\n"
        "# Platform Response Specification\n\n"
        "## Navigation\n\n"
        "**Previous**\n\n"
        "EVE-0010 — Platform Request Specification\n\n"
        "**Part**\n\n"
        "I — Core Platform Specifications\n\n"
        "**Next**\n\n"
        "EVE-0012 — Validation Specification\n");

    const auto metadata = MetadataExtractor::extract(model);

    ASSERT_TRUE(metadata.previous_document.has_value());
    ASSERT_TRUE(metadata.next_document.has_value());
    EXPECT_EQ(*metadata.previous_document, "EVE-0010 — Platform Request Specification");
    EXPECT_EQ(*metadata.next_document, "EVE-0012 — Validation Specification");
    EXPECT_EQ(*metadata.part, "I — Core Platform Specifications");
}

TEST(MetadataExtractorTest, InfersSeriesNavigationFromRepositoryPaths) {
    const auto source = make_source(
        "docs/0000-core-platform-specifications/eve-0011-platform-response-specification.md");
    DocumentModel model = make_model({}, "Platform Response Specification", source, true);

    MetadataExtractionContext context{
        .repository_name = "eve-docs",
        .series_documents = {
            std::filesystem::path{
                "docs/0000-core-platform-specifications/eve-0010-platform-request-specification.md"},
            std::filesystem::path{
                "docs/0000-core-platform-specifications/eve-0011-platform-response-specification.md"},
            std::filesystem::path{
                "docs/0000-core-platform-specifications/eve-0012-validation-specification.md"},
        },
    };

    const auto metadata = MetadataExtractor::extract(model, context);

    EXPECT_EQ(*metadata.previous_document, "EVE-0010");
    EXPECT_EQ(*metadata.next_document, "EVE-0012");
}

TEST(MetadataExtractorTest, DoesNotModifyDocumentModel) {
    DocumentModel model = make_model(
        DocumentMetadataBlock{
            .entries = {{.key = "Status", .value = "Draft"}},
        },
        "Original Title");
    const auto metadata_snapshot = model.metadata;
    const auto title_snapshot = model.title;

    const auto metadata = MetadataExtractor::extract(model);

    EXPECT_EQ(model.metadata.entries.size(), metadata_snapshot.entries.size());
    ASSERT_FALSE(model.metadata.entries.empty());
    EXPECT_EQ(model.metadata.entries.front().key, metadata_snapshot.entries.front().key);
    EXPECT_EQ(model.metadata.entries.front().value, metadata_snapshot.entries.front().value);
    EXPECT_EQ(model.title, title_snapshot);
    EXPECT_EQ(metadata.status, "Draft");
}

TEST(MetadataExtractorTest, ExtractsFromRealSpecificationFile) {
    const auto source = make_source(
        "docs/0000-core-platform-specifications/eve-0012-validation-specification.md");
    const auto model = MarkdownParser::parse_file(source);
    const auto metadata = MetadataExtractor::extract(
        model,
        MetadataExtractionContext{.repository_name = "eve-docs"});

    EXPECT_EQ(metadata.identifier, "EVE-0012");
    EXPECT_EQ(metadata.title, "Validation Specification");
    EXPECT_TRUE(metadata.version.has_value());
    EXPECT_TRUE(metadata.status.has_value());
    EXPECT_EQ(metadata.document_type, DocumentType::Specification);
}

}  // namespace
}  // namespace eve
