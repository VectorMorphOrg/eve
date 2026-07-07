#pragma once

#include "eve/knowledge/repository_index.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace eve {

struct DocumentLink {
    std::string text;
    std::string destination;
    std::size_t block_index{0};
};

struct MetadataEntry {
    std::string key;
    std::string value;
};

struct DocumentMetadataBlock {
    std::vector<MetadataEntry> entries;
};

struct DocumentHeading {
    int level{1};
    std::string text;
    std::size_t block_index{0};
};

struct DocumentParagraph {
    std::string text;
};

struct DocumentListItem {
    int indent{0};
    std::string text;
    int number{0};
};

struct DocumentList {
    bool ordered{false};
    std::vector<DocumentListItem> items;
};

struct DocumentTableRow {
    bool header{false};
    std::vector<std::string> cells;
};

struct DocumentTable {
    std::vector<DocumentTableRow> rows;
};

struct DocumentCodeBlock {
    std::string language;
    std::string content;
};

struct DocumentSection {
    std::string title;
    int level{1};
    std::size_t heading_block_index{0};
    std::size_t start_block_index{0};
    std::size_t end_block_index{0};
};

enum class DocumentBlockKind {
    Metadata,
    Heading,
    Paragraph,
    List,
    Table,
    CodeBlock,
    HorizontalRule,
};

struct DocumentBlock {
    DocumentBlockKind kind{DocumentBlockKind::Paragraph};
    DocumentMetadataBlock metadata;
    DocumentHeading heading;
    DocumentParagraph paragraph;
    DocumentList list;
    DocumentTable table;
    DocumentCodeBlock code_block;
};

struct DocumentModel {
    RepositoryResource source{};
    bool has_source{false};
    std::string title;
    std::string raw_markdown;
    DocumentMetadataBlock metadata;
    std::vector<DocumentBlock> blocks;
    std::vector<DocumentHeading> headings;
    std::vector<DocumentSection> sections;
    std::vector<DocumentLink> links;
};

}  // namespace eve
