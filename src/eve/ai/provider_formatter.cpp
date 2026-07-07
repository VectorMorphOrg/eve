#include "eve/ai/provider_formatting.hpp"

#include <format>
#include <sstream>

namespace eve::ai {
namespace {

void append_section(std::ostringstream& stream, std::string_view title, std::string_view content) {
    if (content.empty()) {
        return;
    }
    stream << title << ":\n" << content << "\n\n";
}

std::string build_identity_section() {
    return
        "You are E.V.E. (Evolutionary Virtual Engineer), an engineering assistant.\n"
        "You answer using supplied engineering evidence only.";
}

std::string build_engineering_rules_section() {
    return
        "The deterministic reasoning pipeline has already completed.\n"
        "The supplied context is authoritative.\n"
        "Do not claim independent retrieval.\n"
        "Do not invent facts outside supplied evidence.\n"
        "State clearly when evidence is insufficient.\n"
        "Prefer engineering precision over speculation.";
}

std::string build_behavior_constraints_section(const context::ContextPackage& package) {
    return std::format(
        "documentation_first={}\n"
        "preserve_citations={}\n"
        "do_not_invent_information={}",
        package.constraints().documentation_first,
        package.constraints().preserve_citations,
        package.constraints().do_not_invent_information);
}

std::string build_evidence_requirements_section() {
    return
        "Use only the evidence provided in the context section.\n"
        "Preserve citation identifiers when referencing documents.\n"
        "Maintain the ordering of supplied evidence.";
}

std::string build_system_prompt(const context::ContextPackage& package) {
    std::ostringstream stream;
    append_section(stream, "IDENTITY", build_identity_section());
    append_section(stream, "ENGINEERING RULES", build_engineering_rules_section());
    append_section(stream, "BEHAVIOR CONSTRAINTS", build_behavior_constraints_section(package));
    append_section(stream, "EVIDENCE REQUIREMENTS", build_evidence_requirements_section());

    if (package.system().personality_profile.has_value()) {
        append_section(stream, "PERSONALITY", *package.system().personality_profile);
    }
    if (package.system().language.has_value()) {
        append_section(stream, "LANGUAGE", *package.system().language);
    }

    return stream.str();
}

std::string build_user_request_prompt(const context::ContextPackage& package) {
    std::ostringstream stream;
    stream << "Capability: " << package.capability().value << '\n';
    stream << "Request ID: " << package.metadata().request_id.value << '\n';

    for (const auto& [name, value] : package.request().parameters()) {
        stream << name << ": " << value << '\n';
    }

    return stream.str();
}

std::string build_context_prompt(const context::ContextPackage& package) {
    std::ostringstream stream;

    for (const auto& object : package.knowledge_objects()) {
        stream << "Document: " << object.identity.document_identifier << " — "
               << object.identity.title << '\n';
        stream << "Repository: " << object.identity.repository << '\n';
        stream << "Series: " << object.metadata.series << '\n';
        stream << "Part: " << object.metadata.part << '\n';
        stream << "Status: " << object.metadata.status << '\n';

        if (!object.search.excerpt.empty()) {
            stream << "Excerpt: " << object.search.excerpt << '\n';
        }

        for (const auto& section : object.content.sections) {
            stream << "Section: " << section.title << '\n' << section.content << '\n';
        }

        for (const auto& heading : object.content.headings) {
            stream << "Heading: " << heading.text << '\n';
        }

        for (const auto& header : object.relationships.related_headers) {
            stream << "Implementation Header: " << header << '\n';
        }
        for (const auto& source : object.relationships.related_sources) {
            stream << "Implementation Source: " << source << '\n';
        }
        for (const auto& test : object.relationships.related_tests) {
            stream << "Validation Test: " << test << '\n';
        }

        if (object.relationships.previous.has_value()) {
            stream << "Previous: " << *object.relationships.previous << '\n';
        }
        if (object.relationships.next.has_value()) {
            stream << "Next: " << *object.relationships.next << '\n';
        }
        if (object.relationships.parent.has_value()) {
            stream << "Parent: " << *object.relationships.parent << '\n';
        }
        for (const auto& child : object.relationships.children) {
            stream << "Child: " << child << '\n';
        }

        stream << '\n';
    }

    if (!package.citations().empty()) {
        stream << "Citations:\n";
        for (const auto& citation : package.citations()) {
            stream << "- " << citation.identifier << " — " << citation.title;
            if (citation.section.has_value()) {
                stream << " (" << *citation.section << ')';
            }
            stream << '\n';
        }
    }

    return stream.str();
}

}  // namespace

std::string to_string(ProviderMessageRole role) {
    switch (role) {
        case ProviderMessageRole::System:
            return "system";
        case ProviderMessageRole::User:
            return "user";
        case ProviderMessageRole::Assistant:
            return "assistant";
        case ProviderMessageRole::Tool:
            return "tool";
    }
    return "user";
}

ProviderCapabilities null_provider_capabilities() {
    return ProviderCapabilities{
        .supports_streaming = false,
        .supports_system_prompts = false,
        .supports_tool_calling = false,
        .supports_json_responses = false,
        .supports_image_input = false,
        .supports_reasoning_mode = false,
        .supports_function_calling = false,
        .supports_multiple_messages = false,
    };
}

ProviderCapabilities ollama_provider_capabilities() {
    return ProviderCapabilities{
        .supports_streaming = false,
        .supports_system_prompts = true,
        .supports_tool_calling = false,
        .supports_json_responses = false,
        .supports_image_input = false,
        .supports_reasoning_mode = false,
        .supports_function_calling = false,
        .supports_multiple_messages = true,
    };
}

ProviderRequest ProviderFormatter::format(
    const context::ContextPackage& package,
    ProviderCapabilities capabilities,
    ProviderOptions options,
    ProviderMetadata metadata) const {
    ProviderRequest request{
        .capabilities = capabilities,
        .options = std::move(options),
        .system_instructions = package.system(),
        .platform_request = package.request(),
        .context_package = package,
        .metadata = std::move(metadata),
    };

    const auto system_prompt = build_system_prompt(package);
    const auto user_request_prompt = build_user_request_prompt(package);
    const auto context_prompt = build_context_prompt(package);

    if (capabilities.supports_system_prompts && !system_prompt.empty()) {
        request.messages.push_back(ProviderMessage{
            .role = ProviderMessageRole::System,
            .content = system_prompt,
        });
    }

    if (capabilities.supports_multiple_messages && !context_prompt.empty()) {
        std::string first_user_message = user_request_prompt;
        if (!capabilities.supports_system_prompts && !system_prompt.empty()) {
            first_user_message = system_prompt + "\n\n" + first_user_message;
        }

        request.messages.push_back(ProviderMessage{
            .role = ProviderMessageRole::User,
            .content = first_user_message,
        });
        request.messages.push_back(ProviderMessage{
            .role = ProviderMessageRole::User,
            .content = "CONTEXT:\n" + context_prompt,
        });
    } else {
        std::ostringstream combined;
        if (!capabilities.supports_system_prompts && !system_prompt.empty()) {
            combined << system_prompt << "\n\n";
        }
        combined << user_request_prompt;
        if (!context_prompt.empty()) {
            combined << "\n\nCONTEXT:\n" << context_prompt;
        }
        request.messages.push_back(ProviderMessage{
            .role = ProviderMessageRole::User,
            .content = combined.str(),
        });
    }

    return request;
}

}  // namespace eve::ai
