#include "eve/knowledge/markdown_parser.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <optional>
#include <sstream>
#include <utility>

namespace eve {
namespace {

std::string trim(std::string value) {
    const auto not_space = [](unsigned char ch) { return !std::isspace(ch); };
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), not_space));
    value.erase(std::find_if(value.rbegin(), value.rend(), not_space).base(), value.end());
    return value;
}

bool is_blank(std::string_view line) {
    return trim(std::string{line}).empty();
}

std::vector<std::string> split_lines(std::string_view markdown) {
    std::vector<std::string> lines;
    std::string current;
    std::istringstream stream{std::string{markdown}};
    while (std::getline(stream, current)) {
        if (!current.empty() && current.back() == '\r') {
            current.pop_back();
        }
        lines.push_back(current);
    }
    return lines;
}

std::optional<DocumentHeading> parse_heading(std::string_view line) {
    std::size_t index = 0;
    while (index < line.size() && line[index] == '#') {
        ++index;
    }
    if (index == 0 || index > 6 || index >= line.size() || line[index] != ' ') {
        return std::nullopt;
    }
    return DocumentHeading{
        .level = static_cast<int>(index),
        .text = trim(std::string{line.substr(index + 1)}),
    };
}

bool is_horizontal_rule(std::string_view line) {
    const auto trimmed = trim(std::string{line});
    if (trimmed.size() < 3) {
        return false;
    }
    const char marker = trimmed.front();
    if (marker != '-' && marker != '*' && marker != '_') {
        return false;
    }
    return std::all_of(trimmed.begin(), trimmed.end(), [marker](char ch) {
        return ch == marker || ch == ' ';
    });
}

std::optional<MetadataEntry> parse_metadata_line(std::string_view line) {
    const auto trimmed = trim(std::string{line});
    if (!trimmed.starts_with("**") || !trimmed.contains(":**")) {
        return std::nullopt;
    }

    const auto close = trimmed.find(":**");
    if (close == std::string::npos || close <= 2) {
        return std::nullopt;
    }

    const auto key = trim(trimmed.substr(2, close - 2));
    const auto value = trim(trimmed.substr(close + 3));
    if (key.empty()) {
        return std::nullopt;
    }
    return MetadataEntry{.key = key, .value = value};
}

bool is_table_row(std::string_view line) {
    const auto trimmed = trim(std::string{line});
    return trimmed.starts_with('|') && trimmed.size() > 1 && trimmed.find('|', 1) != std::string::npos;
}

bool is_table_separator(std::string_view line) {
    if (!is_table_row(line)) {
        return false;
    }
    const auto trimmed = trim(std::string{line});
    for (char ch : trimmed) {
        if (ch == '|' || ch == '-' || ch == ':' || ch == ' ') {
            continue;
        }
        return false;
    }
    return trimmed.contains('-');
}

std::vector<std::string> split_table_cells(std::string_view line) {
    std::vector<std::string> cells;
    const auto trimmed = trim(std::string{line});
    std::string cell;
    bool inside = false;
    for (std::size_t index = 0; index < trimmed.size(); ++index) {
        const char ch = trimmed[index];
        if (ch == '|') {
            if (inside) {
                cells.push_back(trim(cell));
                cell.clear();
            } else {
                inside = true;
            }
            continue;
        }
        cell.push_back(ch);
    }
    if (inside) {
        cells.push_back(trim(cell));
    }
    return cells;
}

struct ListMatch {
    int indent{0};
    std::string text;
    bool ordered{false};
    int number{0};
};

