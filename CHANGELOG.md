# Changelog

All notable changes to the Evolutionary Virtual Engineer
(E.V.E.) project will be documented in this file.

This project follows
[Semantic Versioning](https://semver.org/).

---

## [Unreleased]

### Planned - continuing v0.7.x Developer Platform

- Additional AI Providers (OpenAI, Anthropic; AI-0200 deferred)
- Remaining Part XII developer guides (EVE-2005-EVE-2009;
  EVE-2000-EVE-2003 deferred to interface work)

### Planned - v0.8.x Interfaces

- CLI interface implementation (specifications already exist)
- Discord interface
- REST API
- Website interface

### Planned - v0.9.x+

- Plugin architecture
- Multi-repository support
- Semantic / hybrid search

---

## [0.7.0-alpha] - September 2026

Alpha Core Platform / Developer Platform release. This is not
a production interface release and does not complete the full
V0.7.x roadmap or Part XII developer-guide series.

### Added - Conversation Memory Foundation

- Conversation Memory Service (`IConversationMemoryService`,
  in-memory implementation)
- Structured Conversation Context (`ConversationTurn`,
  User/Assistant roles)
- CAP-0102 session load / Context Package attachment
- Provider Formatter history -> ProviderMessages translation
- Successful-turn persistence (`query` + `generated_text`)
- Optional configuration keys:
  `memory_max_recent_messages`,
  `memory_max_conversation_chars`
  (defaults: 12 messages / 8000 characters)

### Limitations - Conversation Memory

- Process-lifetime in-memory storage only
- No summarization, memory search, TTL, or durable backend
- No CAP-0404 explicit memory-management capability handlers
- No streaming-specific memory handling

### Added - Streaming Responses (provider-layer)

- `StreamChunk` / `StreamConsumer` provider contract
- Additive `IAIProvider::generate_stream` and
  `ProviderManager::generate_stream`
- ProviderManager synchronous fallback for non-streaming
  providers
- NullProvider deterministic multi-chunk streaming
  (`supports_streaming = true`)
- Additive HTTP `IHttpTransport::send_stream` with
  incremental body delivery
- HTTP chunked Transfer-Encoding decoding on the streaming
  path
- OllamaProvider genuine streaming (`stream: true`, NDJSON
  line buffering across transport boundaries;
  `supports_streaming = true`)

### Added - LM Studio AI-0102

- LM Studio provider (AI-0102) over local HTTP Chat Completions
  (`/v1/chat/completions`; default base URL
  `http://localhost:1234`)
- Synchronous generation (`stream: false`)
- Genuine SSE streaming (`stream: true`; terminal
  `data: [DONE]`; `supports_streaming = true`)
- ProviderManager direct streaming dispatch for AI-0102
- Optional bearer authentication via `lm_studio_api_key`
- Provider-specific configuration under `lm_studio_*` keys

### Added - Developer Guides

- EVE-2004 - Provider Development Guide (Part XII; series
  incomplete)

### Limitations - Streaming / Transport

- No async streaming runtime, cancellation, or WebSocket
- No streaming memory persistence
- No Discord/CLI/REST/Website streaming interfaces
- No deterministic real TCP SocketHttpTransport ->
  OllamaProvider integration fixture
- Live Ollama streaming is not part of the normal non-live
  suite
- Synchronous `send()` does not use streaming chunked decoding
- Local `http://` transport only (no hosted HTTPS/TLS)
- AI-0200 (generic OpenAI-compatible API) remains unimplemented
  and is not an alias for AI-0102

### Testing

- Non-live suite (`--gtest_filter=-LiveOllama*`):
  278 tests / 275 PASS / 3 FAIL
- Known baseline failures (unchanged):
  `RepositoryDiscoveryTest.SkipsUnsupportedFiles`
  `RepositoryDiscoveryTest.ExcludesIgnoredDirectories`
  `RepositoryDiscoveryTest.ReturnsDeterministicOrdering`

### Deferred (not in this alpha cut)

- OpenAI / Anthropic providers
- AI-0200 generic OpenAI-compatible provider
- llama.cpp
- Remaining Part XII guides (EVE-2000-EVE-2003, EVE-2005-EVE-2009)
- CAP-0404; durable memory; streaming memory
- Discord / CLI / REST / Website implementations

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
| **Unreleased** | 🚧 | Continuing v0.7.x providers/guides; Interfaces |
| **0.7.0-alpha** | ✅ | Memory, streaming, LM Studio, EVE-2004 |
| **0.6.0-alpha** | ✅ | Complete AI Pipeline & Architecture Stabilization |
| **0.5.0-alpha** | ✅ | Provider Abstraction & Reasoning Pipeline |
| **0.4.0-alpha** | ✅ | Knowledge Engine Architecture |
| **0.3.0-alpha** | ✅ | Core Platform Architecture |
| **0.2.0-alpha** | ✅ | Documentation Framework |
| **0.1.0-alpha** | ✅ | Project Foundation |

---

*"Knowledge should evolve."*
