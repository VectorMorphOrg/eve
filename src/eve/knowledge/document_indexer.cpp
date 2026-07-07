#include "eve/knowledge/knowledge_object.hpp"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace eve {
namespace {

std::string to_lower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

bool contains_case_insensitive(std::string_view haystack, std::string_view needle) {
    return to_lower(std::string{haystack}).find(to_lower(std::string{needle})) != std::string::npos;
}

std::string trim(std::string value) {
    const auto not_space = [](unsigned char ch) { return !std::isspace(ch); };
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), not_space));
    value.erase(std::find_if(value.rbegin(), value.rend(), not_space).base(), value.end());
    return value;
}

}  // namespace

std::string knowledge_object_id_from_path(
    const std::filesystem::path& path,
    std::string_view repository_name) {
    const auto relative = path.generic_string();
    return std::format("{}:{}", repository_name, relative);
}

DocumentType DocumentIndexer::classify_document(
    const std::filesystem::path& path,
    std::string_view title) {
    const auto filename = path.filename().string();
    const auto lower_title = to_lower(std::string{title});
    const auto lower_name = to_lower(filename);

    if (lower_name.find("specification") != std::string::npos ||
        lower_name.starts_with("eve-0") || lower_title.find("specification") != std::string::npos) {
        return DocumentType::Specification;
    }
    if (lower_name.find("catalog") != std::string::npos) {
        return DocumentType::ReferenceCatalog;
    }
    if (lower_name.find("rfc") != std::string::npos) {
        return DocumentType::Rfc;
    }
    if (lower_name == "readme.md") {
        return DocumentType::Readme;
    }
    if (lower_name.find("roadmap") != std::string::npos ||
        lower_name.find("guide") != std::string::npos) {
        return DocumentType::Guide;
    }
    if (lower_name.find("changelog") != std::string::npos) {
        return DocumentType::Changelog;
    }
    if (lower_name.find("whitepaper") != std::string::npos) {
        return DocumentType::Whitepaper;
    }
    return DocumentType::Other;
}

int DocumentIndexer::priority_for(DocumentType type) {
    switch (type) {
        case DocumentType::Specification:
            return 10;
        case DocumentType::Rfc:
            return 20;
        case DocumentType::ReferenceCatalog:
            return 30;
        case DocumentType::Whitepaper:
            return 40;
        case DocumentType::Guide:
            return 50;
        case DocumentType::Readme:
            return 60;
        case DocumentType::Changelog:
            return 70;
        case DocumentType::Other:
            return 100;
    }
    return 100;
}

KnowledgeObject DocumentIndexer::parse_markdown_file(
    const std::filesystem::path& path,
    std::string_view repository_name,
    std::uint32_t sequence) {
    KnowledgeObject object;
    object.identifier = generate_knowledge_object_id(sequence);
    object.repository = std::string{repository_name};
    object.source_path = path;
    object.last_modified = "unknown";

    std::ifstream input(path, std::ios::binary);
    std::ostringstream buffer;
    buffer << input.rdbuf();
    const auto content = buffer.str();

    std::string title = path.stem().string();
    std::istringstream line_stream(content);
    std::string line_str;
    while (std::getline(line_stream, line_str)) {
        if (line_str.starts_with("# ")) {
            title = trim(line_str.substr(2));
            break;
        }
    }

    object.title = title;
    object.document_type = classify_document(path, title);
    object.priority_rank = priority_for(object.document_type);
    object.excerpt = content.substr(0, std::min(content.size(), std::size_t{512}));
    object.sections.push_back(KnowledgeSection{
        .identifier = "body",
        .title = title,
        .content = content,
        .references = {},
    });

    if (auto last_write = std::filesystem::last_write_time(path); last_write != std::filesystem::file_time_type{}) {
        const auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            last_write - std::filesystem::file_time_type::clock::now() +
            std::chrono::system_clock::now());
        const auto time = std::chrono::system_clock::to_time_t(sctp);
        std::tm tm{};
#if defined(_WIN32)
        gmtime_s(&tm, &time);
#else
        gmtime_r(&time, &tm);
#endif
        std::ostringstream stream;
        stream << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
        object.last_modified = stream.str();
    }

    return object;
}

std::vector<KnowledgeObject> DocumentIndexer::index_directory(
    const std::filesystem::path& root,
    std::string_view repository_name) {
    std::vector<KnowledgeObject> objects;
    if (!std::filesystem::exists(root)) {
        return objects;
    }

    std::uint32_t sequence = 1;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(root)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        const auto extension = entry.path().extension().string();
        if (extension != ".md" && extension != ".markdown") {
            continue;
        }
        objects.push_back(parse_markdown_file(entry.path(), repository_name, sequence++));
    }

    std::sort(objects.begin(), objects.end(), [](const KnowledgeObject& lhs, const KnowledgeObject& rhs) {
        return lhs.priority_rank < rhs.priority_rank;
    });
    return objects;
}

}  // namespace eve
