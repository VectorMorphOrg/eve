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
| **Version** | v0.6.0-alpha |
| **Status** | Operational Alpha |
| **Language** | C++23 |
| **Specification** | EVE-SPEC |
| **Tests** | 172 / 172 Passing |
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
│   ├── adr/
│   ├── 0000-core-platform-specifications/
│   ├── 1000-implementation-guides/
│   └── ...
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
├── CHANGELOG.md
└── CMakeLists.txt
```

---

# Documentation

| Document | Purpose |
|----------|---------|
| **README.md** | Project overview |
| **ARCHITECTURE.md** | Complete system architecture |
| **PROJECT_STATUS.md** | Current progress and roadmap |
| **docs/** | Formal EVE-SPEC specifications |
| **docs/adr/** | Architecture Decision Records |

---

# Roadmap

### Completed

- Core Platform
- Validation Engine
- Knowledge Engine
- Reasoning Pipeline
- Ollama Provider
- Provider Formatting
- Live End-to-End Demonstration

### In Progress

- CLI Interface
- REST API
- Discord Integration
- Website

### Planned

- OpenAI Provider
- Anthropic Provider
- LM Studio Provider
- Conversation Memory
- Multi-Repository Support
- Distributed Capability Network

---

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

**Version:** v0.6.0-alpha

*"Knowledge should evolve."*