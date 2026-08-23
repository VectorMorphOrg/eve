# E.V.E. Document Map

**Purpose:** Canonical reference for file names, folder locations, and documentation format conventions across the `eve/` repository. Intended for AI assistants and contributors navigating the project at **v0.6.0-alpha**.

**Last Updated:** August 23, 2026  
**Release Baseline:** v0.6.0-alpha  
**Authoritative Specification:** EVE-SPEC (`docs/README.md`)

---

# Quick Orientation

| What you need | Where to look |
|---------------|---------------|
| Project overview | `README.md` |
| Current engineering status | `PROJECT_STATUS.md` |
| Runtime architecture (non-normative) | `ARCHITECTURE.md` |
| EVE-SPEC index | `docs/README.md` |
| Contributor rules for AI/humans | `AGENTS.md` |
| Release audits | `audits/v0.6.0-alpha/` |
| Platform config | `config/platform.json` |
| Live Ollama config | `config/platform.live.json` |
| Public C++ headers | `include/eve/` |
| Implementation | `src/eve/` |
| Tests | `tests/` |

**Important:** Architecture Decision Records live in `docs/3000-architecture-decision-records/` — **not** `docs/adr/` (that path does not exist).

---

# Repository Root (`eve/`)

```
eve/
├── README.md                 # Project overview, build instructions, v0.6.0-alpha status
├── ARCHITECTURE.md           # High-level runtime architecture (non-normative guide)
├── PROJECT_STATUS.md         # Current milestone, component completion matrix
├── CHANGELOG.md              # Release history v0.1.0-alpha → v0.6.0-alpha
├── ROADMAP.md                # Forward-looking roadmap
├── AGENTS.md                 # AI/contributor onboarding rules (read before coding)
├── CONTRIBUTING.md           # Contribution guidelines
├── CODE_OF_CONDUCT.md
├── SECURITY.md
├── LICENSE                   # MIT
├── CMakeLists.txt            # Root CMake; builds libeve_core + tests
├── .gitignore
│
├── cmake/
│   └── compiler_flags.cmake  # Shared compiler flags
│
├── config/
│   ├── platform.json         # Default config (NullProvider AI-0000 active)
│   └── platform.live.json    # Live Ollama demo config (AI-0100 active)
│
├── include/eve/              # Public C++ headers (see Code Map below)
├── src/eve/                  # C++ implementation (mirrors include layout)
├── tests/                    # GoogleTest suite (172 tests)
│
├── docs/                     # EVE-SPEC — authoritative documentation
├── ideas/                    # Informal future concepts (non-normative)
│   └── distributed-capability-network.md
│
└── audits/                   # Engineering QA audit reports
    ├── README.md
    ├── document-map.md       # Canonical repository map (this file)
    └── v0.6.0-alpha/
        ├── AUDIT-NAVIGATION.md
        ├── DOCUMENTATION_AUDIT.md
        ├── DOCUMENTATION_RECONCILIATION.md
        ├── DOCUMENTATION_RECONCILIATION_RESULT.md
        ├── IMPLEMENTATION_AUDIT.md
        ├── RELEASE_READINESS.md
        └── REORIENTATION_REPORT.md
```

---

# EVE-SPEC Documentation System (`docs/`)

## Series Numbering Convention

EVE-SPEC organizes documents into **numbered series folders**. Each document has a unique ID:

```
EVE-{SERIES}{SEQUENCE}
     ^^^^  ^^^^
     |     └── 4-digit sequence within series (e.g., 0010)
     └── 4-digit series prefix (e.g., 0000, 1000, 3000)
```

**Filename pattern:**

```
docs/{SERIES-FOLDER}/eve-{ID}-{kebab-case-title}.md
```

**Examples:**

| ID | Folder | Filename |
|----|--------|----------|
| EVE-0007 | `0000-core-platform-specifications/` | `eve-0007-ai-provider-specification.md` |
| EVE-0300 | `0300-cli-specifications/` | `eve-0300-cli-interface-specification.md` |
| EVE-1008 | `1000-implementation-guides/` | `eve-1008-reasoning-pipeline-implementation-guide.md` |
| EVE-3005 | `3000-architecture-decision-records/` | `eve-3005-ai-provider-abstraction.md` |

