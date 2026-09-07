# EVE-2004

# Provider Development Guide

> Contributor Guide for Adding and Maintaining AI Providers

---

## Document Information

| Property | Value |
|----------|-------|
| **Document ID** | EVE-2004 |
| **Series** | 2000 — Developer Guides |
| **Title** | Provider Development Guide |
| **Project** | Evolutionary Virtual Engineer (E.V.E.) |
| **Specification** | EVE-SPEC |
| **Version** | 0.1.0 |
| **Status** | Active |
| **Audience** | Contributors |

---

# 1. Purpose / Scope

This guide is for contributors who add or maintain an
**AI Provider** adapter in the Core Platform library.

In E.V.E., a provider is a thin adapter that:

- consumes a formatted `ProviderRequest`
- performs language generation for one backend
- returns a canonical `AIResponse` (or `ProviderError`)

Provider development does **not** mean inventing engineering
reasoning, search, ranking, citations, or memory policy.
Those remain outside the provider layer.

This document is **instructional**. Behavioral requirements
are defined by specifications and catalogs. Engineering
structure is described by implementation guides. Prefer those
authoritative documents over copying their content here.

| Need | Read |
|------|------|
| What providers must do | [EVE-0007](../0000-core-platform-specifications/eve-0007-ai-provider-specification.md) |
| How the provider layer is engineered | [EVE-1010](../1000-implementation-guides/eve-1010-ai-provider-implementation-guide.md) |
| Why provider independence exists | [EVE-3005](../3000-architecture-decision-records/eve-3005-ai-provider-abstraction.md) |
| Which provider IDs exist | [eve-0607](../0600-reference-catalogs/eve-0607-ai-provider-catalog.md) |

---

# 2. Before You Code

Do this before writing C++:

1. Read **EVE-0007** (provider contract and independence rules).
2. Read **EVE-1010** (Manager, Formatter, transport, streaming).
3. Check **eve-0607** for an existing catalog ID and status.
4. Read **EVE-3005** (and **EVE-3009** if reasoning-boundary
   questions arise).
5. Confirm whether the provider identity already exists.

**Do**

- Use the catalog ID already assigned (for example AI-0102).
- Follow existing provider patterns in-tree.

**Don't**

- Invent a new `ProviderId` without catalog authorization.
- Treat AI-0200 as implemented (it is not; see caveats).
- Duplicate Ollama protocol code into an OpenAI-compatible
  provider (or the reverse). Protocols stay provider-local.

Working examples in this repository:

| Provider | ID | Role |
|----------|----|------|
| NullProvider | AI-0000 | Deterministic / testing |
| OllamaProvider | AI-0100 | Local HTTP + NDJSON streaming |
| LMStudioProvider | AI-0102 | Local Chat Completions + SSE streaming |

---

# 3. Provider Architecture

High-level request path (contributor view):

```text
Platform Request
        ↓
Context Package / reasoning pipeline
        ↓
ProviderFormatter  →  ProviderRequest
        ↓
ProviderManager
        ↓
IAIProvider (selected backend)
        ↓
provider implementation (+ HTTP transport when needed)
        ↓
AIResponse  (or ProviderError)
```

Details live elsewhere — do not re-implement them in the
provider:

- Context Package shape → [EVE-0006](../0000-core-platform-specifications/eve-0006-context-package-specification.md)
- Context assembly → [EVE-1009](../1000-implementation-guides/eve-1009-context-assembly-implementation-guide.md)
- Provider Manager / Formatter / transport → [EVE-1010](../1000-implementation-guides/eve-1010-ai-provider-implementation-guide.md)

Providers consume an already-built `ProviderRequest`. They do
not assemble context or run CAP-0102 themselves.

---

# 4. Implementing a Provider

Practical checklist aligned with the current repository layout
(`include/eve/ai/`, `src/eve/ai/`, `tests/`, bootstrap).

## Code

- [ ] Add provider header under `include/eve/ai/`
- [ ] Add provider source under `src/eve/ai/`
- [ ] Implement `IAIProvider` (`id`, `name`, `capabilities`,
      `default_options`, `generate`)
- [ ] Use the catalog `ProviderId` (stable public contract)
- [ ] Implement synchronous `generate(ProviderRequest)`
- [ ] Map backend payload → canonical `AIResponse`
- [ ] Map failures → `ProviderError` (transport, HTTP non-2xx,
      malformed body, application errors)
- [ ] Advertise accurate `ProviderCapabilities`
- [ ] Wire CMake sources for `eve_core` / tests as needed

## Registration and configuration

