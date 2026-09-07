# EVE-1010

# AI Provider Implementation Guide

> Implementation Guide for the E.V.E. AI Provider Architecture

---

## Document Information

| Property | Value |
|----------|-------|
| **Document ID** | EVE-1010 |
| **Series** | 1000 — Implementation Guides |
| **Title** | AI Provider Implementation Guide |
| **Project** | Evolutionary Virtual Engineer (E.V.E.) |
| **Specification** | EVE-SPEC |
| **Version** | 1.0.0 |
| **Status** | Active |
| **Published** | July 2026 |

---

# Purpose

This implementation guide describes the recommended
implementation of the AI Provider architecture used by the
Evolutionary Virtual Engineer (E.V.E.).

The AI Provider layer is responsible for transforming an
already-completed Context Package into natural-language
responses.

Engineering reasoning is intentionally excluded from this
layer.

The provider consumes deterministic engineering context and
returns language.

---

# Scope

This guide covers implementation of:

- AI Provider interface
- Provider Manager
- Provider Formatter
- Provider Request model
- Provider Response model
- HTTP transport
- Provider configuration
- Provider registration
- Provider selection

Behavioral requirements are defined by the AI Provider
Specifications.

---

# Responsibilities

The AI Provider layer is responsible for:

- Formatting provider requests
- Selecting the active provider
- Transporting requests
- Receiving provider responses
- Reporting provider diagnostics
- Supporting multiple AI backends

The provider layer does **not**:

- Search documentation
- Rank engineering evidence
- Generate citations
- Assemble context
- Modify repository knowledge
- Perform deterministic reasoning

---

# Architecture

The AI Provider layer begins after Context Assembly has
completed.

```text
Reasoning Pipeline

↓

Context Package

↓

Provider Formatter

↓

Provider Manager

↓

Selected Provider

↓

HTTP Transport

↓

Language Model

↓

AI Response

↓

Platform Response
```

Every provider receives the same Context Package.

---

# Provider Model

Every provider should implement the common provider
interface.

Typical responsibilities include:

- Provider identifier
- Provider name
- Capability advertisement
- Default options
- Health check
- Response generation

Providers remain interchangeable.

---

# Provider Manager

The Provider Manager coordinates all registered providers.

Responsibilities include:

- Register providers
- Activate providers
- Select providers
- Forward Provider Requests
- Return AI Responses

The Provider Manager is the only platform component that
knows which provider is active.

---

# Provider Formatter

The Provider Formatter converts Context Packages into
provider-independent Provider Requests.

Responsibilities include:

- Build system prompts
- Build user prompts
- Preserve citations
- Preserve engineering evidence
- Translate Conversation Context history into Provider Messages
- Adapt to provider capabilities
- Apply provider options

The formatter performs no engineering reasoning.

Providers do not own conversation sessions. Session history
enters the provider only as ordinary Provider Messages.

## Message Ordering

When Conversation Context is present, the formatter emits:

```text
System message (when supported)
  → historical Conversation Turns (oldest → newest)
  → current user request message
  → knowledge CONTEXT message(s)
```

Role mapping:

- ConversationRole::User → ProviderMessageRole::User
- ConversationRole::Assistant → ProviderMessageRole::Assistant

Historical message content is the exact Conversation Turn
content. The formatter does not summarize, rewrite, or label
history, and does not duplicate the current request into
history.

When Conversation Context is absent or empty, formatter
behavior matches the previous single-shot path.

---

# Provider Request

Every provider consumes the same Provider Request model.

Typical contents include:

- System instructions
- User request
- Context Package
- Provider options
- Provider capabilities
- Metadata

Providers should not receive raw repository data.

---

# Provider Response

Provider responses should be normalized before returning to
the Core Platform.

Typical response information includes:

- Generated text
- Provider identifier
- Model identifier
- Token statistics
- Timing information
- Diagnostics
- Error information

Normalization allows every provider to produce a common
AIResponse.

---

# Provider Capabilities

Each provider advertises its supported features.

Typical capabilities include:

- System prompts
- Multiple messages
- Streaming
- Function calling
- Tool calling
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

Options should remain provider independent whenever
possible.

---

# HTTP Transport

Network communication should be isolated from provider
logic.

Responsibilities include:

- HTTP requests
- HTTP responses
- Timeouts
- Connection errors
- Retry behavior

Providers should never implement networking directly.

## Streaming Transport

The transport contract is additive:

- `send()` — complete blocking response body
- `send_stream()` — incremental HTTP response body bytes via
  `HttpBodyConsumer`

`send_stream()` delivers decoded HTTP body bytes only. It must
not parse Ollama NDJSON or emit StreamChunks.

When `Transfer-Encoding: chunked` is present on the streaming
path, chunk framing is removed before body bytes are delivered
to the consumer. Content-Length and connection-close behavior
remain for non-chunked responses.

Synchronous `send()` remains unchanged and does not use the
streaming chunked decoder.

Known transport limitations:

- No async/event-loop transport
- HTTP trailer fields are not exposed
- No deterministic real TCP integration fixture currently
  exercises `SocketHttpTransport` → `OllamaProvider` together

---

# Streaming Generation

Providers may implement additive streaming:

```text
ProviderManager::generate_stream
        ↓
ProviderFormatter (once)
        ↓
IAIProvider::generate_stream
        ↓
IHttpTransport::send_stream   (network providers)
        ↓
Provider-specific stream parsing
        ↓
StreamConsumer (StreamChunk callbacks)
        ↓
canonical AIResponse
```

## StreamChunk

- `text_delta` — incremental assistant text
- `done` — completion signal

Callbacks are synchronous and non-retained.

## Current Implementations

- Null Provider — deterministic fixed-window multi-chunk
  streaming; shares `generate()` text for exact equivalence
- Ollama — `stream: true` request, NDJSON line buffering across
  arbitrary HTTP callback boundaries, terminal `done`
  semantics, metadata preserved on the final AIResponse

Synchronous `generate()` remains supported. Ollama
`generate()` continues to use `stream: false`.

Providers without streaming advertise
`supports_streaming = false`. ProviderManager falls back to
synchronous generation plus one final StreamChunk.

## Memory

Streaming providers must not write conversation memory.
StreamChunks are transient. Only final canonical
`generated_text` is suitable for eventual persistence by a
caller such as CAP-0102.

CAP-0102 itself remains a synchronous capability path.

---

# Provider Registration

Providers should register during platform bootstrap.

Typical sequence:

```text
Construct Provider

↓

Register Provider

↓

Validate Provider

↓

Advertise Capabilities

↓

Platform Ready
```

Registration should occur once during initialization.

---

# Provider Selection

The active provider is selected by configuration.

Example providers include:

- Null Provider
- Ollama
- OpenAI
- Anthropic
- LM Studio

Changing providers should not require modifications to the
Reasoning Pipeline.

---

# Determinism

The provider layer intentionally separates deterministic
reasoning from probabilistic language generation.

Deterministic components include:

- Search
- Ranking
- Citation generation
- Context assembly

Non-deterministic components include:

- Token generation
- Sampling
- Natural-language phrasing

This separation preserves engineering consistency while
allowing provider flexibility.

---

# Error Handling

Providers should report structured errors.

Typical failures include:

- Connection failure
- Timeout
- Invalid model
- Invalid response
- Authentication failure
- Provider unavailable

Errors should be normalized before reaching the Core
Platform.

---

# Thread Safety

Providers should support concurrent execution.

Recommendations:

- Stateless providers
- Immutable Provider Requests
- Independent HTTP requests
- No shared mutable conversation state

Provider implementations should be safe for parallel
requests.

---

# Performance

Provider execution is expected to dominate request latency.

Recommendations:

- Reuse HTTP connections where practical
- Avoid unnecessary serialization
- Share immutable formatter resources
- Separate network transport from formatting

Provider latency should not affect deterministic reasoning.

---

# Testing Strategy

Implementation should include:

- Provider registration tests
- Formatter tests
- Request serialization tests
- Response parsing tests
- HTTP transport tests
- HTTP streaming / chunked-decoding tests
- Provider streaming contract tests
- Health check tests
- Mock provider tests
- Live provider integration tests

Every provider should satisfy the common provider contract.
Live Ollama streaming is not part of the normal non-live
suite.

---

# Relationship to EVE-SPEC

This guide implements:

- AI Provider Specification
- Provider Manager Specification
- Provider Formatter Specification
- Context Package Specification

Related implementation guides include:

- EVE-1001 — Core Platform Implementation Guide
- EVE-1008 — Reasoning Pipeline Implementation Guide
- EVE-1009 — Context Assembly Implementation Guide
- EVE-1011 — Knowledge Engine Implementation Guide

Architectural rationale is documented by:

- EVE-3003 — Knowledge-First AI Architecture
- EVE-3005 — AI Provider Abstraction
- EVE-3009 — Provider-Independent Reasoning

---

# Best Practices

Recommended engineering practices:

- Keep providers thin.
- Never perform retrieval inside providers.
- Consume immutable Context Packages.
- Normalize all provider responses.
- Separate formatting from transport.
- Keep providers interchangeable.
- Preserve provider independence.

---

# Summary

The AI Provider layer is the language-generation boundary of
the E.V.E. platform.

By consuming deterministic Context Packages through a common
Provider Request model, every AI provider becomes a thin,
replaceable transport layer responsible only for generating
natural language.

This architecture preserves provider independence, enables
multiple AI backends, and ensures that engineering reasoning
remains entirely under the control of the E.V.E. platform.

---

## Navigation

**Previous**

EVE-1009 — Context Assembly Implementation Guide

**Current**

EVE-1010 — AI Provider Implementation Guide

**Next**

EVE-1011 — Knowledge Engine Implementation Guide

**Return**

← Part XI — Implementation Guides

← EVE-SPEC

---

*"Knowledge should evolve."*