Every series folder contains a **`README.md`** index for that part.

---

## EVE-SPEC Parts Table

| Part | Folder | Series | Description | Content Status |
|------|--------|--------|-------------|----------------|
| — | `docs/README.md` | — | EVE-SPEC root index | ✅ Active |
| I | `0000-core-platform-specifications/` | 0000 | Core Platform Specifications | ✅ 13 docs (EVE-0000–0012) |
| II | `0100-discord-specifications/` | 0100 | Discord Interface | ✅ 4 docs (EVE-0100–0103) |
| III | `0200-website-specifications/` | 0200 | Website Interface | ⏳ Placeholders only (EVE-0200–0201 empty) |
| IV | `0300-cli-specifications/` | 0300 | CLI Interface | ✅ 10 docs (EVE-0300–0309) |
| V | `0400-rest-api-specifications/` | 0400 | REST API | ⏳ Placeholder (EVE-0400 empty) |
| VI | `0500-platform-specifications/` | 0500 | Platform Services | ⏳ Placeholders (EVE-0500–0502 empty) |
| VII | `0600-reference-catalogs/` | 0600 | Reference Catalogs | ✅ 10 catalogs (EVE-0600–0609) |
| VIII | `0700-testing-specifications/` | 0700 | Testing | ⏳ README only |
| IX | `0800-deployment-specifications/` | 0800 | Deployment | ⏳ README only |
| X | `0900-security-specifications/` | 0900 | Security | ⏳ README only |
| XI | `1000-implementation-guides/` | 1000 | Implementation Guides | ✅ 12 guides (EVE-1000–1011) |
| XII | `2000-developer-guides/` | 2000 | Developer Guides | ⏳ 10 empty stubs (EVE-2000–2009) |
| XIII | `3000-architecture-decision-records/` | 3000 | ADRs | ✅ 9 ADRs (EVE-3001–3009) |
| XIV | `9000-archive/` | 9000 | Archive | ⏳ README only |

---

## Complete Document Inventory

### Part I — Core Platform Specifications (`docs/0000-core-platform-specifications/`)

| ID | File | Title |
|----|------|-------|
| — | `README.md` | Series index |
| EVE-0000 | `eve-0000-vision-and-design-philosophy.md` | Vision and Design Philosophy |
| EVE-0001 | `eve-0001-system-architecture.md` | System Architecture |
| EVE-0002 | `eve-0002-capability-specification.md` | Capability Specification |
| EVE-0003 | `eve-0003-service-layer-specification.md` | Service Layer Specification |
| EVE-0004 | `eve-0004-knowledge-layer-specification.md` | Knowledge Layer Specification |
| EVE-0005 | `eve-0005-context-builder-specification.md` | Context Builder Specification |
| EVE-0006 | `eve-0006-context-package-specification.md` | Context Package Specification |
| EVE-0007 | `eve-0007-ai-provider-specification.md` | AI Provider Specification |
| EVE-0008 | `eve-0008-personality-specification.md` | Persona and Communication Specification |
| EVE-0009 | `eve-0009-documentation-integration-specification.md` | Documentation Integration Specification |
| EVE-0010 | `eve-0010-platform-request-specification.md` | Platform Request Specification |
| EVE-0011 | `eve-0011-platform-response-specification.md` | Platform Response Specification |
| EVE-0012 | `eve-0012-validation-specification.md` | Validation Specification |

### Part II — Discord Specifications (`docs/0100-discord-specifications/`)

| ID | File | Title |
|----|------|-------|
| — | `README.md` | Series index |
| EVE-0100 | `eve-0100-discord-interface-specification.md` | Discord Interface Specification |
| EVE-0101 | `eve-0101-discord-command-mapping.md` | Discord Command Mapping |
| EVE-0102 | `eve-0102-discord-permissions.md` | Discord Permissions |
| EVE-0103 | `eve-0103-discord-embeds-and-formatting.md` | Discord Response and Formatting *(filename says embeds)* |

