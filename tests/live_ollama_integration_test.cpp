#include "eve/ai/ollama_provider.hpp"
#include "eve/core/core_platform.hpp"
#include "eve/core/platform_request.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <optional>
#include <string_view>

#ifndef EVE_PROJECT_ROOT
#define EVE_PROJECT_ROOT "."
#endif

namespace eve {
namespace {

bool trace_contains_component(const PlatformResponse& response, std::string_view component) {
    return std::ranges::any_of(response.trace().entries, [&](const TraceEntry& entry) {
        return entry.component == component;
    });
}

bool services_executed_contains(const PlatformResponse& response, std::string_view service) {
    return std::ranges::any_of(response.trace().services_executed, [&](const std::string& value) {
        return value == service;
    });
}

bool ollama_server_available() {
    const ai::OllamaProvider probe(ai::OllamaProviderConfig::defaults());
    return probe.health_check().has_value();
}

CorePlatform create_ollama_platform() {
    CorePlatform platform = PlatformBootstrap::create(EVE_PROJECT_ROOT);
    platform.dependencies().provider_manager->set_active_provider(ProviderId{"AI-0100"});
    return platform;
}

std::optional<PlatformResponse> execute_documentation_search(
    CorePlatform& platform,
    std::string_view query) {
    const auto request = PlatformRequest::create(
        CapabilityId{"CAP-0102"},
        InterfaceType::Internal,
        {{"query", std::string(query)}});

    const auto response = platform.process(std::move(request));
    if (!response) {
        return std::nullopt;
    }
    return *response;
}

void assert_full_pipeline_trace(const PlatformResponse& response) {
    EXPECT_TRUE(trace_contains_component(response, "ValidationEngine"));
    EXPECT_TRUE(trace_contains_component(response, "CapabilityEngine"));
    EXPECT_TRUE(trace_contains_component(response, "SearchEngine"));
    EXPECT_TRUE(trace_contains_component(response, "RankingEngine"));
    EXPECT_TRUE(trace_contains_component(response, "CitationEngine"));
    EXPECT_TRUE(trace_contains_component(response, "ContextAssembler"));
    EXPECT_TRUE(trace_contains_component(response, "ProviderFormatter"));
    EXPECT_TRUE(trace_contains_component(response, "Ollama"));
    EXPECT_TRUE(trace_contains_component(response, "CorePlatform"));

    EXPECT_TRUE(services_executed_contains(response, "ValidationEngine"));
    EXPECT_TRUE(services_executed_contains(response, "CapabilityEngine"));
    EXPECT_TRUE(services_executed_contains(response, "SearchEngine"));
    EXPECT_TRUE(services_executed_contains(response, "RankingEngine"));
    EXPECT_TRUE(services_executed_contains(response, "CitationEngine"));
    EXPECT_TRUE(services_executed_contains(response, "ContextAssembler"));
    EXPECT_TRUE(services_executed_contains(response, "ProviderFormatter"));
    EXPECT_TRUE(services_executed_contains(response, "Ollama"));
    EXPECT_TRUE(services_executed_contains(response, "CommandDispatcher"));

    EXPECT_EQ(response.trace().provider_selected, "AI-0100");
    EXPECT_FALSE(response.content().structured.at("context_package_id").empty());
    EXPECT_GT(std::stoul(response.content().structured.at("search_result_count")), 0U);
    EXPECT_GT(std::stoul(response.content().structured.at("ranked_result_count")), 0U);
    EXPECT_GT(std::stoul(response.content().structured.at("citation_bundle_count")), 0U);
    EXPECT_FALSE(response.references().empty());
}

void assert_reasoning_precedes_inference(const PlatformResponse& response) {
    const auto& entries = response.trace().entries;
    const auto validation_index = std::ranges::find_if(entries, [](const TraceEntry& entry) {
        return entry.component == "ValidationEngine" && entry.detail.find("request validated") != std::string::npos;
    });
    const auto search_index = std::ranges::find_if(entries, [](const TraceEntry& entry) {
        return entry.component == "SearchEngine";
    });
    const auto formatter_index = std::ranges::find_if(entries, [](const TraceEntry& entry) {
        return entry.component == "ProviderFormatter";
    });
    const auto ollama_index = std::ranges::find_if(entries, [](const TraceEntry& entry) {
        return entry.component == "Ollama";
    });

    ASSERT_NE(validation_index, entries.end());
    ASSERT_NE(search_index, entries.end());
    ASSERT_NE(formatter_index, entries.end());
    ASSERT_NE(ollama_index, entries.end());
    EXPECT_LT(std::distance(entries.begin(), validation_index), std::distance(entries.begin(), search_index));
    EXPECT_LT(std::distance(entries.begin(), search_index), std::distance(entries.begin(), formatter_index));
    EXPECT_LT(std::distance(entries.begin(), formatter_index), std::distance(entries.begin(), ollama_index));
}

void assert_grounded_response(
    const PlatformResponse& response,
    std::initializer_list<std::string_view> expected_terms) {
    const auto& text = response.content().primary;
    EXPECT_FALSE(text.empty());
    EXPECT_EQ(text.find("No AI provider is configured"), std::string::npos);

    for (const auto term : expected_terms) {
        const bool in_text = text.find(std::string(term)) != std::string::npos;
        const bool in_references = std::ranges::any_of(response.references(), [&](const Reference& reference) {
            return reference.identifier.find(term) != std::string::npos ||
                   reference.title.find(term) != std::string::npos;
        });
        EXPECT_TRUE(in_text || in_references) << "Missing expected term: " << term;
    }
}

class LiveOllamaIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        if (!ollama_server_available()) {
            GTEST_SKIP() << "Ollama server is not available at http://localhost:11434";
        }
    }
};

