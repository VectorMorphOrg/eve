# EVE-3009

# Provider-Independent Reasoning

> Architecture Decision Record

---

## Document Information

| Property | Value |
|----------|-------|
| **Document ID** | EVE-3009 |
| **Series** | 3000 — Architecture Decision Records |
| **Title** | Provider-Independent Reasoning |
| **Status** | Accepted |
| **Version** | 2.0.0 |
| **Published** | July 2026 |
| **Last Updated** | July 2026 |

---

# Decision

The Evolutionary Virtual Engineer (E.V.E.) adopts a
**Provider-Independent Reasoning** architecture.

All engineering reasoning shall be completed by the Core
Platform before any Artificial Intelligence provider is
invoked.

AI Providers are responsible solely for transforming
deterministic engineering context into natural-language
responses.

Changing AI providers shall never change the engineering
reasoning performed by the platform.

---

# Context

Many AI applications rely upon language models to perform
retrieval, reasoning, and response generation.

This tightly couples engineering behavior to the selected
provider and often results in:

- Different answers between providers
- Inconsistent engineering decisions
- Difficult debugging
- Poor reproducibility
- Vendor lock-in
- Limited explainability

E.V.E. instead performs engineering reasoning internally and
uses AI providers only for language generation.

---

# Decision Drivers

Provider-Independent Reasoning supports:

- Deterministic engineering behavior
- Consistent responses
- Explainable reasoning
- Provider independence
- Easier testing
- Simplified provider implementations
- Long-term maintainability

---

# Architecture

Engineering reasoning completes before language generation.

```text
Platform Request

↓

Validation Engine

↓

Capability Engine

↓

Knowledge Engine

↓

Search Engine

↓

Ranking Engine

↓

Citation Engine

↓

Context Builder

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

Every AI Provider receives the same deterministic Provider
Request.

---

# Separation of Responsibilities

## Core Platform

The Core Platform is responsible for:

- Validation
- Capability execution
- Repository discovery
- Knowledge retrieval
- Search
- Ranking
- Citation generation
- Context construction
- Engineering diagnostics

The Core Platform is the authoritative engineering
reasoning system.

---

## AI Providers

AI Providers are responsible for:

- Reading Provider Requests
- Understanding supplied context
- Generating natural-language responses
- Returning AI Responses

AI Providers are not responsible for:

- Repository discovery
- Search
- Ranking
- Citation generation
- Context construction
- Engineering decisions
- Capability execution

Providers communicate engineering knowledge—they do not
create it.

---

# Deterministic Reasoning

The Reasoning Pipeline shall remain deterministic.

Given identical:

- Repository contents
- Platform configuration
- Platform Request

the platform shall always produce identical:

- Knowledge retrieval
- Search results
- Ranking
- Citations
- Context Packages
- Provider Requests

Provider selection shall not affect these results.

---

# Provider Independence

Every provider consumes the same Provider Request model.

Supported providers may include:

- Null Provider
- Ollama
- OpenAI
- Anthropic
- Google Gemini
- LM Studio
- llama.cpp
- Future Providers

Differences between providers should primarily affect:

- Writing style
- Tone
- Latency
- Cost
- Streaming behavior

Engineering reasoning must remain identical.

---

# Provider Formatter

The Provider Formatter forms the architectural boundary
between deterministic engineering reasoning and AI language
generation.

Responsibilities include:

- Context serialization
- System instruction generation
- Provider capability adaptation
- Message construction
- Provider option application

The Provider Formatter never retrieves knowledge or performs
engineering reasoning.

---

# Context Package

The Context Package is the canonical engineering output of
the Core Platform.

Typical contents include:

- Knowledge Objects
- Citations
- Supporting evidence
- Repository metadata
- Constraints
- Diagnostics
- System instructions

The Provider Formatter transforms the Context Package into a
Provider Request suitable for the selected provider.

---

# Benefits

Provider-Independent Reasoning provides:

- Consistent engineering behavior
- Explainable responses
- Deterministic execution
- Easier testing
- Better debugging
- Stable architecture
- Provider portability
- Long-term maintainability

---

# Trade-Offs

Advantages include:

- Thin provider implementations
- Reliable engineering reasoning
- Easier provider replacement
- Improved traceability
- Better separation of responsibilities

Costs include:

- Larger Core Platform
- Additional preprocessing
- Increased implementation effort

These costs are acceptable because engineering correctness
takes precedence over architectural simplicity.

---

# Consequences

The Core Platform becomes the authoritative engineering
reasoning engine for E.V.E.

Future AI Providers may be added without modifying:

- Validation
- Capability execution
- Knowledge retrieval
- Search
- Ranking
- Citation generation
- Context Builder
- Knowledge Engine

Only the language-generation stage changes.

---

# Alternatives Considered

## Provider-Specific Reasoning

Allow each provider to perform independent engineering
reasoning.

Rejected because engineering behavior would vary between
providers.

---

## AI-Centric Architecture

Delegate retrieval and reasoning entirely to language
models.

Rejected because deterministic engineering behavior cannot
be guaranteed.

---

## Mixed Responsibility

Split engineering reasoning between the Core Platform and
AI Providers.

Rejected because responsibility boundaries become unclear
and difficult to validate.

---

# Scope

This Architecture Decision applies to:

- Knowledge Engine
- Reasoning Pipeline
- Search Engine
- Ranking Engine
- Citation Engine
- Context Builder
- Provider Formatter
- Provider Manager
- AI Providers
- Future provider implementations

---

# Relationship to EVE-SPEC

This Architecture Decision supports:

- EVE-0004 — Knowledge Layer Specification
- EVE-0005 — Context Builder Specification
- EVE-0006 — Context Package Specification
- EVE-0007 — AI Provider Specification

Implementation guidance is provided by:

- EVE-1008 — Reasoning Pipeline Implementation Guide
- EVE-1009 — Context Assembly Implementation Guide
- EVE-1010 — AI Provider Implementation Guide
- EVE-1011 — Knowledge Engine Implementation Guide

---

# Related Architecture Decisions

- EVE-3001 — Documentation First Development
- EVE-3002 — Capability-Oriented Architecture
- EVE-3003 — Knowledge-First AI Architecture
- EVE-3004 — Validation Pipeline
- EVE-3005 — AI Provider Abstraction
- EVE-3006 — Repository as Source of Truth
- EVE-3007 — Knowledge Graph Architecture

---

# Review

This Architecture Decision should be reviewed whenever the
boundary between the Core Platform, Reasoning Pipeline, or
AI Provider Layer undergoes significant architectural
changes.

Provider implementations may evolve independently provided
the deterministic Reasoning Pipeline remains the
authoritative engineering system.

---

# Summary

Provider-Independent Reasoning establishes the Core Platform
as the sole engineering reasoning engine within E.V.E.

Validation, knowledge retrieval, search, ranking, citation,
and context construction are completed before any AI
Provider is invoked.

The Provider Formatter converts deterministic Context
Packages into standardized Provider Requests, allowing AI
Providers to operate solely as interchangeable
language-generation components.

This separation ensures that engineering knowledge remains
deterministic, explainable, maintainable, and independent of
any particular AI provider.

---

## Navigation

**Previous**

EVE-3008 — Distributed Capability Network *(Reserved)*

**Current**

EVE-3009 — Provider-Independent Reasoning

**Next**

None

**Return**

← Part XIII — Architecture Decision Records

← EVE-SPEC

---

*"Knowledge should evolve."*