#include "eve/context/context_package.hpp"
#include "eve/core/platform_request.hpp"

#include <gtest/gtest.h>

namespace eve {
namespace {

TEST(ContextPackageTest, ValidatesMissingCapability) {
    const auto request = PlatformRequest::create(CapabilityId{}, InterfaceType::Internal);
    const auto package = context::ContextPackage::create(
        request,
        {},
        {},
        {},
        context::ContextConstraints{},
        context::SystemInstructions{},
        context::PackageDiagnostics{{}, {"missing"}, {}});

    EXPECT_TRUE(validate_context_package(package).has_value());
}

TEST(ContextBuilderTest, RanksSpecificationsFirst) {
    KnowledgeObject spec{
        .identifier = KnowledgeObjectId{"KO-00000001"},
        .repository = "eve",
        .document_type = DocumentType::Specification,
        .title = "Specification",
        .excerpt = "spec content",
        .priority_rank = 10,
    };
    KnowledgeObject readme{
        .identifier = KnowledgeObjectId{"KO-00000002"},
        .repository = "eve",
        .document_type = DocumentType::Readme,
        .title = "Readme",
        .excerpt = "readme content",
        .priority_rank = 60,
    };

    const context::ContextBuilder builder;
    const auto request = PlatformRequest::create(
        CapabilityId{"CAP-0102"},
        InterfaceType::Internal);
    const auto package = builder.build(request, {readme, spec});

    ASSERT_FALSE(package.knowledge_objects().empty());
    EXPECT_EQ(package.knowledge_objects().front().document_type, DocumentType::Specification);
}

}  // namespace
}  // namespace eve
