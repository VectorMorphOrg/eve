#include "eve/services/service_implementations.hpp"

#include <algorithm>

namespace eve::services {
namespace {

void remove_oldest_complete_pair(context::ConversationContext& conversation) {
    if (conversation.recent_messages.size() < 2) {
        return;
    }
    conversation.recent_messages.erase(
        conversation.recent_messages.begin(),
        conversation.recent_messages.begin() + 2);
}

}  // namespace

ConversationMemoryService::ConversationMemoryService(
    std::size_t max_recent_messages,
    std::size_t max_conversation_chars)
    : max_recent_messages_(std::max<std::size_t>(1, max_recent_messages)),
      max_conversation_chars_(std::max<std::size_t>(1, max_conversation_chars)) {}

std::size_t ConversationMemoryService::conversation_char_count(
    const context::ConversationContext& conversation) {
    std::size_t total = 0;
    for (const auto& turn : conversation.recent_messages) {
        total += turn.content.size();
    }
    return total;
}

void ConversationMemoryService::apply_retention(
    context::ConversationContext& conversation) const {
    while (conversation.recent_messages.size() > max_recent_messages_ &&
           conversation.recent_messages.size() >= 2) {
        remove_oldest_complete_pair(conversation);
    }

    while (conversation_char_count(conversation) > max_conversation_chars_ &&
           conversation.recent_messages.size() > 2) {
        remove_oldest_complete_pair(conversation);
    }

    // Newest complete pair is retained even when it alone exceeds the character
    // budget; dropping it would leave an empty session after a long exchange.
}

context::ConversationContext ConversationMemoryService::load(
    std::string_view session_id) const {
    if (session_id.empty()) {
        return {};
    }

    const std::lock_guard lock(mutex_);
    const auto it = sessions_.find(std::string(session_id));
    if (it == sessions_.end()) {
        return {};
    }
    return it->second;
}

void ConversationMemoryService::append(
    std::string_view session_id,
    std::string_view user_text,
    std::string_view assistant_text) {
    if (session_id.empty()) {
        return;
    }

    const std::lock_guard lock(mutex_);
    auto& conversation = sessions_[std::string(session_id)];
    conversation.recent_messages.push_back(context::ConversationTurn{
        .role = context::ConversationRole::User,
        .content = std::string(user_text),
    });
    conversation.recent_messages.push_back(context::ConversationTurn{
        .role = context::ConversationRole::Assistant,
        .content = std::string(assistant_text),
    });
    apply_retention(conversation);
}

void ConversationMemoryService::reset(std::string_view session_id) {
    if (session_id.empty()) {
        return;
    }

    const std::lock_guard lock(mutex_);
    sessions_.erase(std::string(session_id));
}

}  // namespace eve::services
