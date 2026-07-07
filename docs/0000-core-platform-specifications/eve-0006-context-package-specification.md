# EVE-0006

# Context Package Specification

**Project:** E.V.E. (Evolutionary Virtual Engineer)

**Document Version:** 1.1.0

**Published:** July 2026

**Authors:** VectorMorph Research Initiative (VMRI)

**Status:** Foundational Specification

---

# Purpose

This document defines the Context Package used throughout
E.V.E.

A Context Package represents the complete body of
information supplied to an AI Provider for a single request.

It is the standardized interface between the Context Builder
and every supported AI Provider.

Artificial Intelligence shall never access project
documentation directly.

All knowledge supplied to an AI Provider shall be mediated by
the Context Builder through a validated Context Package.

---

# Design Philosophy

The Context Package exists to separate:

- Knowledge Retrieval
- Context Assembly
- Artificial Intelligence

Knowledge determines truth.

The Context Package determines relevance.

Artificial Intelligence determines presentation.

---

# Responsibilities

A Context Package shall:

- Represent one request
- Be immutable after creation
- Contain all relevant knowledge
- Preserve citations
- Preserve repository metadata
- Preserve response constraints
- Remain provider independent
- Remain language independent
- Be serializable
- Be versioned

---

# Lifecycle

```
User Request

↓

Capability Selection

↓

Knowledge Layer

↓

Context Builder

↓

Context Package

↓

Validation

↓

AI Provider

↓

Natural Language Response
```

Only validated Context Packages should be delivered to an
AI Provider.

---

# Canonical Context Package

Every implementation shall logically construct a Context
Package containing the following components.

Implementations may use different programming languages,
serialization formats, or internal data structures provided
the logical structure and semantics remain equivalent.

```
ContextPackage
│
├── metadata
├── request
├── capability
├── repositories
├── knowledge_objects
├── citations
├── conversation
├── constraints
├── system
└── diagnostics
```

---

# Metadata

Metadata describes the Context Package itself.

Examples include:

- Package Identifier
- Request Identifier
- Timestamp
- Specification Version
- Builder Version

Metadata supports traceability and debugging.

---

# Request

The Request section preserves the original user request.

Examples include:

- Original Input
- Normalized Input
- Interface
- User Identifier (optional)

The original request shall always be preserved.

---

# Capability

The Capability section identifies the primary system
capability responsible for handling the request.

Examples include:

- Documentation Search
- Repository Lookup
- Status Report
- Specification Summary

One primary capability shall always exist.

Secondary capabilities are optional.

---

# Repositories

Repositories describe the sources of retrieved knowledge.

Examples include:

- Repository Name
- Branch
- Commit
- Version
- Last Updated

Multiple repositories may contribute knowledge.

---

# Knowledge Objects

Knowledge Objects contain the authoritative information
selected by the Context Builder.

Examples include:

- EVE Specifications
- AEG Specifications
- RFCs
- Whitepapers
- README Files
- CHANGELOGs

Knowledge Objects shall never be modified by an AI Provider.

---

# Citations

Every authoritative statement should reference its
originating documentation whenever practical.

Examples include:

- EVE-0001
- EVE-0004
- AEG-0000
- RFC-0003

Future implementations may support section-level citations.

---

# Conversation

Conversation context is optional.

When enabled it may include:

- Session Summary
- Recent Messages
- Conversation Metadata

Conversation context supplements documentation.

It never overrides documentation.

---

# Constraints

Constraints define behavioral expectations for the AI
Provider.

Examples include:

- Documentation First
- Preserve Citations
- Do Not Invent Information
- Response Length
- Output Format

Constraints should remain implementation independent.

---

# System

System Instructions provide platform-level behavior.

Examples include:

- Personality Profile
- Language
- AI Provider
- Provider Options

System Instructions remain independent of the user request.

---

# Diagnostics

Diagnostics assist development and debugging.

Examples include:

- Validation Warnings
- Missing Documentation
- Confidence Metrics
- Builder Notes

Diagnostics should never affect factual correctness.

---

# Validation

Before delivery every Context Package should be validated.

Validation may include:

- Missing Knowledge
- Duplicate Citations
- Invalid Metadata
- Empty Context
- Invalid Version
- Missing Capability

Invalid Context Packages shall not be delivered to an
AI Provider.

---

# Object Rules

Every Context Package shall satisfy the following
requirements.

- Immutable
- Deterministic
- Serializable
- Self-Contained
- Provider Independent
- Language Independent
- Versioned

---

# Package Lifecycle

```
Created

↓

Validated

↓

Delivered

↓

Consumed

↓

Archived
```

If validation fails:

```
Created

↓

Validation Failed

↓

Discarded
```

---

# Serialization

The internal storage format is intentionally unspecified.

Possible implementations include:

- JSON
- YAML
- Protocol Buffers
- Native Objects

All implementations shall preserve the canonical logical
structure.

---

# Compatibility

Future revisions should maintain backwards compatibility
whenever practical.

New fields may be added.

Existing fields should not change semantic meaning.

Deprecated fields should remain supported for at least one
major specification revision.

---

# Stable Identifiers

Context Packages should expose stable identifiers.

Examples include:

```
CTX-20260706-000001

REQ-20260706-000001

KO-00000025
```

Identifier formats may evolve while preserving uniqueness.

---

# Provider Independence

Every AI Provider shall receive equivalent Context Packages.

Examples include:

- OpenAI
- Anthropic
- Ollama
- llama.cpp
- Local Models
- Future Providers

Changing providers should not require changes to the Context
Builder.

---

# Future Expansion

Future revisions may support:

- Semantic Embeddings
- Knowledge Graph References
- Tool Invocation Metadata
- Multi-Agent Coordination
- User Preferences
- Long-Term Memory
- Confidence Scoring

These additions should preserve compatibility with existing
Context Packages.

---

# Summary

The Context Package is the standardized communication
contract between E.V.E. and every supported AI Provider.

It contains everything necessary to answer a request while
remaining independent of programming language,
implementation, deployment model, or AI Provider.

By standardizing the Context Package, E.V.E. maintains a
clear separation between knowledge management and language
generation, ensuring long-term portability and
maintainability.

---

# Motto

> "Knowledge should evolve."

---

© 2026 VectorMorph Research Initiative (VMRI)

Released under the MIT License unless otherwise noted.