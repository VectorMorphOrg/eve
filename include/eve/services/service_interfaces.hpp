#pragma once

#include "eve/core/platform_request.hpp"
#include "eve/core/platform_response.hpp"
#include "eve/knowledge/knowledge_object.hpp"

#include <expected>
#include <memory>
#include <string>
#include <vector>

namespace eve::services {

class IConfigurationService {
public:
    virtual ~IConfigurationService() = default;
    [[nodiscard]] virtual std::string platform_version() const = 0;
    [[nodiscard]] virtual std::string specification_version() const = 0;
    [[nodiscard]] virtual std::string default_repository() const = 0;
    [[nodiscard]] virtual std::vector<std::filesystem::path> documentation_paths() const = 0;
    [[nodiscard]] virtual std::string active_ai_provider() const = 0;
    [[nodiscard]] virtual std::size_t context_limit_chars() const = 0;
    [[nodiscard]] virtual std::optional<std::string> get(std::string_view key) const = 0;
};

class ILoggingService {
public:
    virtual ~ILoggingService() = default;
    virtual void info(std::string_view message) = 0;
    virtual void warn(std::string_view message) = 0;
    virtual void error(std::string_view message) = 0;
};

class IDocumentationService {
public:
    virtual ~IDocumentationService() = default;
    [[nodiscard]] virtual std::expected<std::vector<KnowledgeObject>, PlatformError> lookup(
        std::string_view identifier) const = 0;
    [[nodiscard]] virtual std::expected<std::vector<KnowledgeObject>, PlatformError> list_repository(
        std::string_view repository) const = 0;
};

class ISearchService {
public:
    virtual ~ISearchService() = default;
    [[nodiscard]] virtual std::expected<std::vector<SearchResult>, PlatformError> search(
        std::string_view query,
        std::optional<std::string_view> repository = std::nullopt) const = 0;
};

class IStatusService {
public:
    virtual ~IStatusService() = default;
    [[nodiscard]] virtual PlatformResponse report_status(const PlatformRequest& request) const = 0;
};

class IDiagnosticsService {
public:
    virtual ~IDiagnosticsService() = default;
    [[nodiscard]] virtual std::vector<DiagnosticMessage> collect() const = 0;
};

}  // namespace eve::services
