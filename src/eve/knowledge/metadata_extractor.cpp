#include "eve/knowledge/metadata_extractor.hpp"

#include <algorithm>
#include <cctype>
#include <map>
#include <regex>
#include <sstream>

namespace eve {
namespace {

std::string to_lower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

std::string trim(std::string value) {
    const auto not_space = [](unsigned char ch) { return !std::isspace(ch); };
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), not_space));
    value.erase(std::find_if(value.rbegin(), value.rend(), not_space).base(), value.end());
    return value;
}

std::string normalize_key(std::string key) {
    key = to_lower(trim(std::move(key)));
    key.erase(std::remove(key.begin(), key.end(), ' '), key.end());
    return key;
}

std::string strip_bold_markers(std::string value) {
    value = trim(std::move(value));
    while (value.starts_with("**")) {
        value.erase(0, 2);
    }
    while (value.ends_with("**")) {
        value.erase(value.size() - 2);
    }
    return trim(std::move(value));
}

std::optional<std::string> find_identifier_in_text(std::string_view text) {
    static const std::regex pattern(R"((EVE-\d{4}))", std::regex::icase);
    std::smatch match;
    const std::string subject{text};
    if (std::regex_search(subject, match, pattern)) {
        std::string identifier = match[1].str();
        std::transform(identifier.begin(), identifier.end(), identifier.begin(), [](unsigned char ch) {
            return static_cast<char>(std::toupper(ch));
        });
        return identifier;
    }
    return std::nullopt;
}

std::optional<std::string> infer_identifier_from_path(const std::filesystem::path& path) {
    return find_identifier_in_text(path.stem().string());
}

std::optional<std::string> infer_series_from_path(const std::filesystem::path& path) {
    for (const auto& component : path) {
        const std::string name = component.string();
        if (name.size() < 4) {
            continue;
        }
        if (!std::isdigit(static_cast<unsigned char>(name[0])) ||
            !std::isdigit(static_cast<unsigned char>(name[1])) ||
            !std::isdigit(static_cast<unsigned char>(name[2])) ||
            !std::isdigit(static_cast<unsigned char>(name[3]))) {
            continue;
        }
        if (name.size() == 4 || name[4] == '-') {
            return name.substr(0, 4);
        }
    }
    return std::nullopt;
}

std::optional<std::string> infer_part_from_series(
    const std::string& series,
    const std::filesystem::path& path) {
    static const std::map<std::string, std::string, std::less<>> kKnownParts{
        {"0000", "I — Core Platform Specifications"},
        {"0600", "VII — Reference Catalogs"},
        {"1000", "XI — Implementation Guides"},
        {"2000", "XII — Developer Guides"},
        {"3000", "XIII — Architecture Decision Records"},
        {"9000", "Archive"},
    };

    if (const auto iterator = kKnownParts.find(series); iterator != kKnownParts.end()) {
        return iterator->second;
    }

    for (const auto& component : path) {
        const auto name = component.string();
        if (name.starts_with(series + "-") && name.size() > series.size() + 1) {
            auto suffix = name.substr(series.size() + 1);
            std::replace(suffix.begin(), suffix.end(), '-', ' ');
            return suffix;
        }
    }

    return std::nullopt;
}

DocumentType infer_document_type(
    const std::filesystem::path& path,
    std::string_view title) {
    const auto filename = to_lower(path.filename().string());
    const auto path_string = to_lower(path.generic_string());
    const auto lower_title = to_lower(std::string{title});

    if (path_string.find("reference-catalogs") != std::string::npos ||
        filename.find("catalog") != std::string::npos) {
        return DocumentType::ReferenceCatalog;
    }
    if (path_string.find("implementation-guides") != std::string::npos ||
        filename.find("guide") != std::string::npos) {
        return DocumentType::Guide;
    }
    if (filename.find("rfc") != std::string::npos) {
        return DocumentType::Rfc;
    }
    if (filename == "readme.md") {
        return DocumentType::Readme;
    }
    if (filename.find("whitepaper") != std::string::npos) {
        return DocumentType::Whitepaper;
    }
    if (filename.find("changelog") != std::string::npos) {
        return DocumentType::Changelog;
    }
    if (filename.starts_with("eve-") || lower_title.find("specification") != std::string::npos ||
        filename.find("specification") != std::string::npos) {
        return DocumentType::Specification;
    }
    return DocumentType::Other;
}

void apply_explicit_entry(DocumentMetadata& metadata, const MetadataEntry& entry) {
    const auto key = normalize_key(entry.key);
    const auto value = trim(entry.value);
    if (value.empty()) {
        return;
    }

    if (key == "documentversion" || key == "version") {
        metadata.version = value;
    } else if (key == "authors" || key == "author") {
        metadata.authors = value;
    } else if (key == "status") {
        metadata.status = value;
    } else if (key == "published") {
        metadata.published = value;
    } else if (key == "project") {
        metadata.project = value;
    } else if (key == "specification") {
        metadata.specification = value;
    } else if (key == "part") {
        metadata.part = value;
    }
}

void extract_explicit_metadata(const DocumentModel& model, DocumentMetadata& metadata) {
    metadata.explicit_entries = model.metadata.entries;
    for (const auto& entry : model.metadata.entries) {
        apply_explicit_entry(metadata, entry);
    }

    for (const auto& block : model.blocks) {
        if (block.kind != DocumentBlockKind::Metadata) {
            continue;
        }
        for (const auto& entry : block.metadata.entries) {
            const auto already_present = std::any_of(
                metadata.explicit_entries.begin(),
                metadata.explicit_entries.end(),
                [&](const MetadataEntry& existing) {
                    return normalize_key(existing.key) == normalize_key(entry.key) &&
                           existing.value == entry.value;
                });
            if (!already_present) {
                metadata.explicit_entries.push_back(entry);
            }
            apply_explicit_entry(metadata, entry);
        }
    }
}