std::optional<ListMatch> parse_list_item(std::string_view line) {
    std::size_t index = 0;
    while (index < line.size() && line[index] == ' ') {
        ++index;
    }
    const int indent = static_cast<int>(index / 2);

    if (index < line.size() && (line[index] == '-' || line[index] == '*' || line[index] == '+') &&
        index + 1 < line.size() && line[index + 1] == ' ') {
        return ListMatch{
            .indent = indent,
            .text = trim(std::string{line.substr(index + 2)}),
            .ordered = false,
        };
    }

    std::size_t digit = index;
    while (digit < line.size() && std::isdigit(static_cast<unsigned char>(line[digit]))) {
        ++digit;
    }
    if (digit > index && digit + 1 < line.size() && line[digit] == '.' && line[digit + 1] == ' ') {
        return ListMatch{
            .indent = indent,
            .text = trim(std::string{line.substr(digit + 2)}),
            .ordered = true,
            .number = std::stoi(std::string{line.substr(index, digit - index)}),
        };
    }

    return std::nullopt;
}

bool is_fence_start(std::string_view line, std::string* language) {
    const auto trimmed = trim(std::string{line});
    if (trimmed.size() < 3) {
        return false;
    }
    if (!(trimmed.starts_with("```") || trimmed.starts_with("~~~"))) {
        return false;
    }
    const char marker = trimmed.front();
    if (!std::all_of(trimmed.begin(), trimmed.begin() + 3, [marker](char ch) { return ch == marker; })) {
        return false;
    }
    *language = trim(trimmed.substr(3));
    return true;
}

bool is_fence_end(std::string_view line, char marker) {
    const auto trimmed = trim(std::string{line});
    if (trimmed.size() < 3) {
        return false;
    }
    if (trimmed.front() != marker) {
        return false;
    }
    return std::all_of(trimmed.begin(), trimmed.end(), [marker](char ch) { return ch == marker; });
}

void extract_links(std::string_view text, std::size_t block_index, std::vector<DocumentLink>& links) {
    for (std::size_t index = 0; index < text.size();) {
        const auto open = text.find('[', index);
        if (open == std::string_view::npos) {
            break;
        }
        const auto middle = text.find("](", open + 1);
        if (middle == std::string_view::npos) {
            break;
        }
        const auto close = text.find(')', middle + 2);
        if (close == std::string_view::npos) {
            break;
        }

        links.push_back(DocumentLink{
            .text = std::string{text.substr(open + 1, middle - open - 1)},
            .destination = std::string{text.substr(middle + 2, close - middle - 2)},
            .block_index = block_index,
        });
        index = close + 1;
    }
}

void append_paragraph_line(DocumentParagraph& paragraph, std::string_view line) {
    if (!paragraph.text.empty()) {
        paragraph.text.push_back('\n');
    }
    paragraph.text.append(line);
}

void finalize_paragraph(DocumentModel& model, DocumentParagraph& paragraph) {
    if (paragraph.text.empty()) {
        return;
    }
    const auto block_index = model.blocks.size();
    extract_links(paragraph.text, block_index, model.links);
    model.blocks.push_back(DocumentBlock{
        .kind = DocumentBlockKind::Paragraph,
        .paragraph = paragraph,
    });
    paragraph = DocumentParagraph{};
}

void finalize_list(DocumentModel& model, DocumentList& list) {
    if (list.items.empty()) {
        return;
    }
    const auto block_index = model.blocks.size();
    for (const auto& item : list.items) {
        extract_links(item.text, block_index, model.links);
    }
    model.blocks.push_back(DocumentBlock{
        .kind = DocumentBlockKind::List,
        .list = list,
    });
    list = DocumentList{};
}

