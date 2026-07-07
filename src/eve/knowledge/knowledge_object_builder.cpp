#include "eve/knowledge/knowledge_object_builder.hpp"

#include <algorithm>
#include <chrono>
#include <cctype>
#include <format>
#include <iomanip>
#include <sstream>
#include <unordered_set>

namespace eve {
namespace {

std::string optional_value(const std::optional<std::string>& value) {
    return value.value_or(std::string{});
}

std::string format_last_modified(TimePoint timestamp) {
    if (timestamp.time_since_epoch().count() == 0) {
        return {};
    }

    const auto time = std::chrono::system_clock::to_time_t(timestamp);
    std::tm tm{};
#if defined(_WIN32)
    gmtime_s(&tm, &time);
#else
    gmtime_r(&time, &tm);
#endif
    std::ostringstream stream;
    stream << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    return stream.str();
}

KnowledgeObjectId make_object_id(
    const DocumentMetadata& metadata,
    const DocumentModel& model) {
    if (metadata.identifier) {
        return KnowledgeObjectId{*metadata.identifier};
    }
    if (metadata.file_path && metadata.repository) {
        return KnowledgeObjectId{
            knowledge_object_id_from_path(*metadata.file_path, *metadata.repository)};
    }
    if (!model.title.empty()) {
        return KnowledgeObjectId{model.title};
    }
    return KnowledgeObjectId{"KO-UNKNOWN"};
}

std::vector<std::string> extract_tags(const DocumentMetadata& metadata) {
    std::vector<std::string> tags;
    for (const auto& entry : metadata.explicit_entries) {
        if (entry.key == "Tags" || entry.key == "Tag") {
            std::stringstream stream(entry.value);
            std::string tag;
            while (std::getline(stream, tag, ',')) {
                const auto start = tag.find_first_not_of(" \t");
                const auto end = tag.find_last_not_of(" \t");
                if (start != std::string::npos) {
                    tags.push_back(tag.substr(start, end - start + 1));
                }
            }
        }
    }
    return tags;
}

void append_unique(std::vector<std::string>& keywords, std::string keyword) {
    if (keyword.empty()) {
        return;
    }
    if (std::ranges::find(keywords, keyword) == keywords.end()) {
        keywords.push_back(std::move(keyword));
    }
}

void append_text(std::string& buffer, std::string_view text) {
    if (text.empty()) {
        return;
    }
    if (!buffer.empty()) {
        buffer.push_back('\n');
    }
    buffer.append(text);
}

std::string section_identifier(const DocumentSection& section, std::size_t index) {
    if (!section.title.empty()) {
        return section.title;
    }
    return std::format("section-{}", index);
}

KnowledgeSection build_knowledge_section(
    const DocumentModel& model,
    const DocumentSection& section,
    std::size_t index) {
    KnowledgeSection knowledge_section{
        .identifier = section_identifier(section, index),
        .title = section.title,
    };

    std::ostringstream content;
    for (std::size_t block_index = section.start_block_index; block_index < section.end_block_index &&
                                                             block_index < model.blocks.size();
         ++block_index) {
        const auto& block = model.blocks[block_index];
        switch (block.kind) {
            case DocumentBlockKind::Paragraph:
                if (!content.str().empty()) {
                    content << '\n';
                }
                content << block.paragraph.text;
                break;
            case DocumentBlockKind::List:
                for (const auto& item : block.list.items) {
                    content << '\n';
                    content << (block.list.ordered ? "- " : "- ");
                    content << item.text;
                }
                break;
            case DocumentBlockKind::CodeBlock:
                content << '\n';
                content << block.code_block.content;
                break;
            case DocumentBlockKind::Table:
                for (const auto& row : block.table.rows) {
                    content << '\n';
                    for (const auto& cell : row.cells) {
                        content << cell << " | ";
                    }
                }
                break;
            default:
                break;
        }
    }

    knowledge_section.content = content.str();
    return knowledge_section;
}

KnowledgeObjectContent build_content(const DocumentModel& model) {
    KnowledgeObjectContent content{
        .raw_markdown = model.raw_markdown,
    };

    content.headings = model.headings;
    content.links = model.links;

    for (std::size_t index = 0; index < model.blocks.size(); ++index) {
        const auto& block = model.blocks[index];
        switch (block.kind) {
            case DocumentBlockKind::Paragraph:
                content.paragraphs.push_back(KnowledgeParagraph{
                    .text = block.paragraph.text,
                    .block_index = index,
                });
                break;
            case DocumentBlockKind::List:
                content.lists.push_back(KnowledgeList{
                    .ordered = block.list.ordered,
                    .items = block.list.items,
                    .block_index = index,
                });
                break;
            case DocumentBlockKind::Table:
                content.tables.push_back(KnowledgeTable{
                    .rows = block.table.rows,
                    .block_index = index,
                });
                break;
            case DocumentBlockKind::CodeBlock:
                content.code_blocks.push_back(KnowledgeCodeBlock{
                    .language = block.code_block.language,
                    .content = block.code_block.content,
                    .block_index = index,
                });
                break;
            default:
                break;
        }
    }

    for (std::size_t index = 0; index < model.sections.size(); ++index) {
        content.sections.push_back(build_knowledge_section(model, model.sections[index], index));
    }

    return content;
}

KnowledgeObjectSearchData build_search_data(
    const KnowledgeObjectIdentity& identity,
    const KnowledgeObjectMetadata& metadata,
    const KnowledgeObjectContent& content) {
    KnowledgeObjectSearchData search{
        .priority_rank = priority_rank_for_document_type(identity.document_type),
    };

    append_text(search.search_text, identity.title);
    append_text(search.search_text, identity.document_identifier);
    append_text(search.search_text, identity.version);
    append_text(search.search_text, metadata.status);
    append_text(search.search_text, metadata.authors);
    append_text(search.search_text, metadata.part);
    append_text(search.search_text, metadata.series);

    for (const auto& heading : content.headings) {
        append_text(search.search_text, heading.text);
        append_unique(search.keywords, heading.text);
    }
    for (const auto& paragraph : content.paragraphs) {
        append_text(search.search_text, paragraph.text);
    }
    for (const auto& code_block : content.code_blocks) {
        append_text(search.search_text, code_block.content);
    }
    for (const auto& section : content.sections) {
        append_text(search.search_text, section.title);
        append_text(search.search_text, section.content);
    }

    append_unique(search.keywords, identity.document_identifier);
    append_unique(search.keywords, identity.title);
    append_unique(search.keywords, to_string(identity.document_type));
    for (const auto& tag : metadata.tags) {
        append_unique(search.keywords, tag);
    }

    search.excerpt = search.search_text.substr(0, std::min(search.search_text.size(), std::size_t{512}));
    return search;
}

KnowledgeObjectSource build_source(const DocumentModel& model, const DocumentMetadata& metadata) {
    KnowledgeObjectSource source;
    if (model.has_source) {
        source.repository_path = model.source.repository_root;
        source.file_path = model.source.relative_path;
        source.last_modified_time = model.source.last_modified;
        source.last_modified = format_last_modified(model.source.last_modified);
    } else if (metadata.file_path) {
        source.file_path = *metadata.file_path;
    }
    return source;
}

}  // namespace

KnowledgeObject KnowledgeObjectBuilder::build(
    const DocumentModel& model,
    const DocumentMetadata& metadata) {
    KnowledgeObject object;

    object.identity = KnowledgeObjectIdentity{
        .id = make_object_id(metadata, model),
        .document_identifier = optional_value(metadata.identifier),
        .title = metadata.title.empty() ? model.title : metadata.title,
        .document_type = metadata.document_type,
        .repository = optional_value(metadata.repository),
        .version = optional_value(metadata.version),
    };

    object.metadata = KnowledgeObjectMetadata{
        .status = optional_value(metadata.status),
        .authors = optional_value(metadata.authors),
        .published = optional_value(metadata.published),
        .part = optional_value(metadata.part),
        .series = optional_value(metadata.series),
        .tags = extract_tags(metadata),
    };

    object.content = build_content(model);

    object.relationships.previous = metadata.previous_document;
    object.relationships.next = metadata.next_document;

    object.search = build_search_data(object.identity, object.metadata, object.content);
    object.source = build_source(model, metadata);

    return object;
}

}  // namespace eve