### Part III — Website Specifications (`docs/0200-website-specifications/`)

| ID | File | Status |
|----|------|--------|
| — | `README.md` | Index |
| EVE-0200 | `eve-0200-website-interface-specification.md` | **Empty placeholder** |
| EVE-0201 | `eve-0201-web-chat-interface.md` | **Empty placeholder** |

### Part IV — CLI Specifications (`docs/0300-cli-specifications/`)

| ID | File | Title |
|----|------|-------|
| — | `README.md` | Series index |
| EVE-0300 | `eve-0300-cli-interface-specification.md` | CLI Interface Specification |
| EVE-0301 | `eve-0301-command-specification.md` | Command Specification |
| EVE-0302 | `eve-0302-argument-parsing-specification.md` | Argument Parsing Specification |
| EVE-0303 | `eve-0303-response-formatting-specification.md` | Response Formatting Specification |
| EVE-0304 | `eve-0304-interactive-shell-specification.md` | Interactive Shell Specification |
| EVE-0305 | `eve-0305-configuration-specification.md` | Configuration Specification |
| EVE-0306 | `eve-0306-logging-specification.md` | Logging Specification |
| EVE-0307 | `eve-0307-exit-code-specification.md` | Exit Code Specification |
| EVE-0308 | `eve-0308-plugin-command-specification.md` | Plugin Command Specification *(Reserved)* |
| EVE-0309 | `eve-0309-cli-session-specification.md` | CLI Session Specification *(Reserved)* |

### Part V — REST API (`docs/0400-rest-api-specifications/`)

| ID | File | Status |
|----|------|--------|
| — | `README.md` | Index |
| EVE-0400 | `eve-0400-rest-api-specification.md` | **Empty placeholder** |

### Part VI — Platform Specifications (`docs/0500-platform-specifications/`)

| ID | File | Status |
|----|------|--------|
| — | `README.md` | Index |
| EVE-0500 | `eve-0500-testing-specification.md` | **Empty placeholder** |
| EVE-0501 | `eve-0501-deployment-specification.md` | **Empty placeholder** |
| EVE-0502 | `eve-0502-build-system.md` | **Empty placeholder** |

### Part VII — Reference Catalogs (`docs/0600-reference-catalogs/`)

| ID | File | Title |
|----|------|-------|
| — | `README.md` | Series index |
| EVE-0600 | `eve-0600-discord-command-catalog.md` | Discord Command Catalog |
| EVE-0601 | `eve-0601-website-route-catalog.md` | Website Route Catalog |
| EVE-0602 | `eve-0602-cli-command-catalog.md` | CLI Command Catalog |
| EVE-0603 | `eve-0603-rest-api-endpoint-catalog.md` | REST API Endpoint Catalog |
| EVE-0604 | `eve-0604-capability-catalog.md` | Capability Catalog |
| EVE-0605 | `eve-0605-service-catalog.md` | Service Catalog |
| EVE-0606 | `eve-0606-knowledge-object-catalog.md` | Knowledge Object Catalog |
| EVE-0607 | `eve-0607-ai-provider-catalog.md` | AI Provider Catalog |
| EVE-0608 | `eve-0608-repository-catalog.md` | Repository Catalog |
| EVE-0609 | `eve-0609-document-catalog.md` | Document Catalog |

### Parts VIII–X — Planned (`docs/0700-*`, `0800-*`, `0900-*`)

Each contains **`README.md` only** listing planned document IDs (EVE-0700–0707, EVE-0800–0808, EVE-0900–0909).

### Part XI — Implementation Guides (`docs/1000-implementation-guides/`)

