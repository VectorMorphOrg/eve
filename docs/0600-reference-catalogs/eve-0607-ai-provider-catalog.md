# EVE-0607

# AI Provider Catalog

**Project:** Evolutionary Virtual Engineer (E.V.E.)

**Specification:** EVE-SPEC

**Part:** VII — Reference Catalogs

**Document Version:** 1.0.0

**Published:** July 2026

**Authors:** VectorMorph Research Initiative (VMRI)

**Status:** Draft

---

# Purpose

This document provides the authoritative catalog of
Artificial Intelligence (AI) providers supported by the
Evolutionary Virtual Engineer (E.V.E.).

Unlike engineering specifications, this catalog does not
define provider behavior or implementation.

Instead, it serves as the centralized inventory of AI
providers that may be integrated into the E.V.E. platform.

Behavioral requirements are defined within the AI Provider
Specification and related engineering specifications.

---

# Scope

This catalog includes:

- Cloud AI Providers
- Local AI Providers
- Self-Hosted AI Providers
- Embedding Providers
- Experimental Providers
- Future Providers

Additional providers may be added as E.V.E. evolves.

---

# Provider Status

| Status | Meaning |
|---------|----------|
| ✅ | Supported |
| 🚧 | Planned |
| 🧪 | Experimental |
| ⚠ | Deprecated |
| ❌ | Removed |

---

# Provider Categories

| Category | Description |
|----------|-------------|
| Cloud | Hosted commercial AI services |
| Local | Models executed on local hardware |
| Self-Hosted | User-managed inference servers |
| Embedding | Embedding and vector generation |
| Experimental | Research integrations |

---

# Cloud Providers

| ID | Provider | Description | Status |
|----|----------|-------------|--------|
| AI-0001 | OpenAI | Hosted language models | 🚧 |
| AI-0002 | Anthropic | Claude family of models | 🚧 |
| AI-0003 | Google | Gemini platform | 🚧 |
| AI-0004 | xAI | Grok platform | 🚧 |
| AI-0005 | Future Cloud Providers | Reserved | 🚧 |

---

# Local Providers

| ID | Provider | Description | Status |
|----|----------|-------------|--------|
| AI-0100 | Ollama | Local model runtime | 🚧 |
| AI-0101 | llama.cpp | Local GGUF inference | 🚧 |
| AI-0102 | LM Studio | Desktop inference platform | 🚧 |
| AI-0103 | Future Local Runtime | Reserved | 🚧 |

---

# Self-Hosted Providers

| ID | Provider | Description | Status |
|----|----------|-------------|--------|
| AI-0200 | OpenAI-Compatible API | Generic OpenAI API servers | 🚧 |
| AI-0201 | vLLM | High-performance inference server | 🚧 |
| AI-0202 | Text Generation Inference | Hugging Face inference server | 🚧 |
| AI-0203 | Custom Provider | User-defined provider | 🚧 |

---

# Embedding Providers

| ID | Provider | Description | Status |
|----|----------|-------------|--------|
| AI-0300 | Local Embeddings | Local embedding models | 🚧 |
| AI-0301 | OpenAI Embeddings | Cloud embeddings | 🚧 |
| AI-0302 | Sentence Transformers | Local semantic embeddings | 🚧 |
| AI-0303 | Future Embedding Provider | Reserved | 🚧 |

---

# Experimental Providers

| ID | Provider | Description | Status |
|----|----------|-------------|--------|
| AI-0400 | Research Provider | Experimental integrations | 🚧 |
| AI-0401 | Prototype Provider | Internal development | 🚧 |
| AI-0402 | Community Provider | Community-developed integrations | 🚧 |

---

# Reserved Provider Identifiers

| Range | Purpose |
|--------|---------|
| AI-0500–0599 | Future Cloud Providers |
| AI-0600–0699 | Future Local Providers |
| AI-0700–0799 | Future Self-Hosted Providers |
| AI-0800–0899 | Future Embedding Providers |
| AI-0900–0999 | Experimental Providers |

Reserved identifiers maintain a stable provider numbering
scheme as E.V.E. evolves.

---

# Provider Naming Guidelines

AI Providers should:

- Represent a single inference platform.
- Be implementation independent.
- Support standardized Provider Interfaces.
- Be replaceable without affecting platform architecture.
- Be documented before integration.

Provider implementations should remain interchangeable
whenever practical.

---

# Relationship to EVE-SPEC

This catalog complements the AI Provider Specification.

Engineering specifications define provider behavior and
integration requirements.

This document serves only as the authoritative inventory of
supported AI providers.

---

# Future Expansion

Future revisions of this catalog may include:

- Supported model families
- Context window limits
- Capability matrices
- Token limits
- Cost characteristics
- Latency metrics
- Supported modalities
- Authentication methods

---

# Summary

The AI Provider Catalog provides a centralized inventory of
the Artificial Intelligence providers supported by the
Evolutionary Virtual Engineer platform.

It establishes a common reference for provider integrations
while preserving the platform's AI provider independence.

---

## Navigation

**Previous**

EVE-0606 — Knowledge Object Catalog

**Part**

VII — Reference Catalogs

**Next**

EVE-0608 — Repository Catalog

**Return**

← Part VII — Reference Catalogs

← EVE-SPEC

---

## Motto

*"Knowledge should evolve."*