- [ ] Register in platform bootstrap
      (`src/eve/core/platform_bootstrap.cpp` today registers
      Null, Ollama, and LM Studio)
- [ ] Define provider-specific configuration keys and defaults
- [ ] Keep secrets out of logs and diagnostics

## Documentation and tests

- [ ] Update **eve-0607** status / capability matrix when the
      provider is real
- [ ] Update status surfaces (PROJECT_STATUS, CHANGELOG) when
      authorized
- [ ] Add focused provider tests under `tests/`

**Formatting note:** `ProviderFormatter` builds the
`ProviderRequest` once. Providers should not re-invent prompt
assembly. Capability-driven formatting is owned by the
formatter (see EVE-1010).

**Do not invent** shared base classes or AI-0200 helpers unless
explicitly authorized. Keep protocol code provider-local.

---

# 5. Streaming Providers

Streaming is additive. Normative/engineering detail:
[EVE-1010 — Streaming Generation](../1000-implementation-guides/eve-1010-ai-provider-implementation-guide.md).

Contributor workflow when a provider truly streams:

1. Implement `generate_stream(request, StreamConsumer)`.
2. Set `supports_streaming = true` **only after** genuine
   streaming works and is tested.
3. Emit incremental `StreamChunk{ text_delta, done=false }` for
   content only.
4. Emit exactly one terminal `StreamChunk{ "", done=true }` last.
5. Return a canonical `AIResponse` whose `generated_text` is the
   ordered concatenation of content deltas.
6. On any mid-stream failure, return `ProviderError` (do not
   treat partial output as success).

`ProviderManager::generate_stream` formats once, then:

- calls the provider’s `generate_stream` when
  `supports_streaming` is true, or
- falls back to synchronous `generate` plus one final
  `done=true` chunk when streaming is not supported.

## Protocol ownership

| Layer | Responsibility |
|-------|----------------|
| `IHttpTransport::send_stream` | HTTP + incremental **body bytes** |
| Provider | Protocol parsing → `StreamChunk` / `AIResponse` |

The HTTP transport must **not** learn SSE, NDJSON, `[DONE]`,
or vendor JSON shapes.

Transport callbacks are **not** guaranteed to align with
protocol lines or events. Buffer across arbitrary boundaries.

## Current protocol examples

| Provider | Streaming shape |
|----------|-----------------|
| Ollama (AI-0100) | `stream: true`, **NDJSON** lines, terminal `done` |
| LM Studio (AI-0102) | Chat Completions `stream: true`, **SSE** (`data:` / `[DONE]`) |
| Null (AI-0000) | In-process deterministic chunks (no HTTP protocol) |

Do not mix these parsers. A second OpenAI-compatible backend
may later share patterns with LM Studio, but **AI-0200 is not
implemented** and must not be assumed.

---

# 6. Memory Boundary

Providers do **not** own conversation-memory persistence.

- Do not write to `ConversationMemoryService` from a provider.
- `StreamChunk` values are transient; do not retain them.
- CAP-0102 persists successful-generation text on the Core /
  platform request path after generation succeeds.
- Chunk-by-chunk / streaming-specific memory persistence is
  **not** implemented.

See project memory/status documentation and the memory notes in
[EVE-1010](../1000-implementation-guides/eve-1010-ai-provider-implementation-guide.md)
rather than duplicating the service design here.

---

# 7. Testing Workflow

Canonical build/test entry points are documented in the root
[README.md](../../README.md) (`cmake` configure/build/`ctest`).

For provider work, contributors typically:

1. Build `eve_core` and `eve_tests`.
2. Run focused provider tests (sync + stream suites).
3. Run the full non-live suite:

```text
eve_tests --gtest_filter=-LiveOllama*
```

Current expected non-live baseline:

| Result | Count |
|--------|-------|
| Total | 278 |
| PASS | 275 |
| FAIL | 3 |

The three failures are known `RepositoryDiscoveryTest` cases.
They are **not** provider regressions. Do not “fix” them as
part of a provider change unless that work is separately
authorized.

## What to cover in provider tests

- [ ] Identity / capabilities (including streaming flag)
- [ ] Request serialization (`stream` true/false where applicable)
- [ ] Successful response parsing → `AIResponse`
- [ ] Transport failure / HTTP non-2xx / malformed payload /
      application errors → `ProviderError`
- [ ] Streaming: ordered chunks, terminal `done=true` last,
      split lines/events across fake transport callbacks
- [ ] `ProviderManager` dispatch (sync and/or stream)
- [ ] Authorization/header behavior when the provider supports
      optional credentials

