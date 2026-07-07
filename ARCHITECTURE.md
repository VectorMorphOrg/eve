# E.V.E. Architecture

> High-level architecture of the Evolutionary Virtual Engineer (E.V.E.) platform.

---

# Project Information

| Property | Value |
|----------|-------|
| **Project** | Evolutionary Virtual Engineer (E.V.E.) |
| **Repository** | VectorMorphOrg/eve |
| **Current Architecture** | v0.6.0-alpha |
| **Specification** | EVE-SPEC |
| **Language** | C++23 |
| **Architecture Style** | Layered, Capability-Oriented, Deterministic |

---

# Purpose

This document provides a high-level overview of the E.V.E.
architecture.

Unlike the EVE-SPEC documents, this guide explains how the
major subsystems interact during runtime.

It is intended for:

- New contributors
- Developers
- Reviewers
- Architecture discussions

It intentionally omits low-level implementation details.

---

# Engineering Philosophy

E.V.E. follows several core principles.

- Documentation First
- Specification Before Implementation
- Deterministic Reasoning
- Knowledge First
- Capability-Oriented Architecture
- AI Provider Independence
- Interface Independence
- Explainable Results
- Long-Term Maintainability

The platform retrieves and organizes engineering knowledge
before invoking an AI model.

Language models generate responses.

They do not perform retrieval.

---

# High-Level Architecture

```text
                Interfaces
────────────────────────────────────

 CLI
 Discord
 REST API
 Website
 Future Interfaces

────────────────────────────────────
            Core Platform
────────────────────────────────────

 Platform Request

 Validation Engine

 Capability Engine

 Service Layer

 Platform Response

────────────────────────────────────
        Deterministic Reasoning
────────────────────────────────────

 Search Engine

 Ranking Engine

 Citation Engine

 Context Assembler

────────────────────────────────────
         Knowledge Engine
────────────────────────────────────

 Repository Discovery

 Markdown Parser

 Metadata Extraction

 Knowledge Object Builder

 Relationship Builder

 Knowledge Graph

 Knowledge Index

────────────────────────────────────
          AI Provider Layer
────────────────────────────────────

 Provider Formatter

 Provider Manager

 Ollama

 OpenAI

 Anthropic

 LM Studio

────────────────────────────────────
         Language Model
────────────────────────────────────

 Local or Remote AI
```

---

# Request Lifecycle

Every request follows the same deterministic execution path.

```text
User

↓

Interface

↓

Platform Request

↓

Validation Engine

↓

Capability Engine

↓

Reasoning Pipeline

↓

Provider Formatter

↓

AI Provider

↓

Platform Response

↓

Interface

↓

User
```

Interfaces never bypass the platform.

All interfaces use the same request pipeline.

---

# Core Platform

The Core Platform orchestrates execution.

Major responsibilities include:

- Request validation
- Capability routing
- Service orchestration
- Provider orchestration
- Response validation
- Trace generation

The Core Platform never performs AI reasoning directly.

---

# Knowledge Engine

The Knowledge Engine transforms repositories into structured
engineering knowledge.

Pipeline:

```text
Repository

↓

Repository Discovery

↓

Markdown Parser

↓

Metadata Extraction

↓

Knowledge Object Builder

↓

Relationship Builder

↓

Knowledge Graph

↓

Knowledge Index
```

This process is deterministic.

The same repository always produces the same knowledge model.

---

# Reasoning Pipeline

The Reasoning Pipeline retrieves engineering evidence.

```text
Knowledge Index

↓

Search

↓

Ranking

↓

Citation

↓

Context Assembly
```

The pipeline is responsible for:

- Retrieval
- Ordering
- Evidence selection
- Context construction

It never generates natural language.

---

# Context Package

The Context Package is the boundary between deterministic
reasoning and AI generation.

It contains:

- Selected Knowledge Objects
- Citations
- Supporting evidence
- Constraints
- System instructions

Every provider receives the same Context Package.

---

# Provider Layer

The Provider Layer converts Context Packages into AI requests.

```text
Context Package

↓

Provider Formatter

↓

Provider Request

↓

Provider Manager

↓

AI Provider
```

Provider implementations are intentionally lightweight.

Their responsibilities are limited to:

- Request serialization
- HTTP communication
- Response parsing
- Diagnostics

Providers never perform retrieval or reasoning.

---

# AI Provider Independence

Every provider consumes the same ProviderRequest.

Examples include:

- Ollama
- OpenAI
- Anthropic
- LM Studio

