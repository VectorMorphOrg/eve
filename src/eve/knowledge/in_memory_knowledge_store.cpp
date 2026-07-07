#include "eve/knowledge/in_memory_knowledge_store.hpp"

#include <algorithm>

namespace eve {

InMemoryKnowledgeStore::InMemoryKnowledgeStore(std::vector<KnowledgeObject> objects)
    : objects_(std::move(objects)) {}

void InMemoryKnowledgeStore::load(std::vector<KnowledgeObject> objects) {
    const std::scoped_lock lock(mutex_);
    objects_ = std::move(objects);
}

void InMemoryKnowledgeStore::add(KnowledgeObject object) {
    const std::scoped_lock lock(mutex_);
    objects_.push_back(std::move(object));
}

std::optional<KnowledgeObject> InMemoryKnowledgeStore::get_by_id(
    const KnowledgeObjectId& id) const {
    const std::scoped_lock lock(mutex_);
    for (const auto& object : objects_) {
        if (object.identity.id.value == id.value) {
            return object;
        }
    }
    return std::nullopt;
}

std::vector<KnowledgeObject> InMemoryKnowledgeStore::list_by_repository(
    std::string_view repository) const {
    const std::scoped_lock lock(mutex_);
    std::vector<KnowledgeObject> results;
    for (const auto& object : objects_) {
        if (object.identity.repository == repository) {
            results.push_back(object);
        }
    }
    return results;
}

std::vector<SearchResult> InMemoryKnowledgeStore::search(
    std::string_view query,
    std::optional<std::string_view> repository) const {
    const std::scoped_lock lock(mutex_);
    std::vector<SearchResult> results;

    for (const auto& object : objects_) {
        if (repository && object.identity.repository != *repository) {
            continue;
        }

        double score = 0.0;
        if (object.identity.title.find(query) != std::string::npos) {
            score += 3.0;
        }
        if (object.search.excerpt.find(query) != std::string::npos ||
            object.search.search_text.find(query) != std::string::npos) {
            score += 1.0;
        }
        for (const auto& section : object.content.sections) {
            if (section.content.find(query) != std::string::npos) {
                score += 0.5;
            }
        }
        if (score > 0.0) {
            score += 100.0 / static_cast<double>(object.search.priority_rank);
            results.push_back(SearchResult{.object = object, .score = score});
        }
    }

    std::sort(results.begin(), results.end(), [](const SearchResult& lhs, const SearchResult& rhs) {
        return lhs.score > rhs.score;
    });
    return results;
}

std::size_t InMemoryKnowledgeStore::document_count() const {
    const std::scoped_lock lock(mutex_);
    return objects_.size();
}

}  // namespace eve
