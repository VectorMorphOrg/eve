#include "eve/knowledge/repository_discovery.hpp"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <system_error>

namespace eve {
namespace {

std::string to_lower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

bool has_extension_insensitive(
    const std::filesystem::path& path,
    std::initializer_list<std::string_view> extensions) {
    const auto extension = to_lower(path.extension().string());
    for (const auto candidate : extensions) {
        if (extension == candidate) {
            return true;
        }
    }
    return false;
}

bool path_contains_directory(
    const std::filesystem::path& path,
    std::initializer_list<std::string_view> directory_names) {
    for (const auto& part : path) {
        const auto lower_part = to_lower(part.string());
        for (const auto directory_name : directory_names) {
            if (lower_part == directory_name) {
                return true;
            }
        }
    }
    return false;
}

bool is_test_file(const std::filesystem::path& relative_path) {
    if (path_contains_directory(relative_path, {"tests", "test"})) {
        return true;
    }

    const auto filename = to_lower(relative_path.filename().string());
    if (filename.starts_with("test_")) {
        return true;
    }
    if (filename.ends_with("_test.cpp") || filename.ends_with("_test.cc") ||
        filename.ends_with("_test.cxx") || filename.ends_with("_test.c")) {
        return true;
    }
    return false;
}

bool is_example_file(const std::filesystem::path& relative_path) {
    return path_contains_directory(relative_path, {"examples", "example"});
}

bool is_configuration_file(const std::filesystem::path& path) {
    if (path.filename() == "CMakeLists.txt") {
        return true;
    }
    return has_extension_insensitive(
        path,
        {".json", ".yaml", ".yml", ".toml", ".ini", ".cfg", ".conf", ".cmake"});
}

std::optional<RepositoryFileType> classify_by_extension(const std::filesystem::path& path) {
    if (has_extension_insensitive(path, {".md", ".markdown"})) {
        return RepositoryFileType::Markdown;
    }
    if (has_extension_insensitive(path, {".h", ".hpp", ".hh", ".hxx", ".h++"})) {
        return RepositoryFileType::Header;
    }
    if (has_extension_insensitive(path, {".c"})) {
        return RepositoryFileType::CSource;
    }
    if (has_extension_insensitive(path, {".cpp", ".cc", ".cxx", ".c++"})) {
        return RepositoryFileType::CppSource;
    }
    if (is_configuration_file(path)) {
        return RepositoryFileType::Configuration;
    }
    return std::nullopt;
}

TimePoint to_time_point(std::filesystem::file_time_type timestamp) {
    using namespace std::chrono;
    const auto system_now = system_clock::now();
    const auto file_now = file_clock::now();
    const auto delta = timestamp - file_now;
    return system_clock::time_point{duration_cast<system_clock::duration>(delta) + system_now.time_since_epoch()};
}

bool should_skip_directory(
    const std::filesystem::path& directory_name,
    const RepositoryDiscovery::Options& options) {
    return options.excluded_directory_names.contains(directory_name.string());
}

}  // namespace

RepositoryDiscovery::Options::Options()
    : excluded_directory_names{
          ".git",
          ".vs",
          ".cursor",
          "build",
          "CMakeFiles",
          "_deps",
          "node_modules",
          "out",
      } {}

const RepositoryDiscovery::Options& RepositoryDiscovery::default_options() {
    static const Options options;
    return options;
}

std::string to_string(RepositoryFileType type) {
    switch (type) {
        case RepositoryFileType::Markdown:
            return "markdown";
        case RepositoryFileType::CSource:
            return "c_source";
        case RepositoryFileType::CppSource:
            return "cpp_source";
        case RepositoryFileType::Header:
            return "header";
        case RepositoryFileType::Test:
            return "test";
        case RepositoryFileType::Example:
            return "example";
        case RepositoryFileType::Configuration:
            return "configuration";
    }
    return "unknown";
}

std::optional<RepositoryFileType> RepositoryDiscovery::classify(
    const std::filesystem::path& path,
    const std::filesystem::path& repository_root) {
    std::error_code error;
    if (!std::filesystem::is_regular_file(path, error) || error) {
        return std::nullopt;
    }

    const auto relative_path = std::filesystem::relative(path, repository_root, error);
    if (error || relative_path.empty()) {
        return std::nullopt;
    }

    if (is_example_file(relative_path)) {
        return RepositoryFileType::Example;
    }
    if (is_test_file(relative_path)) {
        return RepositoryFileType::Test;
    }

    return classify_by_extension(path);
}

RepositoryIndex RepositoryDiscovery::discover(
    const std::filesystem::path& repository_root,
    std::string_view repository_name,
    const Options& options) {
    RepositoryIndex index;
    std::error_code error;
    index.repository_root = std::filesystem::weakly_canonical(repository_root, error);
    if (error || index.repository_root.empty()) {
        index.repository_root = repository_root;
    }
    index.repository_name = std::string{repository_name};

    std::error_code iterator_error;
    for (std::filesystem::recursive_directory_iterator iterator(
             index.repository_root,
             std::filesystem::directory_options::skip_permission_denied,
             iterator_error);
         iterator != std::filesystem::recursive_directory_iterator{};
         iterator.increment(iterator_error)) {
        if (iterator_error) {
            break;
        }

        const auto& entry = *iterator;
        if (entry.is_directory(iterator_error)) {
            if (iterator_error) {
                continue;
            }
            if (should_skip_directory(entry.path().filename(), options)) {
                iterator.disable_recursion_pending();
            }
            continue;
        }

        if (!entry.is_regular_file(iterator_error) || iterator_error) {
            continue;
        }

        const auto absolute_path = entry.path();
        const auto relative_path =
            std::filesystem::relative(absolute_path, index.repository_root, iterator_error);
        if (iterator_error || relative_path.empty()) {
            continue;
        }

        const auto file_type = classify(absolute_path, index.repository_root);
        if (!file_type) {
            continue;
        }

        std::uintmax_t file_size = 0;
        TimePoint last_modified{};
        file_size = entry.file_size(iterator_error);
        if (!iterator_error) {
            last_modified = to_time_point(entry.last_write_time(iterator_error));
        }

        index.resources.push_back(RepositoryResource{
            .absolute_path = absolute_path,
            .relative_path = relative_path,
            .file_type = *file_type,
            .repository_root = index.repository_root,
            .last_modified = last_modified,
            .file_size = file_size,
        });
    }

    std::sort(
        index.resources.begin(),
        index.resources.end(),
        [](const RepositoryResource& lhs, const RepositoryResource& rhs) {
            return lhs.relative_path.generic_string() < rhs.relative_path.generic_string();
        });

    return index;
}

}  // namespace eve