Prefer deterministic fake `IHttpTransport` fixtures (see
existing Ollama / LM Studio tests). No sleeps. No live network
in the default suite.

---

# 8. Configuration / Registration

Providers are constructed and registered during platform
bootstrap (`src/eve/core/platform_bootstrap.cpp`).

Active-provider selection remains configuration-driven (see
EVE-1010). Default active provider behavior is defined by the
platform configuration path — do not hard-code a new default
without authorization.

## Example: LM Studio (AI-0102)

LM Studio illustrates a current local HTTP provider:

| Concern | Example |
|---------|---------|
| Endpoint | `{base}/v1/chat/completions` |
| Default base URL | `http://localhost:1234` |
| Sync | `stream: false` via `generate()` |
| Stream | `stream: true` + SSE via `generate_stream()` |
| Optional auth | `Authorization: Bearer …` when configured |
| Config keys | `lm_studio_base_url`, `lm_studio_model`,
  `lm_studio_temperature`, `lm_studio_top_p`,
  `lm_studio_timeout_ms`, `lm_studio_api_key` |

Use provider-prefixed keys. Do not reuse another provider’s
configuration namespace.

---

# 9. Documentation / Catalog Checklist

Before calling a provider “done,” verify only what changed:

- [ ] **eve-0607** — ID, status, capability matrix if streaming
- [ ] **eve-0604** — only if capability/status notes need the
      provider called out
- [ ] **EVE-0007 / EVE-1010** — update only when behavior or
      engineered guidance actually changed
- [ ] **PROJECT_STATUS** / **CHANGELOG** — when authorized
- [ ] Roadmap/status wording — remove “planned” claims that are
      no longer true

Avoid parallel write-ups that restate EVE-0007 or EVE-1010.
This guide (EVE-2004) is the contributor workflow entry point.

---

# 10. Current V0.7 Caveats

Honest limits of the current tree:

- HTTP transport is oriented to local `http://` URLs
- **HTTPS/TLS is not implemented**
- No async streaming runtime
- No stream cancellation
- No WebSocket transport
- No streaming-specific memory persistence
- Discord / CLI / REST / Website binaries remain **v0.8.x**
- **AI-0200** (generic OpenAI-compatible API provider) remains
  **unimplemented** and is **not** an alias for AI-0102
- Hosted OpenAI / Anthropic providers remain future work
- Shared OpenAI-compatible helper extraction remains deferred

---

# 11. Completion Checklist

Use this before submitting a provider change:

- [ ] Catalog ID confirmed; no invented `ProviderId`
- [ ] EVE-0007 / EVE-1010 / eve-0607 / EVE-3005 reviewed
- [ ] `IAIProvider` sync path complete and mapped to
      `AIResponse` / `ProviderError`
- [ ] Streaming either genuinely implemented
      (`supports_streaming = true`) or explicitly unsupported
- [ ] Protocol parsing stays inside the provider
- [ ] No provider memory writes
- [ ] Bootstrap registration + config keys wired
- [ ] Focused tests + non-live suite green aside from the three
      known `RepositoryDiscoveryTest` failures
- [ ] Catalog / status / changelog updated when authorized
- [ ] Caveats above respected (no silent TLS/async/AI-0200 claims)

---

# Relationship to EVE-SPEC

This guide supports contributors working on the provider layer.

Authoritative behavior and engineering detail remain in:

- [EVE-0007](../0000-core-platform-specifications/eve-0007-ai-provider-specification.md)
- [EVE-1010](../1000-implementation-guides/eve-1010-ai-provider-implementation-guide.md)
- [eve-0607](../0600-reference-catalogs/eve-0607-ai-provider-catalog.md)
- [EVE-3005](../3000-architecture-decision-records/eve-3005-ai-provider-abstraction.md)
- [EVE-3009](../3000-architecture-decision-records/eve-3009-provider-independent-reasoning.md)

Related context documents:

- [EVE-0006](../0000-core-platform-specifications/eve-0006-context-package-specification.md)
- [EVE-1009](../1000-implementation-guides/eve-1009-context-assembly-implementation-guide.md)
- [eve-0604](../0600-reference-catalogs/eve-0604-capability-catalog.md)

---

## Navigation

**Previous**

[Part XII — Developer Guides](README.md)

**Current**

EVE-2004 — Provider Development Guide

**Next**

EVE-2005 — Capability Development Guide

**Return**

← [Part XII — Developer Guides](README.md)

← EVE-SPEC

---

*"Knowledge should evolve."*
