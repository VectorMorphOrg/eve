# EVE-0607

# AI Provider Catalog

> Authoritative Registry of E.V.E. AI Providers

---

## Document Information

| Property | Value |
|----------|-------|
| **Document ID** | EVE-0607 |
| **Series** | 0600 — Reference Catalogs |
| **Title** | AI Provider Catalog |
| **Project** | Evolutionary Virtual Engineer (E.V.E.) |
| **Version** | 2.0.0 |
| **Status** | Active |
| **Published** | July 2026 |

---

# Purpose

This document is the authoritative registry of Artificial
Intelligence (AI) Providers supported by the Evolutionary
Virtual Engineer (E.V.E.).

AI Providers generate natural-language responses from
provider-independent Provider Requests.

Unlike Engineering Specifications, this catalog does not
define provider behavior.

It exists solely to identify supported providers and assign
stable provider identifiers.

---

# Provider Status

| Status | Meaning |
|---------|----------|
| ✅ | Implemented |
| 🚧 | Planned |
| 🧪 | Experimental |
| ⚠ | Deprecated |
| ❌ | Removed |

---

# Provider Categories

| Category | Description |
|----------|-------------|
| Local | Models executed on local hardware |
| Cloud | Hosted AI services |
| Self-Hosted | User-managed inference servers |
| Embedding | Embedding generation |
| Experimental | Research providers |

---

# Internal / Testing Providers

| ID | Provider | Description | Status |
|----|----------|-------------|--------|
| AI-0000 | Null Provider | Deterministic no-op provider for testing and default configuration | ✅ |

---

# Local Providers

| ID | Provider | Description | Status |
|----|----------|-------------|--------|
| AI-0100 | Ollama | Local inference runtime | ✅ |
| AI-0101 | llama.cpp | Local GGUF inference | 🚧 |
| AI-0102 | LM Studio | Desktop inference platform | ✅ |
| AI-0103 | Future Local Runtime | Reserved | 🚧 |

---

# Cloud Providers

| ID | Provider | Description | Status |
|----|----------|-------------|--------|
| AI-0001 | OpenAI | Hosted language models | 🚧 |
| AI-0002 | Anthropic | Claude family | 🚧 |
| AI-0003 | Google Gemini | Gemini platform | 🚧 |
| AI-0004 | xAI | Grok platform | 🚧 |
| AI-0005 | Future Cloud Provider | Reserved | 🚧 |

---

# Self-Hosted Providers

| ID | Provider | Description | Status |
|----|----------|-------------|--------|
| AI-0200 | OpenAI-Compatible API | Generic OpenAI-compatible servers | 🚧 |
| AI-0201 | vLLM | High-performance inference server | 🚧 |
| AI-0202 | Hugging Face TGI | Text Generation Inference | 🚧 |
| AI-0203 | Custom Provider | User-defined implementation | 🚧 |

---

# Embedding Providers

| ID | Provider | Description | Status |
|----|----------|-------------|--------|
| AI-0300 | Local Embeddings | Local embedding models | 🚧 |
| AI-0301 | OpenAI Embeddings | Hosted embeddings | 🚧 |
| AI-0302 | Sentence Transformers | Local semantic embeddings | 🚧 |
| AI-0303 | Future Embedding Provider | Reserved | 🚧 |

---

# Experimental Providers

| ID | Provider | Description | Status |
|----|----------|-------------|--------|
| AI-0400 | Research Provider | Internal experimentation | 🚧 |
| AI-0401 | Prototype Provider | Prototype implementations | 🚧 |
| AI-0402 | Community Provider | Community-developed providers | 🚧 |

---

# Provider Capabilities

All providers expose a common Provider Interface.

Individual providers may advertise support for features
including:

- System Prompts
- Multiple Messages
- Streaming
- Tool Calling
- Function Calling
- JSON Responses
- Image Input
- Reasoning Mode

Capability differences are handled by the Provider
Formatter.

## Implemented Capability Matrix (current)

| Provider | ID | Streaming | System Prompts | Multiple Messages |
|----------|----|-----------|----------------|-------------------|
| Null Provider | AI-0000 | ✅ | ❌ | ❌ |
| Ollama | AI-0100 | ✅ | ✅ | ✅ |
| LM Studio | AI-0102 | ✅ | ✅ | ✅ |