void build_sections(DocumentModel& model) {
    struct SectionFrame {
        std::string title;
        int level{1};
        std::size_t heading_block_index{0};
        std::size_t start_block_index{0};
    };

    std::vector<SectionFrame> stack;
    for (std::size_t index = 0; index < model.blocks.size(); ++index) {
        const auto& block = model.blocks[index];
        if (block.kind != DocumentBlockKind::Heading) {
            continue;
        }

        while (!stack.empty() && stack.back().level >= block.heading.level) {
            const auto frame = stack.back();
            stack.pop_back();
            model.sections.push_back(DocumentSection{
                .title = frame.title,
                .level = frame.level,
                .heading_block_index = frame.heading_block_index,
                .start_block_index = frame.start_block_index,
                .end_block_index = index,
            });
        }

        stack.push_back(SectionFrame{
            .title = block.heading.text,
            .level = block.heading.level,
            .heading_block_index = index,
            .start_block_index = index + 1,
        });
    }

    while (!stack.empty()) {
        const auto frame = stack.back();
        stack.pop_back();
        model.sections.push_back(DocumentSection{
            .title = frame.title,
            .level = frame.level,
            .heading_block_index = frame.heading_block_index,
            .start_block_index = frame.start_block_index,
            .end_block_index = model.blocks.size(),
        });
    }

    std::sort(
        model.sections.begin(),
        model.sections.end(),
        [](const DocumentSection& lhs, const DocumentSection& rhs) {
            return lhs.heading_block_index < rhs.heading_block_index;
        });
}

void assign_title(DocumentModel& model) {
    std::string identifier_heading;
    for (const auto& heading : model.headings) {
        if (heading.level != 1) {
            continue;
        }
        if (identifier_heading.empty()) {
            identifier_heading = heading.text;
            if (heading.text.starts_with("EVE-")) {
                continue;
            }
            model.title = heading.text;
            return;
        }
        model.title = heading.text;
        return;
    }

    if (!identifier_heading.empty()) {
        model.title = identifier_heading;
        return;
    }

    if (!model.metadata.entries.empty()) {
        model.title = model.metadata.entries.front().value;
    }
}