Adding a new provider should not require changes to:

- Search Engine
- Ranking Engine
- Citation Engine
- Context Assembler
- Knowledge Engine

Only a new provider adapter should be required.

---

# Layer Responsibilities

## Interfaces

Responsible for:

- User interaction
- Request creation
- Response presentation

Interfaces never perform reasoning.

---

## Core Platform

Responsible for:

- Validation
- Capability dispatch
- Service execution
- Provider orchestration

---

## Knowledge Engine

Responsible for:

- Repository indexing
- Structured knowledge generation
- Relationship discovery

---

## Reasoning Pipeline

Responsible for:

- Evidence retrieval
- Ranking
- Citation generation
- Context construction

---

## Provider Layer

Responsible for:

- Prompt formatting
- Provider communication
- AI response parsing

---

## AI Models

Responsible only for:

- Natural language generation

AI models do not retrieve engineering knowledge.

---

# Deterministic vs Non-Deterministic Components

## Deterministic

- Validation Engine
- Capability Engine
- Repository Discovery
- Markdown Parser
- Metadata Extraction
- Knowledge Object Builder
- Relationship Builder
- Knowledge Graph
- Knowledge Index
- Search Engine
- Ranking Engine
- Citation Engine
- Context Assembler
- Provider Formatter

Given identical inputs, these components always produce the
same outputs.

---

## Non-Deterministic

- Language Models

Examples:

- Ollama
- OpenAI
- Anthropic
- LM Studio

Only language generation is non-deterministic.

Everything before provider invocation is deterministic.

---

# Architectural Boundaries

The platform intentionally separates concerns.

```text
Knowledge

↓

Reasoning

↓

Context

↓

Language Generation
```

No provider performs reasoning.

No reasoning component performs language generation.

---

# Why This Architecture?

Traditional AI systems often allow language models to:

- Search
- Retrieve
- Rank
- Reason
- Generate

E.V.E. separates these responsibilities.

Benefits include:

- Explainability
- Traceability
- Repeatability
- Easier testing
- Provider independence
- Long-term maintainability

---

# Current Layer Status

| Layer | Status |
|--------|--------|
| Core Platform | ✅ Complete |
| Validation | ✅ Complete |
| Capability Engine | ✅ Complete |
| Knowledge Engine | ✅ Complete |
| Knowledge Graph | ✅ Complete |
| Knowledge Index | ✅ Complete |
| Search Engine | ✅ Complete |
| Ranking Engine | ✅ Complete |
| Citation Engine | ✅ Complete |
| Context Assembler | ✅ Complete |
| Provider Formatter | ✅ Complete |
| Ollama Provider | ✅ Complete |
| End-to-End Pipeline | ✅ Complete |

---

# Future Layers

The architecture is designed for expansion.

Planned additions include:

- CLI
- Discord Bot
- REST API
- Website
- OpenAI Provider
- Anthropic Provider
- LM Studio Provider
- Streaming Responses
- Conversation Memory
- Distributed Capability Network

None of these should require redesigning the Core Platform.

---

# Repository Structure

```text
eve/

├── docs/
│   ├── adr/
│   ├── 0000-core-platform-specifications/
│   ├── 1000-implementation-guides/
│   └── ...
│
├── include/
│   ├── ai/
│   ├── capability/
│   ├── context/
│   ├── core/
│   ├── dispatcher/
│   ├── knowledge/
│   ├── service/
│   └── validation/
│
├── src/
│
├── tests/
│
├── config/
│
├── ideas/
│
├── ARCHITECTURE.md
├── PROJECT_STATUS.md
├── README.md
└── CMakeLists.txt
```

---

# Architecture Maturity

The Core Platform is now feature-complete.

Future development focuses on:

- New interfaces
- Additional AI providers
- User experience
- Performance optimization
- Distributed execution

The architectural foundation is intended to remain stable.

---

# Summary

E.V.E. separates deterministic engineering reasoning from
probabilistic language generation.

Knowledge is discovered, structured, ranked, cited, and
assembled before any AI model is invoked.

This architecture provides:

- Explainable engineering answers
- Provider independence
- Deterministic reasoning
- Traceable execution
- Long-term maintainability

The result is an engineering intelligence platform where AI
serves as the language layer—not the reasoning engine.

---

**Project:** Evolutionary Virtual Engineer (E.V.E.)

**Repository:** VectorMorphOrg/eve

**Architecture:** v0.6.0-alpha

**Status:** Operational Foundation Complete

*"Knowledge should evolve."*