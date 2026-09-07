# EVE-0007

# AI Provider Specification

> Engineering Specification for the E.V.E. AI Provider Layer

---

## Document Information

| Property | Value |
|----------|-------|
| **Document ID** | EVE-0007 |
| **Series** | 0000 — Engineering Specifications |
| **Title** | AI Provider Specification |
| **Project** | Evolutionary Virtual Engineer (E.V.E.) |
| **Version** | 2.0.0 |
| **Status** | Active |
| **Published** | July 2026 |

---

# Purpose

This specification defines the AI Provider Layer of the
Evolutionary Virtual Engineer (E.V.E.).

The AI Provider Layer is responsible for transforming a
provider-independent Provider Request into a natural-language
response.

Engineering reasoning is completed before the AI Provider is
invoked.

The provider consumes deterministic engineering context and
returns language.

---

# Design Philosophy

Artificial intelligence is one interchangeable component of
the E.V.E. architecture.

Engineering knowledge determines truth.

The Reasoning Pipeline determines relevance.

The Provider Formatter prepares provider-independent
requests.

The AI Provider generates natural language.

Replacing an AI Provider should require no architectural
changes outside the Provider Layer.

---

# Responsibilities

The AI Provider shall:

- Accept validated Provider Requests
- Generate natural-language responses
- Respect provider options
- Preserve engineering context
- Report provider diagnostics
- Return standardized AI Responses
- Report provider failures

The AI Provider shall not:

- Search documentation
- Read repositories
- Rank engineering evidence
- Generate citations
- Assemble Context Packages
- Construct prompts
- Modify engineering knowledge
- Invent undocumented information

---

# Architecture

The AI Provider Layer begins after deterministic reasoning
has completed.

```text
Reasoning Pipeline

↓

Context Package

↓

Provider Formatter

↓

Provider Request

↓

AI Provider

↓

AI Response

↓

Platform Response
```

The AI Provider is responsible only for language generation.

---

# Provider Request

Every AI Provider receives the same Provider Request.

A Provider Request may contain:

- System Instructions
- Platform Request
- Context Package
- Provider Capabilities
- Provider Options
- Metadata

Provider Requests are provider independent.

---

# AI Response

Every provider returns a normalized AI Response.

Typical response information includes:

- Generated text
- Provider identifier
- Model identifier
- Provider metadata
- Diagnostics
- Timing
- Error information

Platform interfaces consume normalized AI Responses rather
than provider-specific formats.

---

# Provider Interface

Every provider shall implement the common provider
interface.

Typical responsibilities include:

- Provider identification
- Capability advertisement
- Default options
- Health checking
- Response generation

Implementations may differ internally while preserving the
same logical contract.

---

# Provider Architecture

Each supported provider is implemented as an independent
adapter.

```text
Provider Interface

        │

        ├── Null Provider
        ├── Ollama
        ├── OpenAI
        ├── Anthropic
        ├── LM Studio
        ├── llama.cpp
        ├── Gemini
        └── Future Providers
```

Adapters translate between the canonical Provider Request
and provider-specific APIs.

---

# Provider Formatter

Prompt construction is performed by the Provider Formatter.

Responsibilities include:

- Formatting system instructions
- Formatting user requests
- Preserving engineering evidence
- Preserving citations
- Adapting to provider capabilities
- Producing Provider Requests

The Provider Formatter performs no engineering reasoning.

---

# Provider Capabilities

Providers advertise supported capabilities.

Examples include:

- System prompts
- Multiple messages
- Streaming
- Tool calling
- Function calling
- JSON responses
- Image input
- Reasoning mode

The Provider Formatter adapts automatically based upon these
capabilities.

---

# Provider Options

Provider configuration may include:

- Model
- Temperature
- Top-p
- Maximum tokens
- Context length
- Seed
- Stop sequences
- Timeout

These options remain provider independent whenever
possible.

---

# Provider Independence

Provider-specific behavior remains isolated.

Examples include:

- API serialization
- Authentication
- Model identifiers
- Network transport
- Streaming protocols
- Provider-specific diagnostics

These differences shall never affect deterministic
engineering reasoning.

---

# Error Handling

Providers shall detect and report:

- Provider unavailable
- Connection failure
- Timeout
- Invalid model
- Invalid response
- Authentication failure
- Rate limiting

Errors shall be normalized before reaching the Core
Platform.

---

# Determinism

The Provider Layer intentionally separates deterministic
engineering reasoning from probabilistic language
generation.

Deterministic components include:

- Search
- Ranking
- Citation generation
- Context Assembly
- Provider Formatting

Non-deterministic behavior begins only when the AI Provider
generates tokens.

---

# Streaming

The AI Provider Layer supports additive provider streaming.

## StreamChunk

A provider-independent incremental text event:

