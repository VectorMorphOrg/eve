#include "eve/knowledge/repository_discovery.hpp"

#include <algorithm>
#include <fstream>
#include <format>
#include <gtest/gtest.h>

namespace eve {
namespace {

std::filesystem::path create_temp_repository() {
    const auto base =
        std::filesystem::temp_directory_path() / std::format("eve-repo-discovery-{}", ::testing::UnitTest::GetInstance()->random_seed());
    std::filesystem::create_directories(base);
    return base;
}

void write_file(const std::filesystem::path& path, std::string_view content) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream stream{path, std::ios::binary};
    stream << content;
    ASSERT_TRUE(stream.good());
}

RepositoryResource find_resource(const RepositoryIndex& index, std::string_view relative_path) {
    const auto iterator = std::find_if(
        index.resources.begin(),
        index.resources.end(),
        [relative_path](const RepositoryResource& resource) {
            return resource.relative_path.generic_string() == relative_path;
        });
    EXPECT_NE(iterator, index.resources.end());
    return *iterator;
}

TEST(RepositoryDiscoveryTest, DiscoversMarkdownFiles) {
    const auto root = create_temp_repository();
    write_file(root / "docs" / "guide.md", "# Guide");

    const auto index = RepositoryDiscovery::discover(root, "test-repo");
    const auto resource = find_resource(index, "docs/guide.md");

    EXPECT_EQ(resource.file_type, RepositoryFileType::Markdown);
    EXPECT_EQ(resource.repository_root, index.repository_root);
    EXPECT_GT(resource.file_size, 0U);
    EXPECT_NE(resource.last_modified.time_since_epoch().count(), 0);
}

TEST(RepositoryDiscoveryTest, DiscoversSourceAndHeaderFiles) {
    const auto root = create_temp_repository();
    write_file(root / "src" / "module.c", "int main() {}");
    write_file(root / "src" / "module.cpp", "int run() {}");
    write_file(root / "include" / "module.hpp", "#pragma once");

    const auto index = RepositoryDiscovery::discover(root, "test-repo");

    EXPECT_EQ(find_resource(index, "src/module.c").file_type, RepositoryFileType::CSource);
    EXPECT_EQ(find_resource(index, "src/module.cpp").file_type, RepositoryFileType::CppSource);
    EXPECT_EQ(find_resource(index, "include/module.hpp").file_type, RepositoryFileType::Header);
}

TEST(RepositoryDiscoveryTest, ClassifiesTestsAndExamplesByLocation) {
    const auto root = create_temp_repository();
    write_file(root / "tests" / "platform_test.cpp", "TEST(Platform, Works) {}");
    write_file(root / "examples" / "demo.cpp", "int main() {}");

    const auto index = RepositoryDiscovery::discover(root, "test-repo");

    EXPECT_EQ(find_resource(index, "tests/platform_test.cpp").file_type, RepositoryFileType::Test);
    EXPECT_EQ(find_resource(index, "examples/demo.cpp").file_type, RepositoryFileType::Example);
}

TEST(RepositoryDiscoveryTest, ClassifiesConfigurationFiles) {
    const auto root = create_temp_repository();
    write_file(root / "config" / "platform.json", R"({"version":"1.0.0"})");
    write_file(root / "CMakeLists.txt", "project(example)");

    const auto index = RepositoryDiscovery::discover(root, "test-repo");

    EXPECT_EQ(find_resource(index, "config/platform.json").file_type, RepositoryFileType::Configuration);
    EXPECT_EQ(find_resource(index, "CMakeLists.txt").file_type, RepositoryFileType::Configuration);
}

TEST(RepositoryDiscoveryTest, SkipsUnsupportedFiles) {
    const auto root = create_temp_repository();
    write_file(root / "README.md", "# Readme");
    write_file(root / "notes.txt", "unsupported");

    const auto index = RepositoryDiscovery::discover(root, "test-repo");
    EXPECT_EQ(index.resources.size(), 1U);
    EXPECT_EQ(index.resources.front().relative_path.generic_string(), "README.md");
}

TEST(RepositoryDiscoveryTest, ExcludesIgnoredDirectories) {
    const auto root = create_temp_repository();
    write_file(root / "src" / "main.cpp", "int main() {}");
    write_file(root / "build" / "generated.cpp", "int generated() {}");
    write_file(root / ".git" / "config.cpp", "int hidden() {}");

    const auto index = RepositoryDiscovery::discover(root, "test-repo");
    EXPECT_EQ(index.resources.size(), 1U);
    EXPECT_EQ(index.resources.front().relative_path.generic_string(), "src/main.cpp");
}

TEST(RepositoryDiscoveryTest, ReturnsDeterministicOrdering) {
    const auto root = create_temp_repository();
    write_file(root / "z.md", "z");
    write_file(root / "a.md", "a");
    write_file(root / "m.md", "m");

    const auto first = RepositoryDiscovery::discover(root, "test-repo");
    const auto second = RepositoryDiscovery::discover(root, "test-repo");

    ASSERT_EQ(first.resources.size(), 3U);
    EXPECT_EQ(first.resources[0].relative_path.generic_string(), "a.md");
    EXPECT_EQ(first.resources[1].relative_path.generic_string(), "m.md");
    EXPECT_EQ(first.resources[2].relative_path.generic_string(), "z.md");

    for (std::size_t index = 0; index < first.resources.size(); ++index) {
        EXPECT_EQ(
            first.resources[index].relative_path,
            second.resources[index].relative_path);
        EXPECT_EQ(first.resources[index].file_type, second.resources[index].file_type);
    }
}

TEST(RepositoryDiscoveryTest, StoresAbsoluteAndRelativePaths) {
    const auto root = create_temp_repository();
    write_file(root / "docs" / "spec.md", "# Spec");

    const auto index = RepositoryDiscovery::discover(root, "test-repo");
    const auto resource = find_resource(index, "docs/spec.md");

    EXPECT_TRUE(resource.absolute_path.is_absolute());
    EXPECT_EQ(resource.absolute_path, index.repository_root / resource.relative_path);
}

#ifndef EVE_PROJECT_ROOT
#define EVE_PROJECT_ROOT "."
#endif

TEST(RepositoryDiscoveryTest, DiscoversProjectDocumentation) {
    const auto index = RepositoryDiscovery::discover(
        std::filesystem::path{EVE_PROJECT_ROOT} / "docs",
        "eve-docs");

    EXPECT_FALSE(index.resources.empty());
    EXPECT_TRUE(std::ranges::any_of(index.resources, [](const RepositoryResource& resource) {
        return resource.file_type == RepositoryFileType::Markdown;
    }));
}

}  // namespace
}  // namespace eve
