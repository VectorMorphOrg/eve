# EVE-1009

# Context Assembly Implementation Guide

> Implementation Guide for the E.V.E. Context Assembly System

---

## Document Information

| Property | Value |
|----------|-------|
| **Document ID** | EVE-1009 |
| **Series** | 1000 — Implementation Guides |
| **Title** | Context Assembly Implementation Guide |
| **Project** | Evolutionary Virtual Engineer (E.V.E.) |
| **Specification** | EVE-SPEC |
| **Version** | 1.0.0 |
| **Status** | Active |
| **Published** | July 2026 |

---

# Purpose

This implementation guide describes the recommended
implementation of the Context Assembly subsystem used by the
Evolutionary Virtual Engineer (E.V.E.).

Context Assembly is the final deterministic stage of the
Reasoning Pipeline.

Its responsibility is to transform ranked engineering
evidence into a structured Context Package that can be
consumed by any AI provider without requiring additional
retrieval, ranking, or reasoning.

---

# Scope

This guide covers implementation of:

- Context Assembler
- Context Package construction
- Evidence selection
- Budget management
- Context policies
- Diagnostics
- Trace generation
- Thread safety

Behavioral requirements are defined by the Context Builder
Specifications.

---

# Responsibilities

The Context Assembler is responsible for:

- Building Context Packages
- Selecting engineering evidence
- Enforcing context budgets
- Preserving deterministic ordering
- Recording diagnostics
- Producing AI-ready context

The Context Assembler does **not**:

- Search repositories
- Rank results
- Generate citations
- Invoke AI providers
- Generate natural language
- Own conversation session persistence

Conversation history may be attached to a Context Package
after knowledge assembly. That attachment is performed by the
AI-invoking capability path (CAP-0102), not by the Context
Assembler knowledge-selection logic.

---

# Architecture

Context Assembly follows the Citation Engine and precedes AI
provider formatting.

```text
Ranked Results

↓

Citation Bundles

↓

Context Assembler

↓

Context Package

↓

Provider Formatter

↓

AI Provider
```

The assembler consumes deterministic evidence and produces
structured context.

---

# Context Package

The Context Package is the canonical interface between the
Reasoning Pipeline and AI Providers.

Typical contents include:

- Platform Request
- System Instructions
- Knowledge Objects
- Selected Sections
- Selected Headings
- Citations
- Navigation
- Repository Metadata
- Diagnostics
- Constraints

Every provider receives the same Context Package.

---

# Context Assembly Process

Recommended execution sequence:

1. Receive Ranked Search Results
2. Receive Citation Bundles
3. Resolve Knowledge Objects
4. Select engineering evidence
5. Apply context budget
6. Generate diagnostics
7. Produce Context Package

Assembly is entirely deterministic.

---

# Evidence Selection

Evidence should be processed in ranked order.

Typical evidence includes:

- Primary specifications
- Supporting specifications
- Document sections
- Headings
- Implementation headers
- Implementation sources
- Validation tests
- Navigation
- Cross references

Selection policies determine how much evidence is included.

---

# Context Policies

The assembler supports multiple deterministic policies.

## Full

Includes all available evidence within configured budgets.

Typical use:

- Deep engineering analysis
- Repository exploration
- Architecture review

---

## Balanced

Prioritizes engineering specifications while limiting
implementation detail.

Typical use:

- General documentation search
- AI provider requests
- Interactive engineering assistance

Balanced is the recommended default policy.

---

## Minimal

Produces the smallest useful Context Package.

Typical use:

- Low-latency requests
- Resource-constrained systems
- Quick lookups

---

# Budget Management

Context size is controlled through deterministic budgets.

Typical limits include:

- Maximum documents
- Maximum sections
- Maximum headings
- Maximum implementation files
- Maximum test files
- Maximum references
- Estimated token budget

Budgets prevent oversized provider requests.

---

# Token Estimation

Token estimation should remain deterministic.

Recommended approach:

```text
Estimated Tokens

=

(Character Count + 3)

/

4
```

Provider-specific tokenizers should not be required during
assembly.

