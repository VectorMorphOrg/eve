#pragma once

#include "eve/knowledge/knowledge_object.hpp"

#include <memory>
#include <mutex>
#include <vector>

namespace eve {

class InMemoryKnowledgeStore final : public IKnowledgeStore {
public:
    explicit InMemoryKnowledgeStore(std::vector<KnowledgeObject> objects = {});

    void load(std::vector<KnowledgeObject> objects);
    void add(KnowledgeObject object);

    [[nodiscard]] std::optional<KnowledgeObject> get_by_id(
        const KnowledgeObjectId& id) const override;
    [[nodiscard]] std::vector<KnowledgeObject> list_by_repository(
        std::string_view repository) const override;
    [[nodiscard]] std::vector<SearchResult> search(
        std::string_view query,
        std::optional<std::string_view> repository = std::nullopt) const override;
    [[nodiscard]] std::size_t document_count() const override;

private:
    mutable std::mutex mutex_;
    std::vector<KnowledgeObject> objects_;
};

}  // namespace eve
