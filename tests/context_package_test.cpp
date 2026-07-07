#include "eve/context/context_package.hpp"
#include "eve/core/platform_request.hpp"

#include <gtest/gtest.h>

namespace eve {
namespace {

KnowledgeObject make_object(
    KnowledgeObjectId id,
    DocumentType type,
    std::string title,
    std::string excerpt,
    int priority_rank) {
    KnowledgeObject object;
    object.identity.id = std::move(id);
    object.identity.title = std::move(title);
    object.identity.document_type = type;
    object.identity.repository = "eve";
    object.search.excerpt = std::move(excerpt);
    object.search.search_text = object.search.excerpt;
    object.search.priority_rank = priority_rank;
    return object;
}

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
    const KnowledgeObject spec = make_object(
        KnowledgeObjectId{"KO-00000001"},
        DocumentType::Specification,
        "Specification",
        "spec content",
        10);
    const KnowledgeObject readme = make_object(
        KnowledgeObjectId{"KO-00000002"},
        DocumentType::Readme,
        "Readme",
        "readme content",
        60);

    const context::ContextBuilder builder;
    const auto request = PlatformRequest::create(
        CapabilityId{"CAP-0102"},
        InterfaceType::Internal);
    const auto package = builder.build(request, {readme, spec});

    ASSERT_FALSE(package.knowledge_objects().empty());
    EXPECT_EQ(package.knowledge_objects().front().identity.document_type, DocumentType::Specification);
}

}  // namespace
}  // namespace eve
