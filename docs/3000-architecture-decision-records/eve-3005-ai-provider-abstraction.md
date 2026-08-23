# EVE-3005

# AI Provider Abstraction

> Architecture Decision Record

---

## Document Information

| Property | Value |
|----------|-------|
| **Document ID** | EVE-3005 |
| **Series** | 3000 — Architecture Decision Records |
| **Title** | AI Provider Abstraction |
| **Status** | Accepted |
| **Version** | 2.0.0 |
| **Published** | July 2026 |
| **Last Updated** | July 2026 |

---

# Decision

The Evolutionary Virtual Engineer (E.V.E.) adopts a
**Provider-Independent AI Architecture**.

Language models shall be treated as interchangeable
language-generation engines behind a common provider
abstraction.

The Core Platform shall never depend upon any specific AI
vendor, model, API, runtime, or transport protocol.

Engineering reasoning remains entirely outside the Provider
Layer.

---

# Context

Many AI applications are tightly coupled to a single AI
provider.

Examples include:

- OpenAI-only implementations
- Anthropic-only implementations
- Ollama-specific prompt generation
- Vendor-specific response handling

These approaches tightly couple application architecture to
external services and make migration difficult.

E.V.E. is intended to remain operational regardless of which
AI provider is selected.

Provider replacement should never require changes to the
Core Platform.

---

# Decision Drivers

Provider Abstraction supports:

- Provider independence
- Long-term maintainability
- Simplified provider replacement
- Local and cloud deployment
- Stable engineering contracts
- Easier testing
- Consistent engineering behavior

---

# Architecture

The AI Provider Layer represents the final stage of the
engineering pipeline.

```text
Platform Request

↓

Reasoning Pipeline

↓

Context Package

↓

Provider Formatter

↓

Provider Request

↓

Provider Manager

↓

IAIProvider

↓

AI Provider

↓

AI Response

↓

Platform Response
```

All engineering reasoning completes before the AI Provider
is invoked.

---

# Provider Responsibilities

AI Providers are responsible for:

- Receiving Provider Requests
- Communicating with AI models
- Returning AI Responses
- Reporting provider diagnostics
- Supporting provider health checks

Providers are not responsible for:

- Repository indexing
- Search
- Ranking
- Citation generation
- Context assembly
- Validation
- Capability execution
- Prompt construction

Providers generate language only.

---

# Provider Manager

The Provider Manager owns and coordinates all registered AI
providers.

Responsibilities include:

- Provider registration
- Provider discovery
- Active provider selection
- Provider lifecycle
- Health monitoring
- Configuration management

The remainder of the platform is unaware of which provider
is currently active.

---

# Provider Formatter

The Provider Formatter is the authoritative source for
Provider Request construction.

Responsibilities include:

- Formatting system instructions
- Formatting user requests
- Serializing Context Packages
- Preserving engineering evidence
- Preserving citations
- Applying provider capabilities
- Applying provider options

Providers receive fully prepared Provider Requests.

---

# Provider Request

Every AI Provider consumes the same Provider Request model.

Typical contents include:

- System instructions
- User request
- Context Package
- Provider capabilities
- Provider options
- Metadata

Provider-specific serialization occurs only within provider
implementations.

---

# Provider Capabilities

Providers advertise supported capabilities through a common
capability model.

Typical capabilities include:

- System prompts
- Streaming
- Function calling
- Tool calling
- JSON responses
- Multiple messages
- Image input
- Reasoning mode

The Provider Formatter automatically adapts requests based
upon these advertised capabilities.

---

# Provider Independence

Every provider consumes the same Provider Request and
returns the same AI Response model.

Changing providers shall never affect:

- Search
- Ranking
- Citation generation
- Context assembly
- Validation
- Capability execution
- Platform behavior

Only language generation changes.

---

# Transport Independence

Network communication is isolated behind a transport
abstraction.

Responsibilities include:

- HTTP communication
- Authentication
- Serialization
- Timeouts
- Retry behavior

Transport implementation details remain hidden from the Core
Platform.

---

# Supported Providers

As of **v0.6.0-alpha**, implemented providers include:

- Null Provider
- Ollama Provider

Future providers may include:

- OpenAI
- Anthropic
- Google Gemini
- LM Studio
- llama.cpp
- OpenAI-Compatible APIs
- Additional local or hosted providers

All providers implement the same Provider Interface.

---

# Benefits

Provider Abstraction provides:

- Vendor independence
- Stable engineering architecture
- Thin provider implementations
- Simplified testing
- Easier mocking
- Cleaner separation of responsibilities
- Long-term maintainability

---

# Trade-Offs

Advantages include:

- Simplified provider integration
- Consistent platform behavior
- Easier provider replacement
- Improved portability

Costs include:

- Additional abstraction
- Provider adaptation logic
- Slight implementation overhead

These trade-offs are appropriate for a long-lived
engineering platform.

---

# Consequences

Replacing one AI provider with another changes only the
language-generation stage.

Engineering reasoning remains identical because:

- Search remains unchanged.
- Ranking remains unchanged.
- Citation generation remains unchanged.
- Context Assembly remains unchanged.
- Provider Requests remain standardized.

This guarantees deterministic engineering behavior across
all supported providers.

---

# Alternatives Considered

## Direct Provider Integration

Allow platform components to communicate directly with
individual AI providers.

Rejected because provider-specific logic becomes scattered
throughout the platform.

---

## Provider-Specific Prompt Construction

Allow each provider to generate its own prompts.

Rejected because engineering behavior becomes inconsistent
between providers.

---

## AI-Centric Architecture

Allow providers to retrieve repository information directly.

Rejected because engineering reasoning would become
provider-dependent and non-deterministic.

---

# Scope

This Architecture Decision applies to:

- Provider Manager
- Provider Formatter
- Provider Request
- AI Response
- AI Providers
- HTTP Transport
- Provider configuration
- Future provider implementations

---

# Relationship to EVE-SPEC

This Architecture Decision supports:

- EVE-0005 — Context Builder Specification
- EVE-0006 — Context Package Specification
- EVE-0007 — AI Provider Specification

Implementation guidance is provided by:

- EVE-1010 — AI Provider Implementation Guide

---

# Related Architecture Decisions

- EVE-3001 — Documentation First Development
- EVE-3002 — Capability-Oriented Architecture
- EVE-3003 — Knowledge-First AI Architecture
- EVE-3004 — Validation Pipeline
- EVE-3009 — Provider-Independent Reasoning

---

# Review

This Architecture Decision should be reviewed whenever the
Provider Interface, Provider Request model, transport
architecture, or Provider Capability model undergoes
significant architectural changes.

---

# Summary

AI Provider Abstraction establishes language models as
replaceable language-generation components within the E.V.E.
architecture.

The Core Platform completes deterministic engineering
reasoning before invoking an AI Provider.

The Provider Formatter converts Context Packages into
provider-independent Provider Requests, allowing every AI
provider to operate as a thin adapter responsible only for
language generation.

This architecture preserves provider independence,
engineering determinism, and long-term maintainability while
allowing E.V.E. to support local, cloud, and future AI
providers without modifying the engineering pipeline.

---

## Navigation

**Previous**

EVE-3004 — Validation Pipeline

**Current**

EVE-3005 — AI Provider Abstraction

**Next**

EVE-3006 — Repository as Source of Truth

**Return**

← Part XIII — Architecture Decision Records

← EVE-SPEC

---

*"Knowledge should evolve."*