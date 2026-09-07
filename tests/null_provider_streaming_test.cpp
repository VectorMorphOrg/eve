#include "eve/ai/provider_formatting.hpp"
#include "eve/ai/provider_manager.hpp"
#include "eve/context/context_package.hpp"
#include "eve/core/platform_request.hpp"

#include <gtest/gtest.h>

#include <string>
#include <vector>

namespace eve::ai {
namespace {

context::ContextPackage make_null_stream_package() {
    const auto request = PlatformRequest::create(
        CapabilityId{"CAP-0102"},
        InterfaceType::Internal,
        {{"query", "Platform Request"}});

    KnowledgeObject object;
    object.identity.id = KnowledgeObjectId{"EVE-0010"};
    object.identity.document_identifier = "EVE-0010";
    object.identity.title = "Platform Request Specification";
    object.search.excerpt = "Defines the Platform Request contract.";

    return context::ContextPackage::create(
        request,
        {std::move(object)},
        {context::RepositoryMetadata{.name = "eve"}},
        {context::Citation{.identifier = "EVE-0010", .title = "Platform Request Specification"}},
        context::ContextConstraints{},
        context::SystemInstructions{.language = "en"});
}

ProviderRequest make_null_stream_request() {
    NullProvider provider;
    const ProviderFormatter formatter;
    return formatter.format(
        make_null_stream_package(),
        provider.capabilities(),
        provider.default_options(),
        ProviderMetadata{
            .provider_id = provider.id(),
            .provider_name = provider.name(),
            .request_id = RequestId{"REQ-NULL-STREAM"},
            .package_id = PackageId{"PKG-NULL-STREAM"},
        });
}

TEST(NullProviderStreamingTest, AdvertisesSupportsStreaming) {
    const NullProvider provider;
    EXPECT_TRUE(provider.capabilities().supports_streaming);
}

TEST(NullProviderStreamingTest, EmitsMultipleDeterministicChunks) {
    const NullProvider provider;
    const auto request = make_null_stream_request();

    std::vector<StreamChunk> chunks;
    bool callback_during_call = false;
    bool returned = false;

    const auto response = provider.generate_stream(request, [&](const StreamChunk& chunk) {
        EXPECT_FALSE(returned);
        callback_during_call = true;
        chunks.push_back(chunk);
    });
    returned = true;

    ASSERT_TRUE(response.has_value());
    ASSERT_GT(chunks.size(), 1U);
    EXPECT_TRUE(callback_during_call);

    for (std::size_t i = 0; i + 1 < chunks.size(); ++i) {
        EXPECT_FALSE(chunks[i].done) << "intermediate chunk " << i;
        EXPECT_FALSE(chunks[i].text_delta.empty()) << "intermediate chunk " << i;
    }
    EXPECT_TRUE(chunks.back().done);
}

TEST(NullProviderStreamingTest, ReconstructsExactGeneratedText) {
    const NullProvider provider;
    const auto request = make_null_stream_request();

    std::string concatenated;
    const auto response = provider.generate_stream(request, [&](const StreamChunk& chunk) {
        concatenated += chunk.text_delta;
    });

    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(concatenated, response->generated_text);
}

TEST(NullProviderStreamingTest, MatchesSynchronousGenerateText) {
    const NullProvider provider;
    const auto request = make_null_stream_request();

    const auto sync = provider.generate(request);
    ASSERT_TRUE(sync.has_value());

    std::string concatenated;
    const auto streamed = provider.generate_stream(request, [&](const StreamChunk& chunk) {
        concatenated += chunk.text_delta;
    });
    ASSERT_TRUE(streamed.has_value());

    EXPECT_EQ(streamed->generated_text, sync->generated_text);
    EXPECT_EQ(concatenated, sync->generated_text);
    EXPECT_EQ(streamed->provider_id, sync->provider_id);
    ASSERT_EQ(streamed->warnings.size(), sync->warnings.size());
    for (std::size_t i = 0; i < sync->warnings.size(); ++i) {
        EXPECT_EQ(streamed->warnings[i].level, sync->warnings[i].level);
        EXPECT_EQ(streamed->warnings[i].message, sync->warnings[i].message);
    }
}

context::ContextPackage make_manager_stream_package() {
    const auto request = PlatformRequest::create(
        CapabilityId{"CAP-0102"},
        InterfaceType::Internal,
        {{"query", "Current user request about CAP-0102"}});

    KnowledgeObject object;
    object.identity.id = KnowledgeObjectId{"EVE-0010"};
    object.identity.document_identifier = "EVE-0010";
    object.identity.title = "Platform Request Specification";
    object.search.excerpt = "Defines the Platform Request contract.";
    object.content.sections.push_back(
        KnowledgeSection{.title = "Purpose", .content = "Knowledge context evidence."});

    context::ConversationContext conversation{
        .recent_messages =
            {
                context::ConversationTurn{
                    .role = context::ConversationRole::User,
                    .content = "Earlier user turn",
                },
                context::ConversationTurn{
                    .role = context::ConversationRole::Assistant,
                    .content = "Earlier assistant turn",
                },
            },
    };

    return context::ContextPackage::create(
        request,
        {std::move(object)},
        {context::RepositoryMetadata{.name = "eve"}},
        {context::Citation{.identifier = "EVE-0010", .title = "Platform Request Specification"}},
        context::ContextConstraints{},
        context::SystemInstructions{.language = "en"},
        {},
        std::move(conversation));
}

ProviderManager make_null_provider_manager() {
    ProviderManager manager;
    manager.register_provider(std::make_shared<NullProvider>());
    manager.set_active_provider(ProviderId{"AI-0000"});
    return manager;
}

TEST(ProviderManagerNullStreamingTest, DispatchesRealNullMultiChunkStream) {
    auto manager = make_null_provider_manager();
    const auto package = make_manager_stream_package();

    std::vector<StreamChunk> chunks;
    bool returned = false;
    bool callback_during_call = false;

    const auto response = manager.generate_stream(package, [&](const StreamChunk& chunk) {
        EXPECT_FALSE(returned);
        callback_during_call = true;
        chunks.push_back(chunk);
    });
    returned = true;

    ASSERT_TRUE(response.has_value());
    EXPECT_TRUE(callback_during_call);
    ASSERT_GT(chunks.size(), 1U);

    std::size_t done_count = 0;
    for (std::size_t i = 0; i < chunks.size(); ++i) {
        if (chunks[i].done) {
            ++done_count;
            EXPECT_EQ(i, chunks.size() - 1U);
        } else {
            EXPECT_FALSE(chunks[i].text_delta.empty());
        }
    }
    EXPECT_EQ(done_count, 1U);
    EXPECT_TRUE(chunks.back().done);
}

TEST(ProviderManagerNullStreamingTest, ReconstructsExactFinalText) {
    auto manager = make_null_provider_manager();
    const auto package = make_manager_stream_package();

    std::string concatenated;
    const auto response = manager.generate_stream(package, [&](const StreamChunk& chunk) {
        concatenated += chunk.text_delta;
    });

    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(concatenated, response->generated_text);
}

TEST(ProviderManagerNullStreamingTest, UsesFormatterPathAndCanonicalNullResponse) {
    auto manager = make_null_provider_manager();
    const auto package = make_manager_stream_package();

    const auto sync_via_manager = manager.generate(package);
    ASSERT_TRUE(sync_via_manager.has_value());

    const NullProvider null_provider;
    const auto direct_request = manager.formatter().format(
        package,
        null_provider.capabilities(),
        null_provider.default_options(),
        ProviderMetadata{
            .provider_id = null_provider.id(),
            .provider_name = null_provider.name(),
            .request_id = package.metadata().request_id,
            .package_id = package.metadata().package_id,
        });

    EXPECT_EQ(direct_request.context_package.knowledge_objects().size(), 1U);
    ASSERT_TRUE(direct_request.context_package.conversation().has_value());
    EXPECT_EQ(direct_request.context_package.conversation()->recent_messages.size(), 2U);
    EXPECT_NE(
        direct_request.platform_request.parameters().at("query").find("Current user request"),
        std::string::npos);

    std::string concatenated;
    const auto streamed_via_manager = manager.generate_stream(
        package,
        [&](const StreamChunk& chunk) { concatenated += chunk.text_delta; });
    ASSERT_TRUE(streamed_via_manager.has_value());

    const auto direct_streamed = null_provider.generate_stream(direct_request, [](const StreamChunk&) {});
    ASSERT_TRUE(direct_streamed.has_value());

    EXPECT_EQ(streamed_via_manager->generated_text, sync_via_manager->generated_text);
    EXPECT_EQ(streamed_via_manager->generated_text, direct_streamed->generated_text);
    EXPECT_EQ(concatenated, streamed_via_manager->generated_text);
    EXPECT_EQ(streamed_via_manager->provider_id, "AI-0000");
    ASSERT_EQ(streamed_via_manager->warnings.size(), sync_via_manager->warnings.size());
    ASSERT_EQ(streamed_via_manager->warnings.size(), direct_streamed->warnings.size());
    for (std::size_t i = 0; i < streamed_via_manager->warnings.size(); ++i) {
        EXPECT_EQ(streamed_via_manager->warnings[i].level, sync_via_manager->warnings[i].level);
        EXPECT_EQ(streamed_via_manager->warnings[i].message, sync_via_manager->warnings[i].message);
        EXPECT_EQ(streamed_via_manager->warnings[i].level, direct_streamed->warnings[i].level);
        EXPECT_EQ(streamed_via_manager->warnings[i].message, direct_streamed->warnings[i].message);
    }
    EXPECT_NE(
        streamed_via_manager->generated_text.find(package.metadata().package_id.value),
        std::string::npos);
    EXPECT_NE(streamed_via_manager->generated_text.find("1 knowledge object"), std::string::npos);
}

TEST(ProviderManagerNullStreamingTest, DoesNotRequireConversationMemoryService) {
    ProviderManager manager;
    manager.register_provider(std::make_shared<NullProvider>());
    manager.set_active_provider(ProviderId{"AI-0000"});

    int callback_calls = 0;
    const auto response = manager.generate_stream(
        make_manager_stream_package(),
        [&](const StreamChunk&) { ++callback_calls; });

    ASSERT_TRUE(response.has_value());
    EXPECT_GT(callback_calls, 1);
}

}  // namespace
}  // namespace eve::ai
