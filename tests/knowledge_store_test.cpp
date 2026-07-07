#include "eve/knowledge/knowledge_object.hpp"
#include "eve/knowledge/in_memory_knowledge_store.hpp"

#include <gtest/gtest.h>

#ifndef EVE_PROJECT_ROOT
#define EVE_PROJECT_ROOT "."
#endif

namespace eve {
namespace {

TEST(KnowledgeStoreTest, IndexesProjectDocs) {
    const auto objects = DocumentIndexer::index_directory(
        std::filesystem::path{EVE_PROJECT_ROOT} / "docs",
        "eve");
    ASSERT_FALSE(objects.empty());

    InMemoryKnowledgeStore store(objects);
    const auto results = store.search("Platform Request");
    EXPECT_FALSE(results.empty());
}

}  // namespace
}  // namespace eve