const DocumentSection* find_section(const DocumentModel& model, std::string_view title) {
    for (const auto& section : model.sections) {
        if (to_lower(section.title) == to_lower(std::string{title})) {
            return &section;
        }
    }
    return nullptr;
}

bool is_navigation_label(std::string_view text, std::string_view label) {
    return to_lower(strip_bold_markers(std::string{text})) == to_lower(std::string{label});
}

bool is_reserved_navigation_value(std::string_view value) {
    const auto normalized = to_lower(trim(std::string{value}));
    return normalized.empty() || normalized == "reserved" || normalized == "none";
}

void extract_navigation_metadata(const DocumentModel& model, DocumentMetadata& metadata) {
    const auto* navigation = find_section(model, "Navigation");
    if (navigation == nullptr) {
        return;
    }

    enum class PendingField {
        None,
        Previous,
        Next,
        Part,
    };

    PendingField pending = PendingField::None;
    for (std::size_t index = navigation->start_block_index; index < navigation->end_block_index; ++index) {
        const auto& block = model.blocks[index];
        if (block.kind != DocumentBlockKind::Paragraph) {
            continue;
        }

        const auto text = trim(block.paragraph.text);
        if (text.empty()) {
            continue;
        }

        if (is_navigation_label(text, "Previous")) {
            pending = PendingField::Previous;
            continue;
        }
        if (is_navigation_label(text, "Next")) {
            pending = PendingField::Next;
            continue;
        }
        if (is_navigation_label(text, "Part")) {
            pending = PendingField::Part;
            continue;
        }

        switch (pending) {
            case PendingField::Previous:
                if (!is_reserved_navigation_value(text)) {
                    metadata.previous_document = text;
                }
                pending = PendingField::None;
                break;
            case PendingField::Next:
                if (!is_reserved_navigation_value(text)) {
                    metadata.next_document = text;
                }
                pending = PendingField::None;
                break;
            case PendingField::Part:
                if (!metadata.part) {
                    metadata.part = text;
                }
                pending = PendingField::None;
                break;
            case PendingField::None:
                break;
        }
    }
}

void infer_repository_metadata(
    const DocumentModel& model,
    const MetadataExtractionContext& context,
    DocumentMetadata& metadata) {
    if (model.has_source) {
        metadata.file_path = model.source.relative_path;
        if (!model.source.repository_root.empty()) {
            metadata.repository = context.repository_name.empty()
                ? model.source.repository_root.filename().string()
                : context.repository_name;
        }
    } else if (!context.repository_name.empty()) {
        metadata.repository = context.repository_name;
    }

    const std::filesystem::path path =
        metadata.file_path ? *metadata.file_path : std::filesystem::path{};

    if (!metadata.identifier) {
        for (const auto& heading : model.headings) {
            if (auto identifier = find_identifier_in_text(heading.text)) {
                metadata.identifier = identifier;
                break;
            }
        }
    }

    if (!metadata.identifier && !path.empty()) {
        metadata.identifier = infer_identifier_from_path(path);
    }

    if (!metadata.series && !path.empty()) {
        metadata.series = infer_series_from_path(path);
    }

    if (!metadata.part && metadata.series && !path.empty()) {
        metadata.part = infer_part_from_series(*metadata.series, path);
    }

    metadata.document_type = infer_document_type(path, metadata.title);
}

void infer_series_navigation(
    const MetadataExtractionContext& context,
    DocumentMetadata& metadata) {
    if (!metadata.file_path || context.series_documents.size() < 2) {
        return;
    }

    const auto current = metadata.file_path->generic_string();
    std::vector<std::filesystem::path> ordered = context.series_documents;
    std::sort(ordered.begin(), ordered.end(), [](const auto& lhs, const auto& rhs) {
        return lhs.generic_string() < rhs.generic_string();
    });

    const auto iterator = std::find_if(ordered.begin(), ordered.end(), [&](const auto& path) {
        return path.generic_string() == current;
    });
    if (iterator == ordered.end()) {
        return;
    }

    if (iterator != ordered.begin() && !metadata.previous_document) {
        metadata.previous_document = infer_identifier_from_path(*std::prev(iterator))
                                         .value_or(std::prev(iterator)->filename().string());
    }

    if (std::next(iterator) != ordered.end() && !metadata.next_document) {
        metadata.next_document = infer_identifier_from_path(*std::next(iterator))
                                       .value_or(std::next(iterator)->filename().string());
    }
}

DocumentMetadata extract_internal(
    const DocumentModel& model,
    const MetadataExtractionContext& context) {
    DocumentMetadata metadata;
    metadata.title = model.title;

    extract_explicit_metadata(model, metadata);
    extract_navigation_metadata(model, metadata);
    infer_repository_metadata(model, context, metadata);
    infer_series_navigation(context, metadata);

    return metadata;
}

}  // namespace

DocumentMetadata MetadataExtractor::extract(const DocumentModel& model) {
    return extract_internal(model, MetadataExtractionContext{});
}

DocumentMetadata MetadataExtractor::extract(
    const DocumentModel& model,
    const MetadataExtractionContext& context) {
    return extract_internal(model, context);
}

}  // namespace eve