- `text_delta` — incremental assistant text
- `done` — whether this chunk signals stream completion

StreamChunks are transient callback events. Providers must not
retain the consumer after `generate_stream()` returns.

Streaming does not introduce asynchronous execution, threads,
cancellation tokens, or WebSocket transport.

## IAIProvider

Providers expose:

- `generate(ProviderRequest)` — synchronous generation
  (unchanged)
- `generate_stream(ProviderRequest, StreamConsumer)` —
  additive streaming generation

The default `generate_stream()` implementation reports that
streaming is unsupported via `ProviderError`.

Providers that advertise `supports_streaming = true` must
implement genuine streaming.

## ProviderManager

`ProviderManager::generate_stream()`:

1. Resolves the active provider
2. Formats the Context Package once through Provider Formatter
3. Dispatches to `IAIProvider::generate_stream()` when
   `supports_streaming` is true
4. Falls back to synchronous `generate()` when streaming is
   unsupported, then invokes the consumer once with
   `StreamChunk{ text_delta = generated_text, done = true }`
5. Returns the canonical `AIResponse`

ProviderManager does not parse provider-specific stream
payloads and does not write conversation memory.

## Implemented Providers

As of the v0.7.0-alpha streaming workstream:

| Provider | ID | `supports_streaming` | Behavior |
|----------|----|----------------------|----------|
| Null Provider | AI-0000 | true | Deterministic multi-chunk streaming |
| Ollama | AI-0100 | true | HTTP `send_stream` + NDJSON parsing |

## HTTP Transport Boundary

Streaming HTTP delivery belongs at `IHttpTransport`:

- Additive `send_stream()` delivers incremental HTTP body bytes
- Chunked Transfer-Encoding is decoded before the provider
  consumes body bytes
- The HTTP layer remains unaware of Ollama / NDJSON /
  StreamChunk semantics

## Memory Invariant

Streaming chunks are transient.

Providers and ProviderManager do not persist conversation
memory from stream deltas.

Only a completed canonical `AIResponse.generated_text` is
suitable for eventual memory persistence by an appropriate
caller (today: CAP-0102 synchronous path).

## Platform Surfaces

Provider-level streaming does not imply Discord, CLI, REST, or
Website streaming interfaces are implemented. Those remain
v0.8.x interface work.

## Known Limitations

- No async streaming runtime
- No cancellation
- No WebSocket streaming
- No streaming memory persistence
- No deterministic real TCP
  `SocketHttpTransport` → `OllamaProvider` integration fixture
- Live Ollama streaming is not part of the normal non-live
  suite
- HTTP trailer fields are not exposed
- Synchronous `send()` does not use the streaming chunked
  decoder (unchanged by design)

Streaming should remain transparent to Core Platform request
processing unless a future interface layer explicitly consumes
stream callbacks.

---

# Tool Integration

Future providers may support:

- Function Calling
- Tool Invocation
- Image Understanding
- Audio Processing
- External APIs

Tool integration should preserve provider independence.

---

# Multi-Provider Support

Future versions of E.V.E. may support:

- Local fallback
- Provider failover
- Cost optimization
- Model specialization
- Provider routing

Provider selection occurs before response generation.

---

# Security

AI Providers shall receive only validated Provider Requests.

Providers shall never receive:

- Repository credentials
- Platform secrets
- Hidden system state
- Internal implementation details

Only deterministic engineering context is transmitted.

---

# Future Expansion

Future revisions may support:

- Local model orchestration
- Multi-model consensus
- Distributed inference
- Offline operation
- Streaming providers
- Agent collaboration

These additions shall preserve provider interchangeability.

---

# Relationship to EVE-SPEC

This specification depends upon:

- EVE-0005 — Context Builder Specification
- EVE-0006 — Context Package Specification

Implementation guidance is provided by:

- EVE-1010 — AI Provider Implementation Guide

Architectural rationale is documented by:

- EVE-3005 — AI Provider Abstraction
- EVE-3009 — Provider-Independent Reasoning

---

# Summary

The AI Provider Layer is responsible solely for generating
natural language from deterministic engineering context.

Engineering reasoning is performed entirely by the E.V.E.
platform before the provider is invoked.

The Provider Formatter transforms Context Packages into
provider-independent Provider Requests, allowing every AI
backend to operate as a thin, replaceable adapter.

This architecture preserves deterministic engineering
reasoning, provider independence, and long-term
maintainability while allowing E.V.E. to evolve alongside
future AI technologies.

---

## Navigation

**Previous**

EVE-0006 — Context Package Specification

**Current**

EVE-0007 — AI Provider Specification

**Next**

EVE-0008 — Persona and Communication Specification

**Return**

← Part I — Engineering Specifications

← EVE-SPEC

---

*"Knowledge should evolve."*