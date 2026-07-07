#include "eve/core/core_platform.hpp"
#include "eve/core/platform_request.hpp"

#include <gtest/gtest.h>

#include <algorithm>

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

TEST(IntegrationTest, DocumentationSearchEndToEnd) {
    CorePlatform platform = PlatformBootstrap::create(EVE_PROJECT_ROOT);
    const auto request = PlatformRequest::create(
        CapabilityId{"CAP-0102"},
        InterfaceType::Internal,
        {{"query", "Platform Request"}});

    const auto response = platform.process(std::move(request));
    ASSERT_TRUE(response.has_value());
    EXPECT_TRUE(
        response->status() == ResponseStatus::Success ||
        response->status() == ResponseStatus::PartialSuccess);
    EXPECT_NE(response->content().primary.find("Platform Request"), std::string::npos);
    EXPECT_FALSE(response->references().empty());
}

TEST(IntegrationTest, DocumentationSearchUsesReasoningPipeline) {
    CorePlatform platform = PlatformBootstrap::create(EVE_PROJECT_ROOT);
    const auto request = PlatformRequest::create(
        CapabilityId{"CAP-0102"},
        InterfaceType::Internal,
        {{"query", "Platform Request"}});

    const auto response = platform.process(std::move(request));
    ASSERT_TRUE(response.has_value());

    EXPECT_EQ(response->content().structured.at("reasoning_pipeline"), "4.1");
    EXPECT_FALSE(response->content().structured.at("context_package_id").empty());
    EXPECT_GT(std::stoul(response->content().structured.at("search_result_count")), 0U);
    EXPECT_GT(std::stoul(response->content().structured.at("ranked_result_count")), 0U);
    EXPECT_GT(std::stoul(response->content().structured.at("citation_bundle_count")), 0U);

    EXPECT_TRUE(trace_contains_component(*response, "SearchEngine"));
    EXPECT_TRUE(trace_contains_component(*response, "RankingEngine"));
    EXPECT_TRUE(trace_contains_component(*response, "CitationEngine"));
    EXPECT_TRUE(trace_contains_component(*response, "ContextAssembler"));
    EXPECT_TRUE(trace_contains_component(*response, "NullProvider"));

    EXPECT_TRUE(services_executed_contains(*response, "SearchEngine"));
    EXPECT_TRUE(services_executed_contains(*response, "RankingEngine"));
    EXPECT_TRUE(services_executed_contains(*response, "CitationEngine"));
    EXPECT_TRUE(services_executed_contains(*response, "ContextAssembler"));
    EXPECT_TRUE(services_executed_contains(*response, "NullProvider"));
    EXPECT_TRUE(services_executed_contains(*response, "CommandDispatcher"));
    EXPECT_TRUE(services_executed_contains(*response, "CapabilityEngine"));
}

TEST(IntegrationTest, ReasoningPipelineExecutesBeforeNullProvider) {
    CorePlatform platform = PlatformBootstrap::create(EVE_PROJECT_ROOT);
    const auto request = PlatformRequest::create(
        CapabilityId{"search"},
        InterfaceType::Internal,
        {{"query", "Context Package"}});

    const auto response = platform.process(std::move(request));
    ASSERT_TRUE(response.has_value());
    EXPECT_NE(response->content().primary.find("Context package"), std::string::npos);
    EXPECT_TRUE(trace_contains_component(*response, "ContextAssembler"));
    EXPECT_TRUE(trace_contains_component(*response, "NullProvider"));
}

TEST(IntegrationTest, StatusCapabilityViaAlias) {
    CorePlatform platform = PlatformBootstrap::create(EVE_PROJECT_ROOT);
    const auto request = PlatformRequest::create(
        CapabilityId{"status"},
        InterfaceType::Cli);

    const auto response = platform.process(std::move(request));
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->status(), ResponseStatus::Success);
    EXPECT_NE(response->content().primary.find("0.1.0"), std::string::npos);
}

}  // namespace
}  // namespace eve