| ID | File | Title |
|----|------|-------|
| — | `README.md` | Series index |
| EVE-1000 | `eve-1000-implementation-roadmap.md` | Implementation Roadmap |
| EVE-1001 | `eve-1001-core-platform-implementation-guide.md` | Core Platform Implementation Guide |
| EVE-1002 | `eve-1002-validation-engine-implementation-guide.md` | Validation Engine Implementation Guide |
| EVE-1003 | `eve-1003-capability-engine-implementation-guide.md` | Capability Engine Implementation Guide |
| EVE-1004 | `eve-1004-service-layer-implementation-guide.md` | Service Layer Implementation Guide |
| EVE-1005 | `eve-1005-knowledge-object-implementation-guide.md` | Knowledge Objects Implementation Guide |
| EVE-1006 | `eve-1006-knowledge-graph-implementation-guide.md` | Knowledge Graph Implementation Guide |
| EVE-1007 | `eve-1007-knowledge-index-implementation-guide.md` | Knowledge Index Implementation Guide |
| EVE-1008 | `eve-1008-reasoning-pipeline-implementation-guide.md` | Reasoning Pipeline Implementation Guide |
| EVE-1009 | `eve-1009-context-assembly-implementation-guide.md` | Context Assembly Implementation Guide |
| EVE-1010 | `eve-1010-ai-provider-implementation-guide.md` | AI Provider Implementation Guide |
| EVE-1011 | `eve-1011-knowledge-engine-implementation-guide.md` | Knowledge Engine Implementation Guide |

### Part XII — Developer Guides (`docs/2000-developer-guides/`)

| ID | File | Status |
|----|------|--------|
| — | `README.md` | Index (lists all as Planned) |
| EVE-2000 | `eve-2000-command-line-interface-guide.md` | **Empty placeholder** |
| EVE-2001 | `eve-2001-rest-api-developer-guide.md` | **Empty placeholder** |
| EVE-2002 | `eve-2002-discord-integration-guide.md` | **Empty placeholder** |
| EVE-2003 | `eve-2003-website-integration-guide.md` | **Empty placeholder** |
| EVE-2004 | `eve-2004-provider-development-guide.md` | **Empty placeholder** |
| EVE-2005 | `eve-2005-capability-development-guide.md` | **Empty placeholder** |
| EVE-2006 | `eve-2006-service-development-guide.md` | **Empty placeholder** |
| EVE-2007 | `eve-2007-knowledge-repository-guide.md` | **Empty placeholder** |
| EVE-2008 | `eve-2008-testing-guide.md` | **Empty placeholder** |
| EVE-2009 | `eve-2009-release-process-guide.md` | **Empty placeholder** |

### Part XIII — Architecture Decision Records (`docs/3000-architecture-decision-records/`)

| ID | File | Title |
|----|------|-------|
| — | `README.md` | Series index |
| EVE-3001 | `eve-3001-documentation-first-development.md` | Documentation First Development |
| EVE-3002 | `eve-3002-capability-oriented-architecture.md` | Capability-Oriented Architecture |
| EVE-3003 | `eve-3003-knowledge-first-ai-architecture.md` | Knowledge-First AI Architecture |
| EVE-3004 | `eve-3004-validation-pipeline.md` | Validation Pipeline |
| EVE-3005 | `eve-3005-ai-provider-abstraction.md` | AI Provider Abstraction |
| EVE-3006 | `eve-3006-repository-as-source-of-truth.md` | Repository as Source of Truth |
| EVE-3007 | `eve-3007-knowledge-graph-architecture.md` | Knowledge Graph Architecture |
| EVE-3008 | `eve-3008-distributed-capability-network.md` | Distributed Capability Network *(Reserved)* |
| EVE-3009 | `eve-3009-provider-independent-reasoning.md` | Provider-Independent Reasoning |

### Part XIV — Archive (`docs/9000-archive/`)

| File | Status |
|------|--------|
| `README.md` | Index only (EVE-9000–9005 planned) |

---

# Markdown Document Format Style

EVE-SPEC documents follow a consistent structure. AI assistants should preserve this format when editing specs.

## Standard Document Header