TEST_F(LiveOllamaIntegrationTest, EndToEndDocumentationSearchExecutesFullPipeline) {
    auto platform = create_ollama_platform();
    const auto response = execute_documentation_search(platform, "Platform Request");
    ASSERT_TRUE(response.has_value());

    assert_full_pipeline_trace(*response);
    assert_reasoning_precedes_inference(*response);
    assert_grounded_response(*response, {"Platform Request", "AI Provider"});
}

TEST_F(LiveOllamaIntegrationTest, WhatIsEve0010) {
    auto platform = create_ollama_platform();
    const auto response = execute_documentation_search(platform, "What is EVE-0010?");
    ASSERT_TRUE(response.has_value());

    assert_full_pipeline_trace(*response);
    assert_reasoning_precedes_inference(*response);
    assert_grounded_response(*response, {"EVE-0010"});
}

TEST_F(LiveOllamaIntegrationTest, ExplainValidationEngine) {
    auto platform = create_ollama_platform();
    const auto response = execute_documentation_search(platform, "Explain the Validation Engine.");
    ASSERT_TRUE(response.has_value());

    assert_full_pipeline_trace(*response);
    assert_reasoning_precedes_inference(*response);
    assert_grounded_response(*response, {"Validation"});
}

TEST_F(LiveOllamaIntegrationTest, HowDoesContextBuilderWork) {
    auto platform = create_ollama_platform();
    const auto response = execute_documentation_search(platform, "How does Context Builder work?");
    ASSERT_TRUE(response.has_value());

    assert_full_pipeline_trace(*response);
    assert_reasoning_precedes_inference(*response);
    assert_grounded_response(*response, {"Context Builder"});
}

TEST_F(LiveOllamaIntegrationTest, RelationshipBetweenEve0010AndEve0011) {
    auto platform = create_ollama_platform();
    const auto response = execute_documentation_search(
        platform,
        "What is the relationship between EVE-0010 and EVE-0011?");
    ASSERT_TRUE(response.has_value());

    assert_full_pipeline_trace(*response);
    assert_reasoning_precedes_inference(*response);
    assert_grounded_response(*response, {"EVE-0010", "EVE-0011"});
}

}  // namespace
}  // namespace eve
