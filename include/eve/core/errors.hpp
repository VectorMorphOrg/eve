#pragma once

#include <string>
#include <vector>

namespace eve {

struct PlatformError {
    std::string type;
    std::string description;
    std::string resolution;
};

struct ValidationError {
    std::string field;
    std::string message;
};

struct CapabilityError {
    std::string capability_id;
    std::string message;
};

struct ProviderError {
    std::string provider_id;
    std::string message;
};

struct DiagnosticMessage {
    std::string level;
    std::string message;
};

struct Reference {
    std::string kind;
    std::string identifier;
    std::string title;
};

struct SuggestedAction {
    std::string label;
    std::string action_type;
    std::string target;
};

struct TraceEntry {
    std::string component;
    std::string detail;
};

}  // namespace eve