```markdown
# EVE-XXXX

# Document Title

> Optional subtitle or tagline

---

## Document Information

| Property | Value |
|----------|-------|
| **Document ID** | EVE-XXXX |
| **Series** | NNNN — Series Name |
| **Title** | Full Title |
| **Project** | Evolutionary Virtual Engineer (E.V.E.) |
| **Specification** | EVE-SPEC |
| **Version** | 1.0.0 |
| **Status** | Active | Complete | Draft | Reserved |
| **Published** | July 2026 |

---

# Purpose
...
```

**Notes:**
- Two consecutive `#` H1 lines are intentional (ID line + title line).
- Body sections use `# Section Name` (H1) for major sections.
- Metadata tables use `## Document Information` or `## Series Information` for README indexes.

## Standard Navigation Block (Target Format)

```markdown
## Navigation

**Previous**

EVE-XXXX — Previous Document Title

**Current**

EVE-YYYY — Current Document Title

**Next**

EVE-ZZZZ — Next Document Title

**Return**

← Part N — Part Name

← EVE-SPEC
```

**Known variants in the corpus:**
- **`## Series Navigation`** — used in README indexes and 0300 CLI specs (legacy).
- **`**Part**` instead of `**Current**`** — used in EVE-0010–0012 and some 0600 catalogs (legacy).
- Series README files may omit `Return`.

## Series README Format

```markdown
# Part N

# NNNN Series Title

> Tagline

---

## Series Information
| Property | Value |
...

# Documents
| ID | Title | Status |
...

## Series Navigation
...
```

## Catalog Document Format

Catalogs (0600 series) use status emoji in tables:

| Emoji | Meaning |
|-------|---------|
| ✅ | Implemented |
| 🚧 | Planned |
| 🧪 | Experimental |
| ⚠ | Deprecated |
| ❌ | Removed |

## ADR Document Format

ADRs (3000 series) include sections: Context, Decision, Consequences, Related Architecture Decisions, Summary, Navigation.

## Implementation Guide Format

Guides (1000 series) include: Purpose, Architecture, Implementation, Testing Strategy, Relationship to EVE-SPEC, Navigation.

---

# C++ Code Map

## Layout Convention

```
include/eve/{subsystem}/   ← Public headers (.hpp)
src/eve/{subsystem}/         ← Implementation (.cpp)
tests/{subsystem}_test.cpp   ← GoogleTest files
```

Headers and sources mirror each other. Namespace: `eve::` with nested sub-namespaces (`eve::ai`, `eve::knowledge`, etc.).

## Subsystem Map

### `include/eve/core/` — Platform Contracts

| Header | Purpose |
|--------|---------|
| `types.hpp` | Strong ID types (`RequestId`, `CapabilityId`, `ProviderId`, etc.) |
| `errors.hpp` | Error types, `TraceEntry`, `Reference`, `DiagnosticMessage` |
| `platform_request.hpp` | `PlatformRequest`, `ValidatedPlatformRequest` |
| `platform_response.hpp` | `PlatformResponse`, `ValidatedPlatformResponse` |
| `core_platform.hpp` | `CorePlatform`, `PlatformBootstrap`, `CorePlatformDependencies` |

| Source | Purpose |
|--------|---------|
| `src/eve/core/platform_request.cpp` | Request factories and validation |
| `src/eve/core/platform_response.cpp` | Response factories |
| `src/eve/core/core_platform.cpp` | Top-level `process()` orchestration |
| `src/eve/core/platform_bootstrap.cpp` | DI wiring, index bootstrap |
| `src/eve/core/identifiers.cpp` | ID generation utilities |

### `include/eve/dispatcher/`

| Header | Source |
|--------|--------|
| `command_dispatcher.hpp` | `src/eve/dispatcher/command_dispatcher.cpp` |

### `include/eve/validation/`

| Header | Source |
|--------|---------|
| `validation_engine.hpp` | `validation_engine.cpp` |
| `validation_result.hpp` | `validation_result.cpp` |
| `validators.hpp` | `validators.cpp` |
| `parameter_schema.hpp` | `parameter_validator.cpp` |
| `policy_validator.hpp` | `policy_validator.cpp` |