Unimplemented catalog providers remain streaming-unsupported
until they provide a genuine `generate_stream()`
implementation.

AI-0102 uses OpenAI-compatible Chat Completions over local
HTTP (`/v1/chat/completions`), including synchronous
generation and genuine SSE streaming
(`supports_streaming = true`). Optional bearer API key
authentication is supported via provider-specific
`lm_studio_*` configuration. AI-0200 (generic OpenAI-
compatible API) remains a separate, unimplemented catalog
entry and is not an alias for AI-0102.

---

# Provider Architecture

Every provider consumes the same Provider Request.

```text
Context Package

↓

Provider Formatter

↓

Provider Request

↓

AI Provider

↓

AI Response
```

Providers perform language generation only.

Engineering reasoning is completed before provider
invocation.

---

# Reserved Provider Ranges

| Range | Purpose |
|--------|---------|
| AI-0500–0599 | Future Cloud Providers |
| AI-0600–0699 | Future Local Providers |
| AI-0700–0799 | Future Self-Hosted Providers |
| AI-0800–0899 | Future Embedding Providers |
| AI-0900–0999 | Experimental Providers |

Reserved identifiers preserve long-term compatibility.

---

# Provider Naming

Providers should:

- Represent one inference platform.
- Implement the common Provider Interface.
- Consume Provider Requests.
- Return standardized AI Responses.
- Remain interchangeable.
- Be documented before implementation.

Provider identifiers are stable public contracts.

---

# Relationship to EVE-SPEC

Behavior is defined by:

- EVE-0007 — AI Provider Specification

Implementation guidance is provided by:

- EVE-1010 — AI Provider Implementation Guide

This document serves only as the authoritative registry of
AI Providers.

---

# Current Implementation Status

As of **v0.6.0-alpha**, with unreleased v0.7 streaming and
additional-provider work on main, the following providers are
implemented:

- AI-0000 — Null Provider
  (`supports_streaming = true`; deterministic multi-chunk
  streaming)
- AI-0100 — Ollama
  (`supports_streaming = true`; HTTP streaming + NDJSON)
- AI-0102 — LM Studio
  (`supports_streaming = true`; local HTTP Chat Completions
  with synchronous generation and genuine SSE streaming;
  default base URL `http://localhost:1234`; optional
  `Authorization: Bearer` via `lm_studio_api_key`;
  ProviderManager dispatches streaming directly)

AI-0200 — OpenAI-Compatible API remains planned (generic
OpenAI-compatible servers). It is not implemented and is not
an alias for AI-0102.

Known provider/transport limitations for AI-0102:

- Local `http://` only with the current transport (no hosted
  HTTPS/TLS)
- No cancellation, async streaming, or WebSocket transport
- Streaming memory persistence remains deferred
- Discord/CLI/REST/Website streaming interfaces remain deferred

The provider architecture additionally includes:

- Provider Manager (`generate` and `generate_stream`)
- Provider Formatter
- HTTP Transport (`send` and additive `send_stream`, including
  chunked-transfer decoding on the streaming path)
- Provider Request model
- StreamChunk / StreamConsumer contract
- AI Response normalization

Additional providers may be implemented without changing the
Provider Interface.

---

# Future Expansion

Future revisions of this catalog may include:

- Supported model families
- Context window sizes
- Expanded capability matrix detail
- Tool support
- Vision support
- Authentication methods
- Performance benchmarks

---

# Summary

The AI Provider Catalog provides the authoritative registry
of AI Providers supported by E.V.E.

By standardizing provider identifiers while preserving a
common Provider Interface, E.V.E. allows new AI platforms to
be integrated as thin adapters without affecting the
deterministic engineering architecture.

---

## Navigation

**Previous**

EVE-0606 — Knowledge Object Catalog

**Current**

EVE-0607 — AI Provider Catalog

**Next**

EVE-0608 — Repository Catalog

**Return**

← Part VII — Reference Catalogs

← EVE-SPEC

---

*"Knowledge should evolve."*