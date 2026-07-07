# EVE-0005

# Context Builder Specification

**Project:** E.V.E. (Evolutionary Virtual Engineer)

**Document Version:** 1.0.0

**Published:** July 2026

**Authors:** VectorMorph Research Initiative (VMRI)

**Status:** Foundational Specification

---

# Purpose

This document defines the Context Builder of E.V.E.

The Context Builder transforms structured knowledge into
high-quality context suitable for downstream AI providers.

The Context Builder is responsible for deciding what
information should be presented—not how that information is
expressed.

---

# Design Philosophy

Knowledge retrieval and language generation are separate
problems.

The Knowledge Layer retrieves information.

The Context Builder selects, organizes, validates, and
prepares that information.

The AI Provider generates natural language.

---

# Responsibilities

The Context Builder is responsible for:

- Selecting relevant knowledge
- Ranking retrieved documents
- Building structured context
- Preserving document hierarchy
- Removing duplicate information
- Enforcing context limits
- Preparing citations
- Maintaining deterministic ordering

The Context Builder never generates answers.

---

# Inputs

The Context Builder receives:

- User request
- Capability request
- Knowledge Objects
- Repository metadata
- Configuration
- System limits

---

# Outputs

The Context Builder produces a structured Context Package.

A Context Package may contain:

- Relevant documents
- Relevant sections
- Citations
- Metadata
- Repository information
- Conversation context
- System instructions

The Context Package is consumed by the AI Provider.

---

# Context Selection

Only relevant knowledge should be selected.

Priority should favor:

- Official specifications
- RFCs
- Architecture documents
- Documentation
- Whitepapers

Lower priority sources should never replace
authoritative documentation.

---

# Ranking

Context should be ranked using objective criteria.

Examples include:

- Document relevance
- Repository priority
- Specification status
- Version
- User intent

Future ranking algorithms may evolve without changing the
overall architecture.

---

# Context Assembly

The Context Builder should assemble information into a
logical structure.

Example

User Question

↓

Relevant Documents

↓

Relevant Sections

↓

Metadata

↓

Citations

↓

Context Package

---

# Context Limits

Every AI Provider has finite context capacity.

The Context Builder is responsible for:

- Truncation
- Prioritization
- Compression
- Deduplication

Important information should never be removed before less
important information.

---

# Citation Preservation

Whenever possible, context should retain references to the
original documentation.

Examples:

- EVE-0001
- AEG-0000
- RFC-0005

Future implementations may support section-level citations.

---

# Conversation Awareness

Future versions may incorporate conversation history.

Conversation history should supplement documentation.

It should never override documented knowledge.

---

# Determinism

Given identical inputs, the Context Builder should produce
identical Context Packages whenever practical.

Deterministic behavior improves reproducibility and testing.

---

# AI Independence

The Context Builder should remain completely independent of
the AI Provider.

It should not contain:

- Prompt engineering specific to one provider
- Provider APIs
- Model-specific logic

Its output should be portable.

---

# Error Handling

If insufficient documentation exists, the Context Builder
should produce an incomplete Context Package rather than
inventing information.

Missing knowledge should be explicitly identified.

---

# Future Expansion

Future versions may support:

- Semantic retrieval
- Embedding ranking
- Knowledge graphs
- Cross-repository relationships
- User personalization
- Session memory
- Multi-agent context sharing

These capabilities should preserve deterministic behavior
whenever possible.

---

# Context Package

Although implementation details are intentionally left open,
a Context Package should conceptually contain:

- User Request
- Capability
- Relevant Knowledge Objects
- Repository Metadata
- Citations
- Constraints
- System Instructions

The Context Package represents the complete body of
information supplied to the AI Provider.

---

# Summary

The Context Builder is the bridge between documented
knowledge and natural language generation.

It determines what information should be considered.

It never determines what information is true.

Truth originates from documentation.

Artificial intelligence is responsible only for expressing
that truth in a useful manner.

---

# Motto

> "Knowledge should evolve."

---

© 2026 VectorMorph Research Initiative (VMRI)

Released under the MIT License unless otherwise noted.