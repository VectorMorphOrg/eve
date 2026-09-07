#include "eve/services/service_implementations.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#ifndef EVE_PROJECT_ROOT
#define EVE_PROJECT_ROOT "."
#endif

namespace eve {
namespace {

std::filesystem::path write_temp_config(std::string_view contents) {
    const auto path =
        std::filesystem::temp_directory_path() / "eve_memory_config_test_platform.json";
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    output << contents;
    return path;
}

TEST(ConfigurationMemoryTest, AbsentMemoryKeysPreserveServiceDefaults) {
    const auto path = write_temp_config(R"({
  "platform_version": "0.1.0",
  "specification_version": "1.0.0",
  "default_repository": "eve",
  "active_ai_provider": "AI-0000",
  "context_limit_chars": 32000
})");

    const services::ConfigurationService configuration(path);
    EXPECT_EQ(
        configuration.memory_max_recent_messages(),
        services::ConversationMemoryService::default_max_recent_messages);
    EXPECT_EQ(
        configuration.memory_max_conversation_chars(),
        services::ConversationMemoryService::default_max_conversation_chars);
    EXPECT_FALSE(configuration.get("memory_max_recent_messages").has_value());
    EXPECT_FALSE(configuration.get("memory_max_conversation_chars").has_value());
}

TEST(ConfigurationMemoryTest, ConfiguredRetentionValuesAreExposed) {
    const auto path = write_temp_config(R"({
  "platform_version": "0.1.0",
  "specification_version": "1.0.0",
  "default_repository": "eve",
  "active_ai_provider": "AI-0000",
  "context_limit_chars": 32000,
  "memory_max_recent_messages": 4,
  "memory_max_conversation_chars": 20
})");

    const services::ConfigurationService configuration(path);
    EXPECT_EQ(configuration.memory_max_recent_messages(), 4U);
    EXPECT_EQ(configuration.memory_max_conversation_chars(), 20U);
}

TEST(ConfigurationMemoryTest, InvalidMemoryValuesFallBackToDefaults) {
    const auto path = write_temp_config(R"({
  "platform_version": "0.1.0",
  "specification_version": "1.0.0",
  "default_repository": "eve",
  "active_ai_provider": "AI-0000",
  "context_limit_chars": 32000,
  "memory_max_recent_messages": not-a-number,
  "memory_max_conversation_chars": also-bad
})");

    const services::ConfigurationService configuration(path);
    EXPECT_EQ(
        configuration.memory_max_recent_messages(),
        services::ConversationMemoryService::default_max_recent_messages);
    EXPECT_EQ(
        configuration.memory_max_conversation_chars(),
        services::ConversationMemoryService::default_max_conversation_chars);
}

TEST(ConfigurationMemoryTest, ExistingPlatformJsonLoadsWithoutMemoryKeys) {
    const services::ConfigurationService configuration(
        std::filesystem::path(EVE_PROJECT_ROOT) / "config/platform.json");
    EXPECT_EQ(configuration.context_limit_chars(), 32000U);
    EXPECT_EQ(
        configuration.memory_max_recent_messages(),
        services::ConversationMemoryService::default_max_recent_messages);
    EXPECT_EQ(
        configuration.memory_max_conversation_chars(),
        services::ConversationMemoryService::default_max_conversation_chars);
}

TEST(ConfigurationMemoryTest, ConfiguredLimitsDriveRuntimeRetention) {
    const auto path = write_temp_config(R"({
  "platform_version": "0.1.0",
  "specification_version": "1.0.0",
  "default_repository": "eve",
  "active_ai_provider": "AI-0000",
  "context_limit_chars": 32000,
  "memory_max_recent_messages": 4,
  "memory_max_conversation_chars": 8000
})");

    const services::ConfigurationService configuration(path);
    services::ConversationMemoryService memory(
        configuration.memory_max_recent_messages(),
        configuration.memory_max_conversation_chars());

    memory.append("session", "u1", "a1");
    memory.append("session", "u2", "a2");
    memory.append("session", "u3", "a3");

    const auto loaded = memory.load("session");
    ASSERT_EQ(loaded.recent_messages.size(), 4U);
    EXPECT_EQ(loaded.recent_messages[0].content, "u2");
    EXPECT_EQ(loaded.recent_messages[1].content, "a2");
    EXPECT_EQ(loaded.recent_messages[2].content, "u3");
    EXPECT_EQ(loaded.recent_messages[3].content, "a3");
}

TEST(ConfigurationMemoryTest, ConfiguredCharacterLimitDrivesRuntimeRetention) {
    const auto path = write_temp_config(R"({
  "platform_version": "0.1.0",
  "specification_version": "1.0.0",
  "default_repository": "eve",
  "active_ai_provider": "AI-0000",
  "context_limit_chars": 32000,
  "memory_max_recent_messages": 12,
  "memory_max_conversation_chars": 20
})");

    const services::ConfigurationService configuration(path);
    services::ConversationMemoryService memory(
        configuration.memory_max_recent_messages(),
        configuration.memory_max_conversation_chars());

    memory.append("session", "1234567890", "abcdefghij");
    memory.append("session", "xyz", "uvw");

    const auto loaded = memory.load("session");
    ASSERT_EQ(loaded.recent_messages.size(), 2U);
    EXPECT_EQ(loaded.recent_messages[0].content, "xyz");
    EXPECT_EQ(loaded.recent_messages[1].content, "uvw");
}

}  // namespace
}  // namespace eve