---

# Ordering

Evidence should preserve deterministic ordering.

Recommended order:

```text
Primary Specification

↓

Supporting Specifications

↓

Sections

↓

Headings

↓

Implementation

↓

Validation

↓

Navigation

↓

References
```

Identical inputs should always produce identical Context
Packages.

---

# Diagnostics

Assembly should record:

- Selected documents
- Budget utilization
- Truncated evidence
- Missing references
- Missing citations
- Selection policy
- Estimated tokens

Diagnostics assist debugging and provider optimization.

---

# Execution Tracing

Context Assembly should emit structured trace entries.

Typical trace:

```text
Context Assembler

↓

Evidence Selected

↓

Budget Applied

↓

Context Package Created
```

Trace information becomes part of the Platform Response.

---

# Immutability

The Context Assembler should not modify:

- Ranked Results
- Citation Bundles
- Knowledge Objects
- Knowledge Graph
- Knowledge Index

The resulting Context Package should be an immutable
snapshot of the selected engineering evidence.

---

# Thread Safety

Context Assembly should support concurrent execution.

Recommendations:

- Stateless assembler
- Immutable inputs
- Immutable outputs
- Shared read-only repositories

Concurrent requests should never interfere with one another.

---

# Performance

Assembly should operate only on pipeline outputs.

Runtime work includes:

- Evidence selection
- Budget enforcement
- Context construction
- Diagnostics generation

No repository parsing or graph rebuilding should occur.

---

# Integration

The Context Assembler integrates directly with:

- Search Engine
- Ranking Engine
- Citation Engine
- Provider Formatter

It forms the boundary between deterministic reasoning and
AI language generation.

---

# Testing Strategy

Implementation should include:

- Context assembly tests
- Budget tests
- Policy tests
- Ordering tests
- Citation propagation tests
- Diagnostics tests
- Determinism tests
- Concurrency tests

Every policy should produce reproducible Context Packages.

---

# Relationship to EVE-SPEC

This guide implements:

- Context Builder Specification
- Context Package Specification
- Context Assembly Requirements

Related implementation guides include:

- EVE-1008 — Reasoning Pipeline Implementation Guide
- EVE-1010 — AI Provider Implementation Guide
- EVE-1011 — Knowledge Engine Implementation Guide

Architectural rationale is documented by:

- EVE-3003 — Knowledge-First AI Architecture
- EVE-3009 — Provider-Independent Reasoning

---

# Best Practices

Recommended engineering practices:

- Assemble context once.
- Preserve deterministic ordering.
- Respect configured budgets.
- Produce immutable Context Packages.
- Keep provider formatting separate.
- Record complete diagnostics.
- Keep engineering reasoning independent of AI providers.
- Attach conversation memory after knowledge assembly.
- Keep conversation memory outside Knowledge Objects.

---

# Conversation Memory Attachment

Knowledge assembly remains knowledge-first:

```text
Search
  → Rank
  → Cite
  → Assemble Context Package (knowledge)
```

When a non-empty `session_id` is present on the Platform
Request, CAP-0102 then:

```text
ConversationMemoryService::load(session_id)
  → ConversationContext
  → attach to Context Package
  → Provider Formatter
  → Provider Request
  → provider generation
```

On Success or PartialSuccess, CAP-0102 persists:

```text
memory.append(session_id, query, generated_text)
```

Conversation memory does not alter search queries, ranking,
citations, or knowledge budgets.

---

# Summary

The Context Assembler is the final deterministic stage of
the E.V.E. Reasoning Pipeline.

By transforming ranked engineering evidence into immutable
Context Packages, it enables provider-independent language
generation while preserving explainability, traceability,
and deterministic engineering reasoning.

---

## Navigation

**Previous**

EVE-1008 — Reasoning Pipeline Implementation Guide

**Current**

EVE-1009 — Context Assembly Implementation Guide

**Next**

EVE-1010 — AI Provider Implementation Guide

**Return**

← Part XI — Implementation Guides

← EVE-SPEC

---

*"Knowledge should evolve."*