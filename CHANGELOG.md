# Changelog

All notable changes to the Evolutionary Virtual Engineer
(E.V.E.) project will be documented in this file.

This project follows
[Semantic Versioning](https://semver.org/).

---

## [Unreleased]

### Added — v0.7.0 Conversation Memory Foundation

- Conversation Memory Service (`IConversationMemoryService`,
  in-memory implementation)
- Structured Conversation Context (`ConversationTurn`,
  User/Assistant roles)
- CAP-0102 session load / Context Package attachment
- Provider Formatter history → ProviderMessages translation
- Successful-turn persistence (`query` + `generated_text`)
- Optional configuration keys:
  `memory_max_recent_messages`,
  `memory_max_conversation_chars`
  (defaults: 12 messages / 8000 characters)

### Limitations — Conversation Memory

- Process-lifetime in-memory storage only
- No summarization, memory search, TTL, or durable backend
- No CAP-0404 explicit memory-management capability handlers
- Streaming-specific memory handling deferred

### Planned — remaining v0.7.x Developer Platform

- Streaming responses
- Additional AI Providers (OpenAI, Anthropic, LM Studio)
- 2000 Developer Guide series

### Planned — v0.8.x Interfaces

- CLI interface implementation (specifications already exist)
- Discord interface
- REST API
- Website interface

### Planned — v0.9.x+

- Plugin architecture
- Multi-repository support
- Semantic / hybrid search

---

## [0.6.0-alpha] - July 2026

### Added

- Complete deterministic Reasoning Pipeline
- Provider Formatter architecture
- Provider Request model
- Provider Manager
- AI Response model
- HTTP Transport abstraction
- Ollama Provider implementation
- Null Provider implementation
- End-to-end AI generation pipeline
- Knowledge Engine implementation architecture
- Complete 1000 Implementation Guide series
- Complete 3000 Architecture Decision Record series

### Changed

- Refactored AI Provider architecture around Provider
  Requests.
- Standardized Context Package flow.
- Updated Engineering Specifications to match implemented
  architecture.
- Expanded Reference Catalogs.
- Improved repository organization and documentation
  hierarchy.

### Documentation

- Added comprehensive Implementation Guides.
- Added Architecture Decision Records.
- Updated README files across documentation series.
- Standardized navigation throughout documentation.

---

## [0.5.0-alpha] - July 2026

### Added

- Provider-independent AI architecture
- Provider abstraction layer
- Initial Reasoning Pipeline
- Context Builder architecture
- Context Package architecture
- AI Provider specifications

### Changed

- Separated engineering reasoning from language generation.
- Introduced Provider Formatter concept.

---

## [0.4.0-alpha] - July 2026

### Added

- Knowledge Layer architecture
- Knowledge Objects
- Knowledge Graph
- Knowledge Index
- Search architecture
- Citation architecture
- Repository integration specifications

### Changed

- Established Repository as Source of Truth.
- Expanded deterministic engineering pipeline.

---

## [0.3.0-alpha] - July 2026

### Added

- Validation Engine
- Capability Engine
- Platform Request specification
- Platform Response specification
- Core Platform contracts
- Initial Reference Catalogs

### Changed

- Adopted Capability-Oriented Architecture.

---

## [0.2.0-alpha] - July 2026

### Added

- Engineering documentation framework
- Documentation hierarchy
- Initial project structure
- Architecture planning
- EVE-SPEC organization

### Changed

- Adopted Documentation First Development.

---

## [0.1.0-alpha] - July 2026

### Added

- Initial repository
- Repository structure
- Project vision
- Engineering philosophy
- Initial specifications
- README files
- MIT License
- Semantic Versioning

---

## Version History

| Version | Status | Summary |
|----------|--------|---------|
| **Unreleased** | 🚧 | Developer Guides, Interfaces, Additional Providers |
| **0.6.0-alpha** | ✅ | Complete AI Pipeline & Architecture Stabilization |
| **0.5.0-alpha** | ✅ | Provider Abstraction & Reasoning Pipeline |
| **0.4.0-alpha** | ✅ | Knowledge Engine Architecture |
| **0.3.0-alpha** | ✅ | Core Platform Architecture |
| **0.2.0-alpha** | ✅ | Documentation Framework |
| **0.1.0-alpha** | ✅ | Project Foundation |

---

*"Knowledge should evolve."*