# E.V.E.

> **Evolutionary Virtual Engineer**

Deterministic engineering reasoning powered by interchangeable AI providers.

E.V.E. is the engineering intelligence platform developed by the
VectorMorph Research Initiative.

Unlike traditional AI assistants, E.V.E. retrieves, validates,
relates, ranks, cites, and assembles engineering knowledge before any
language model is invoked.

The AI generates language.

The platform generates knowledge.

---

# Current Status

| Property | Value |
|----------|-------|
| **Version** | v0.7.0-alpha |
| **Status** | Operational Alpha |
| **Language** | C++23 |
| **Specification** | EVE-SPEC |
| **Tests** | 278 non-live / 275 PASS / 3 known failures |
| **License** | MIT |

---

# What Makes E.V.E. Different?

Traditional AI systems ask an LLM to both retrieve information and
generate an answer.

E.V.E. separates those responsibilities.

```text
Platform Request

↓

Validation

↓

Knowledge Engine

↓

Search

↓

Ranking

↓

Citation

↓

Context Assembly

↓

AI Provider

↓

Platform Response
```

The deterministic reasoning pipeline produces an evidence-backed
ContextPackage.

Only after reasoning is complete is an AI provider invoked to generate
natural language.

This architecture makes E.V.E.:

- Deterministic
- Explainable
- Provider Independent
- Knowledge Grounded
- Evidence Backed
- Testable

---

# Current Features

## Core Platform

- Platform Request / Response contracts
- Validation Engine
- Capability Engine
- Service Layer
- Documentation-first architecture

## Knowledge Engine

- Repository Discovery
- Markdown Parser
- Metadata Extraction
- Knowledge Object Builder
- Relationship Builder
- Knowledge Graph
- Knowledge Index

## Reasoning Pipeline

- Search Engine
- Ranking Engine
- Citation Engine
- Context Assembler
- End-to-end deterministic pipeline

## AI Providers

- Null Provider
- Ollama Provider
- Provider Formatting Layer
- Live Ollama Integration

---

# Current Architecture

```text
User

↓

Platform Request

↓

Validation Engine

↓

Capability Engine

↓

Search Engine

↓

Ranking Engine

↓

Citation Engine

↓

Context Assembler

↓

Provider Formatter

↓

AI Provider

↓

Platform Response
```

---

# Repository Structure

```text
eve/

├── docs/
│   ├── 0000-core-platform-specifications/
│   ├── 0300-cli-specifications/
│   ├── 0600-reference-catalogs/
│   ├── 1000-implementation-guides/
│   ├── 3000-architecture-decision-records/
│   └── ...
│
├── audits/
│   ├── document-map.md
│   └── v0.6.0-alpha/
│
├── ideas/
├── include/
├── src/
├── tests/
├── config/
│
├── README.md
├── ARCHITECTURE.md
├── PROJECT_STATUS.md
├── ROADMAP.md
├── CHANGELOG.md
└── CMakeLists.txt
```

---

# Documentation

| Document | Purpose |
|----------|---------|
| **README.md** | Project overview |
| **ARCHITECTURE.md** | Complete system architecture |
| **PROJECT_STATUS.md** | Current progress |
| **ROADMAP.md** | Canonical release sequencing |
| **docs/** | Formal EVE-SPEC specifications |
| **docs/3000-architecture-decision-records/** | Architecture Decision Records |
| **audits/document-map.md** | Repository file / path map |

---

# Roadmap

Canonical sequencing (Hybrid / Option C):

### Completed — v0.6.0-alpha

- Core Platform
- Validation Engine
- Knowledge Engine
- Reasoning Pipeline
- Ollama Provider
- Provider Formatting
- Live End-to-End Demonstration

### Completed — v0.7.0-alpha

- Conversation Memory foundation
- Provider-layer streaming (Null, Ollama, HTTP chunked)
- LM Studio provider (AI-0102; sync + SSE)
- EVE-2004 Provider Development Guide

### Continuing — v0.7.x

- OpenAI / Anthropic providers (future)
- AI-0200 generic OpenAI-compatible provider (deferred)
- Remaining Part XII developer guides

### Then — v0.8.x Interfaces

- CLI (specifications already exist)
- Discord
- REST API
- Website

### Later — v0.9.x+

- Multi-Repository Support
- Semantic / Hybrid Search
- Plugin System
- Distributed Capability Network

See `ROADMAP.md` for the authoritative sequence.

# Engineering Philosophy

E.V.E. follows a documentation-first engineering process.

Core principles include:

- Documentation First
- Specification Before Implementation
- Knowledge First
- Deterministic Behavior
- Explainable Results
- Provider Independence
- Capability-Oriented Architecture
- Separation of Responsibilities
- Test-Driven Development

---

# Building

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build
```

---

# Vision

E.V.E. is being developed as a provider-independent engineering
intelligence platform capable of powering multiple interfaces,
including:

- Command Line
- Discord
- REST API
- Website
- Future integrations

The long-term goal is to provide deterministic, evidence-backed
engineering assistance regardless of which language model is used.

---

**Project:** Evolutionary Virtual Engineer (E.V.E.)

**Repository:** VectorMorphOrg/eve

**Version:** v0.7.0-alpha

*"Knowledge should evolve."*