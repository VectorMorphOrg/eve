#include "eve/context/context_package.hpp"
#include "eve/core/platform_request.hpp"
#include "eve/services/service_implementations.hpp"

#include <gtest/gtest.h>
#include <string>

namespace eve {
namespace {

TEST(ConversationContextModelTest, CreateAttachesStructuredTurns) {
    const auto request = PlatformRequest::create(
        CapabilityId{"CAP-0102"},
        InterfaceType::Internal);

    context::ConversationContext conversation{
        .session_summary = std::nullopt,
        .recent_messages =
            {
                context::ConversationTurn{
                    .role = context::ConversationRole::User,
                    .content = "first",
                },
                context::ConversationTurn{
                    .role = context::ConversationRole::Assistant,
                    .content = "second",
                },
            },
    };

    const auto package = context::ContextPackage::create(
        request,
        {},
        {},
        {},
        context::ContextConstraints{},
        context::SystemInstructions{},
        context::PackageDiagnostics{{}, {"missing"}, {}},
        conversation);

    ASSERT_TRUE(package.conversation().has_value());
    ASSERT_EQ(package.conversation()->recent_messages.size(), 2U);
    EXPECT_EQ(
        package.conversation()->recent_messages[0].role,
        context::ConversationRole::User);
    EXPECT_EQ(package.conversation()->recent_messages[0].content, "first");
    EXPECT_EQ(
        package.conversation()->recent_messages[1].role,
        context::ConversationRole::Assistant);
    EXPECT_EQ(package.conversation()->recent_messages[1].content, "second");
}

TEST(ConversationMemoryServiceTest, NewSessionLoadsEmpty) {
    services::ConversationMemoryService memory;
    const auto loaded = memory.load("session-new");
    EXPECT_TRUE(loaded.recent_messages.empty());
    EXPECT_FALSE(loaded.session_summary.has_value());
}

TEST(ConversationMemoryServiceTest, AppendCreatesSessionAndOrdersTurns) {
    services::ConversationMemoryService memory;
    memory.append("session-a", "hello", "hi there");

    const auto loaded = memory.load("session-a");
    ASSERT_EQ(loaded.recent_messages.size(), 2U);
    EXPECT_EQ(loaded.recent_messages[0].role, context::ConversationRole::User);
    EXPECT_EQ(loaded.recent_messages[0].content, "hello");
    EXPECT_EQ(loaded.recent_messages[1].role, context::ConversationRole::Assistant);
    EXPECT_EQ(loaded.recent_messages[1].content, "hi there");
}

TEST(ConversationMemoryServiceTest, MultipleAppendsPreserveChronologicalOrder) {
    services::ConversationMemoryService memory;
    memory.append("session-a", "one", "reply-one");
    memory.append("session-a", "two", "reply-two");
    memory.append("session-a", "three", "reply-three");

    const auto loaded = memory.load("session-a");
    ASSERT_EQ(loaded.recent_messages.size(), 6U);
    EXPECT_EQ(loaded.recent_messages[0].content, "one");
    EXPECT_EQ(loaded.recent_messages[1].content, "reply-one");
    EXPECT_EQ(loaded.recent_messages[2].content, "two");
    EXPECT_EQ(loaded.recent_messages[3].content, "reply-two");
    EXPECT_EQ(loaded.recent_messages[4].content, "three");
    EXPECT_EQ(loaded.recent_messages[5].content, "reply-three");
}

TEST(ConversationMemoryServiceTest, IndependentSessionsRemainIndependent) {
    services::ConversationMemoryService memory;
    memory.append("session-a", "a-user", "a-assistant");
    memory.append("session-b", "b-user", "b-assistant");

    const auto a = memory.load("session-a");
    const auto b = memory.load("session-b");
    ASSERT_EQ(a.recent_messages.size(), 2U);
    ASSERT_EQ(b.recent_messages.size(), 2U);
    EXPECT_EQ(a.recent_messages[0].content, "a-user");
    EXPECT_EQ(b.recent_messages[0].content, "b-user");
}

TEST(ConversationMemoryServiceTest, ResetClearsSession) {
    services::ConversationMemoryService memory;
    memory.append("session-a", "hello", "world");
    memory.reset("session-a");

    const auto loaded = memory.load("session-a");
    EXPECT_TRUE(loaded.recent_messages.empty());
}

TEST(ConversationMemoryServiceTest, EmptySessionIdDoesNotCreateStoredMemory) {
    services::ConversationMemoryService memory;
    memory.append("", "user", "assistant");
    memory.reset("");

    EXPECT_TRUE(memory.load("").recent_messages.empty());
    EXPECT_TRUE(memory.load("other").recent_messages.empty());
}

TEST(ConversationMemoryServiceTest, MaximumMessageRetentionRemovesOldestPairs) {
    services::ConversationMemoryService memory(/*max_recent_messages=*/4);

    memory.append("session-a", "u1", "a1");
    memory.append("session-a", "u2", "a2");
    memory.append("session-a", "u3", "a3");

    const auto loaded = memory.load("session-a");
    ASSERT_EQ(loaded.recent_messages.size(), 4U);
    EXPECT_EQ(loaded.recent_messages[0].content, "u2");
    EXPECT_EQ(loaded.recent_messages[1].content, "a2");
    EXPECT_EQ(loaded.recent_messages[2].content, "u3");
    EXPECT_EQ(loaded.recent_messages[3].content, "a3");
    EXPECT_EQ(loaded.recent_messages[0].role, context::ConversationRole::User);
    EXPECT_EQ(loaded.recent_messages[1].role, context::ConversationRole::Assistant);
}

TEST(ConversationMemoryServiceTest, MaximumCharacterRetentionRemovesOldestPairs) {
    services::ConversationMemoryService memory(
        /*max_recent_messages=*/12,
        /*max_conversation_chars=*/20);

    memory.append("session-a", "1234567890", "abcdefghij");  // 20 chars
    memory.append("session-a", "xyz", "uvw");               // would exceed unless trim

    const auto loaded = memory.load("session-a");
    ASSERT_EQ(loaded.recent_messages.size(), 2U);
    EXPECT_EQ(loaded.recent_messages[0].content, "xyz");
    EXPECT_EQ(loaded.recent_messages[1].content, "uvw");
}

TEST(ConversationMemoryServiceTest, RetentionNeverSplitsUserAssistantPair) {
    services::ConversationMemoryService memory(/*max_recent_messages=*/3);

    memory.append("session-a", "u1", "a1");
    memory.append("session-a", "u2", "a2");

    const auto loaded = memory.load("session-a");
    // max_recent_messages=3 would force a split if trimming by single messages;
    // retention removes a complete pair, leaving the newest pair only.
    ASSERT_EQ(loaded.recent_messages.size(), 2U);
    EXPECT_EQ(loaded.recent_messages[0].role, context::ConversationRole::User);
    EXPECT_EQ(loaded.recent_messages[0].content, "u2");
    EXPECT_EQ(loaded.recent_messages[1].role, context::ConversationRole::Assistant);
    EXPECT_EQ(loaded.recent_messages[1].content, "a2");
}

TEST(ConversationMemoryServiceTest, OversizedNewestPairIsRetained) {
    services::ConversationMemoryService memory(
        /*max_recent_messages=*/12,
        /*max_conversation_chars=*/10);

    memory.append(
        "session-a",
        "this-user-turn-is-already-too-long",
        "this-assistant-turn-is-also-too-long");

    const auto loaded = memory.load("session-a");
    // Documented invariant: the newest complete pair is retained even when it
    // alone exceeds the character budget.
    ASSERT_EQ(loaded.recent_messages.size(), 2U);
    EXPECT_EQ(loaded.recent_messages[0].content, "this-user-turn-is-already-too-long");
    EXPECT_EQ(
        loaded.recent_messages[1].content,
        "this-assistant-turn-is-also-too-long");
}

}  // namespace
}  // namespace eve
