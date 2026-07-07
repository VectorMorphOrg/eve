#pragma once

#include "eve/core/types.hpp"

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace eve {

enum class RepositoryFileType {
    Markdown,
    CSource,
    CppSource,
    Header,
    Test,
    Example,
    Configuration,
};

[[nodiscard]] std::string to_string(RepositoryFileType type);

struct RepositoryResource {
    std::filesystem::path absolute_path;
    std::filesystem::path relative_path;
    RepositoryFileType file_type{RepositoryFileType::Markdown};
    std::filesystem::path repository_root;
    TimePoint last_modified{};
    std::uintmax_t file_size{0};
};

struct RepositoryIndex {
    std::filesystem::path repository_root;
    std::string repository_name;
    std::vector<RepositoryResource> resources;
};

}  // namespace eve
