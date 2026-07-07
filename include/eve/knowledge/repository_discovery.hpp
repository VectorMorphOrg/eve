#pragma once

#include "eve/knowledge/repository_index.hpp"

#include <filesystem>
#include <optional>
#include <set>
#include <string>
#include <string_view>

namespace eve {

class RepositoryDiscovery {
public:
    struct Options {
        std::set<std::string> excluded_directory_names;

        Options();
    };

    [[nodiscard]] static RepositoryIndex discover(
        const std::filesystem::path& repository_root,
        std::string_view repository_name = {},
        const Options& options = default_options());

    [[nodiscard]] static const Options& default_options();

    [[nodiscard]] static std::optional<RepositoryFileType> classify(
        const std::filesystem::path& path,
        const std::filesystem::path& repository_root);
};

}  // namespace eve