DocumentModel parse_lines(const std::vector<std::string>& lines, std::string raw_markdown) {
    DocumentModel model;
    model.raw_markdown = std::move(raw_markdown);

    DocumentMetadataBlock pending_metadata;
    DocumentParagraph paragraph;
    DocumentList list;
    bool metadata_region = true;
    bool seen_horizontal_rule = false;

    for (std::size_t line_index = 0; line_index < lines.size();) {
        const auto& line = lines[line_index];

        if (is_blank(line)) {
            finalize_paragraph(model, paragraph);
            finalize_list(model, list);
            ++line_index;
            continue;
        }

        if (is_horizontal_rule(line)) {
            finalize_paragraph(model, paragraph);
            finalize_list(model, list);
            seen_horizontal_rule = true;
            metadata_region = false;

            if (!pending_metadata.entries.empty()) {
                model.metadata = pending_metadata;
                model.blocks.push_back(DocumentBlock{
                    .kind = DocumentBlockKind::Metadata,
                    .metadata = pending_metadata,
                });
                pending_metadata = DocumentMetadataBlock{};
            }

            model.blocks.push_back(DocumentBlock{.kind = DocumentBlockKind::HorizontalRule});
            ++line_index;
            continue;
        }

        if (auto metadata = parse_metadata_line(line); metadata && metadata_region && !seen_horizontal_rule) {
            finalize_paragraph(model, paragraph);
            finalize_list(model, list);
            pending_metadata.entries.push_back(*metadata);
            ++line_index;
            continue;
        }

        if (auto metadata = parse_metadata_line(line); metadata && !seen_horizontal_rule) {
            finalize_paragraph(model, paragraph);
            finalize_list(model, list);
            pending_metadata.entries.push_back(*metadata);
            ++line_index;
            continue;
        }

        metadata_region = false;

        if (auto heading = parse_heading(line)) {
            finalize_paragraph(model, paragraph);
            finalize_list(model, list);

            if (!pending_metadata.entries.empty()) {
                model.metadata = pending_metadata;
                model.blocks.push_back(DocumentBlock{
                    .kind = DocumentBlockKind::Metadata,
                    .metadata = pending_metadata,
                });
                pending_metadata = DocumentMetadataBlock{};
            }

            heading->block_index = model.blocks.size();
            model.headings.push_back(*heading);
            model.blocks.push_back(DocumentBlock{
                .kind = DocumentBlockKind::Heading,
                .heading = *heading,
            });
            ++line_index;
            continue;
        }

        std::string fence_language;
        if (is_fence_start(line, &fence_language)) {
            finalize_paragraph(model, paragraph);
            finalize_list(model, list);

            const char marker = trim(std::string{line}).front();
            ++line_index;
            std::ostringstream content;
            while (line_index < lines.size() && !is_fence_end(lines[line_index], marker)) {
                if (!content.str().empty()) {
                    content << '\n';
                }
                content << lines[line_index];
                ++line_index;
            }
            if (line_index < lines.size()) {
                ++line_index;
            }

            model.blocks.push_back(DocumentBlock{
                .kind = DocumentBlockKind::CodeBlock,
                .code_block = DocumentCodeBlock{
                    .language = fence_language,
                    .content = content.str(),
                },
            });
            continue;
        }

        if (is_table_row(line)) {
            finalize_paragraph(model, paragraph);
            finalize_list(model, list);

            DocumentTable table;
            if (line_index + 1 < lines.size() && is_table_separator(lines[line_index + 1])) {
                table.rows.push_back(DocumentTableRow{
                    .header = true,
                    .cells = split_table_cells(line),
                });
                line_index += 2;
            } else {
                table.rows.push_back(DocumentTableRow{
                    .header = false,
                    .cells = split_table_cells(line),
                });
                ++line_index;
            }

            while (line_index < lines.size() && is_table_row(lines[line_index]) &&
                   !is_table_separator(lines[line_index])) {
                table.rows.push_back(DocumentTableRow{
                    .header = false,
                    .cells = split_table_cells(lines[line_index]),
                });
                ++line_index;
            }

            if (!table.rows.empty()) {
                const auto block_index = model.blocks.size();
                for (const auto& row : table.rows) {
                    for (const auto& cell : row.cells) {
                        extract_links(cell, block_index, model.links);
                    }
                }
                model.blocks.push_back(DocumentBlock{
                    .kind = DocumentBlockKind::Table,
                    .table = table,
                });
            }
            continue;
        }

        if (auto list_item = parse_list_item(line)) {
            finalize_paragraph(model, paragraph);

            if (!list.items.empty() && list.ordered != list_item->ordered) {
                finalize_list(model, list);
            }

            if (list.items.empty()) {
                list.ordered = list_item->ordered;
            }

            list.items.push_back(DocumentListItem{
                .indent = list_item->indent,
                .text = list_item->text,
                .number = list_item->number,
            });
            ++line_index;
            continue;
        }

        finalize_list(model, list);
        append_paragraph_line(paragraph, line);
        ++line_index;
    }

    finalize_paragraph(model, paragraph);
    finalize_list(model, list);

    if (!pending_metadata.entries.empty() && model.metadata.entries.empty()) {
        model.metadata = pending_metadata;
        model.blocks.push_back(DocumentBlock{
            .kind = DocumentBlockKind::Metadata,
            .metadata = pending_metadata,
        });
    }

    assign_title(model);
    build_sections(model);
    return model;
}

}  // namespace

DocumentModel MarkdownParser::parse(std::string markdown) {
    const auto lines = split_lines(markdown);
    return parse_lines(lines, std::move(markdown));
}

DocumentModel MarkdownParser::parse(const RepositoryResource& resource, std::string markdown) {
    const auto lines = split_lines(markdown);
    auto model = parse_lines(lines, std::move(markdown));
    model.source = resource;
    model.has_source = true;
    return model;
}

DocumentModel MarkdownParser::parse_file(const RepositoryResource& resource) {
    std::ifstream input(resource.absolute_path, std::ios::binary);
    std::ostringstream buffer;
    buffer << input.rdbuf();
    return parse(resource, buffer.str());
}

}  // namespace eve