### `include/eve/capability/`

| Header | Source |
|--------|--------|
| `capability_engine.hpp` | `capability_engine.cpp` |
| `handlers/capability_handlers.hpp` | `handlers/core_handlers.cpp`, `handlers/documentation_handlers.cpp` |

### `include/eve/services/`

| Header | Source |
|--------|--------|
| `service_interfaces.hpp` | — (interfaces only) |
| `service_implementations.hpp` | `configuration_service.cpp`, `logging_service.cpp`, `documentation_service.cpp`, `search_service.cpp`, `status_service.cpp`, `diagnostics_service.cpp` |

### `include/eve/knowledge/`

| Header | Source |
|--------|--------|
| `repository_discovery.hpp` | `repository_discovery.cpp` |
| `repository_index.hpp` | — |
| `markdown_parser.hpp` | `markdown_parser.cpp` |
| `document_model.hpp` | — |
| `document_metadata.hpp` | — |
| `metadata_extractor.hpp` | `metadata_extractor.cpp` |
| `knowledge_object.hpp` | `knowledge_object.cpp` |
| `knowledge_object_builder.hpp` | `knowledge_object_builder.cpp` |
| `relationship_builder.hpp` | `relationship_builder.cpp` |
| `knowledge_graph.hpp` | `knowledge_graph.cpp` |
| `knowledge_index.hpp` | `knowledge_index.cpp` |
| `search_engine.hpp` | `search_engine.cpp` |
| `ranking_engine.hpp` | `ranking_engine.cpp` |
| `citation_engine.hpp` | `citation_engine.cpp` |
| `reasoning_pipeline.hpp` | `reasoning_pipeline.cpp` |
| `in_memory_knowledge_store.hpp` | `in_memory_knowledge_store.cpp` |
| — | `document_indexer.cpp` *(no dedicated header)* |

### `include/eve/context/`

| Header | Source |
|--------|--------|
| `context_package.hpp` | `context_package.cpp` |
| `context_assembler.hpp` | `context_assembler.cpp` |
| — | `context_builder.cpp` *(Phase 1 legacy; header in context_package area)* |

### `include/eve/ai/`

| Header | Source |
|--------|--------|
| `provider_formatting.hpp` | `provider_formatter.cpp` |
| `provider_manager.hpp` | `provider_manager.cpp`, `null_provider.cpp` |
| `ollama_provider.hpp` | `ollama_provider.cpp` |
| `http_transport.hpp` | `http_transport.cpp` |

---

# Test Map (`tests/`)

| File | Tests |
|------|-------|
| `platform_request_test.cpp` | PlatformRequest contracts |
| `platform_response_test.cpp` | PlatformResponse contracts |
| `validation_engine_test.cpp` | Validation pipeline |
| `capability_engine_test.cpp` | CapabilityEngine |
| `contract_compliance_test.cpp` | Immutability, alias dispatch |
| `integration_test.cpp` | CorePlatform E2E (NullProvider) |
| `live_ollama_integration_test.cpp` | Live Ollama E2E (optional skip) |
| `knowledge_store_test.cpp` | InMemoryKnowledgeStore |
| `repository_discovery_test.cpp` | RepositoryDiscovery |
| `markdown_parser_test.cpp` | MarkdownParser |
| `metadata_extractor_test.cpp` | MetadataExtractor |
| `knowledge_object_builder_test.cpp` | KnowledgeObjectBuilder |
| `relationship_builder_test.cpp` | RelationshipBuilder |
| `knowledge_graph_test.cpp` | KnowledgeGraph |
| `knowledge_index_test.cpp` | KnowledgeIndex |
| `search_engine_test.cpp` | SearchEngine |
| `ranking_engine_test.cpp` | RankingEngine |
| `citation_engine_test.cpp` | CitationEngine |
| `context_assembler_test.cpp` | ContextAssembler |
| `context_package_test.cpp` | ContextPackage, ContextBuilder |
| `provider_formatter_test.cpp` | ProviderFormatter, OllamaProvider, ProviderManager |
| `CMakeLists.txt` | Test target registration |

**Total:** 172 tests (v0.6.0-alpha)

---

# Catalog ID Conventions (Runtime References)

These IDs appear in code, config, and catalogs. Do not confuse document IDs (EVE-XXXX) with runtime IDs.

| Prefix | Catalog | Example |
|--------|---------|---------|
| `CAP-` | Capability Catalog (EVE-0604) | `CAP-0102` = Documentation Search |
| `SVC-` | Service Catalog (EVE-0605) | `SVC-0001` = Command Dispatcher |
| `AI-` | AI Provider Catalog (EVE-0607) | `AI-0000` = Null, `AI-0100` = Ollama |
| `REQ-` | Platform Request IDs | Generated at runtime |
| `PKG-` | Context Package IDs | Generated at runtime |

---

# Common Path Confusions (Avoid These)

| Wrong / Outdated | Correct |
|------------------|---------|
| `docs/adr/` | `docs/3000-architecture-decision-records/` |
| `docs/0000/` | `docs/0000-core-platform-specifications/` |
| `docs/1000/eve-1000-...` | `docs/1000-implementation-guides/eve-1000-...` |
| `docs/0600/` | `docs/0600-reference-catalogs/` |
| `Prompt Builder` (old name) | `ProviderFormatter` (current code name) |
| `Context Builder` (pipeline terminus) | `ContextAssembler` (Phase 2) + reasoning engines |
| `Personality Specification` (filename) | Title is **Persona and Communication** (EVE-0008) |
| `Response Object Specification` (EVE-0011 nav) | Title is **Platform Response Specification** |

---

# AGENTS.md Reading Order (Actual Paths)

`AGENTS.md` uses shorthand. Map to real paths:

| AGENTS.md Reference | Actual Path |
|---------------------|-------------|
| `/docs/0000/README.md` | `docs/0000-core-platform-specifications/README.md` |
| Every specification in `/docs/0000/` | `docs/0000-core-platform-specifications/eve-0000-*.md` through `eve-0012-*.md` |
| `/docs/0600/README.md` | `docs/0600-reference-catalogs/README.md` |
| Every catalog in `/docs/0600/` | `docs/0600-reference-catalogs/eve-0600-*.md` through `eve-0609-*.md` |
| `/docs/1000/eve-1000-implementation-roadmap.md` | `docs/1000-implementation-guides/eve-1000-implementation-roadmap.md` |

**Recommended extended reading for v0.6.0-alpha implementation work:**

1. `docs/1000-implementation-guides/eve-1008-reasoning-pipeline-implementation-guide.md`
2. `docs/1000-implementation-guides/eve-1009-context-assembly-implementation-guide.md`
3. `docs/1000-implementation-guides/eve-1010-ai-provider-implementation-guide.md`
4. `docs/3000-architecture-decision-records/eve-3005-ai-provider-abstraction.md`
5. `docs/3000-architecture-decision-records/eve-3009-provider-independent-reasoning.md`

---

# Build Artifacts (Not in Source Tree)

| Path | Purpose |
|------|---------|
| `build/` | CMake out-of-tree build directory (gitignored) |
| `build/libeve_core.a` | Static library target |
| `build/tests/eve_tests.exe` | Test executable |

**Build commands:**

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build
```

---

# File Count Summary

| Area | Files |
|------|-------|
| EVE-SPEC markdown (`docs/`) | 89 |
| C++ headers (`include/eve/`) | 37 |
| C++ sources (`src/eve/`) | 42 |
| Test sources (`tests/`) | 22 |
| Root documentation | 10 |
| Config | 2 |
| Audits (v0.6.0-alpha) | 5 |
| Ideas (informal) | 1 |

---

*This map reflects repository state at v0.6.0-alpha. Update when series folders, naming conventions, or major layout changes occur.*
