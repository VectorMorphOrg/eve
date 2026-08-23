# EVE-0604

# Capability Catalog

> Authoritative Registry of E.V.E. Platform Capabilities

---

## Document Information

| Property | Value |
|----------|-------|
| **Document ID** | EVE-0604 |
| **Series** | 0600 — Reference Catalogs |
| **Title** | Capability Catalog |
| **Project** | Evolutionary Virtual Engineer (E.V.E.) |
| **Version** | 2.0.0 |
| **Status** | Active |
| **Published** | July 2026 |

---

# Purpose

This document is the authoritative registry of Platform
Capabilities implemented by the Evolutionary Virtual
Engineer (E.V.E.).

Capabilities represent the public engineering operations
that may be invoked through Platform Requests.

Unlike Engineering Specifications, this catalog does not
define behavior.

It exists solely to provide stable capability identifiers
and a centralized inventory of platform functionality.

---

# Capability Status

| Status | Meaning |
|---------|----------|
| ✅ | Implemented |
| 🚧 | Planned |
| 🧪 | Experimental |
| ⚠ | Deprecated |
| ❌ | Removed |

---

# Capability Categories

| Category | Description |
|----------|-------------|
| Core | Platform infrastructure |
| Documentation | Documentation retrieval |
| Knowledge | Knowledge processing |
| Repository | Repository management |
| AI | AI Provider operations |
| Development | Engineering workflows |
| Administration | Platform administration |

---

# Core Capabilities

| ID | Capability | Owner | Status |
|----|------------|-------|--------|
| CAP-0001 | Platform Request Processing | Core Platform | ✅ |
| CAP-0002 | Platform Response Generation | Core Platform | ✅ |
| CAP-0003 | Validation | Validation Engine | ✅ |
| CAP-0004 | Capability Dispatch | Command Dispatcher | ✅ |
| CAP-0005 | Capability Execution | Capability Engine | ✅ |

---

# Documentation Capabilities

| ID | Capability | Owner | Status |
|----|------------|-------|--------|
| CAP-0100 | Documentation Discovery | Knowledge Engine | 🚧 |
| CAP-0101 | Specification Lookup | Knowledge Engine | 🚧 |
| CAP-0102 | Documentation Search | Documentation Search Handler | ✅ |
| CAP-0103 | Documentation Navigation | Knowledge Engine | 🚧 |
| CAP-0104 | Cross-Reference Resolution | Knowledge Engine | 🚧 |

---

# Knowledge Capabilities

| ID | Capability | Owner | Status |
|----|------------|-------|--------|
| CAP-0200 | Knowledge Retrieval | Search Engine | ✅ |
| CAP-0201 | Search | Search Engine | ✅ |
| CAP-0202 | Ranking | Ranking Engine | ✅ |
| CAP-0203 | Citation Generation | Citation Engine | ✅ |
| CAP-0204 | Context Assembly | Context Assembler | ✅ |

---

# Repository Capabilities

| ID | Capability | Owner | Status |
|----|------------|-------|--------|
| CAP-0300 | Repository Discovery | Repository Service | ✅ |
| CAP-0301 | Repository Indexing | Knowledge Engine | ✅ |
| CAP-0302 | Source Discovery | Repository Service | 🚧 |
| CAP-0303 | Release Discovery | Repository Service | 🚧 |
| CAP-0304 | Repository Search | Knowledge Engine | 🚧 |

---

# AI Provider Capabilities

| ID | Capability | Owner | Status |
|----|------------|-------|--------|
| CAP-0400 | Provider Formatting | Provider Formatter | ✅ |
| CAP-0401 | Provider Selection | Provider Manager | ✅ |
| CAP-0402 | AI Response Generation | AI Provider | ✅ |
| CAP-0403 | Streaming Responses | AI Provider | 🚧 |
| CAP-0404 | Conversation Memory | Future Platform | 🚧 |

---

# Development Capabilities

| ID | Capability | Owner | Status |
|----|------------|-------|--------|
| CAP-0500 | Project Roadmap | Development Services | 🚧 |
| CAP-0501 | ADR Discovery | Knowledge Engine | 🚧 |
| CAP-0502 | Repository Search | Knowledge Engine | 🚧 |
| CAP-0503 | Build Information | Development Services | 🚧 |
| CAP-0504 | Test Information | Development Services | 🚧 |

---

# Administrative Capabilities

| ID | Capability | Owner | Status |
|----|------------|-------|--------|
| CAP-0600 | Configuration Management | Configuration Service | ✅ |
| CAP-0601 | Metrics | Platform Services | 🚧 |
| CAP-0602 | Logging | Logging Service | 🚧 |
| CAP-0603 | Health Monitoring | Provider Manager | ✅ |
| CAP-0604 | Platform Synchronization | Platform Services | 🚧 |

---

# Reserved Capability Ranges

| Range | Purpose |
|--------|---------|
| CAP-0700–0799 | Future AI Capabilities |
| CAP-0800–0899 | Future Knowledge Capabilities |
| CAP-0900–0999 | Experimental Capabilities |

Reserved ranges preserve long-term identifier stability.

---

# Capability Naming

Capabilities should:

- Represent one engineering responsibility.
- Remain interface independent.
- Remain implementation independent.
- Be uniquely identified.
- Be documented before implementation.
- Remain stable over time.

Capability identifiers are considered public platform
contracts.

---

# Relationship to EVE-SPEC

Behavior is defined by:

- Part I — Engineering Specifications

Implementation guidance is provided by:

- Part XI — Implementation Guides

This catalog serves only as the authoritative registry of
Capability identifiers.

---

# Current Implementation Status

As of **v0.6.0-alpha**:

- Platform Request architecture is implemented.
- Validation Engine is implemented.
- Capability Engine is implemented.
- Documentation Search (CAP-0102) is operational.
- Deterministic Reasoning Pipeline is operational.
- Provider Formatter is implemented.
- Ollama Provider is implemented.
- End-to-end documentation search is operational.

Additional capabilities will be added without changing
existing identifiers.

---

## Navigation

**Previous**

EVE-0603 — REST API Endpoint Catalog

**Current**

EVE-0604 — Capability Catalog

**Next**

EVE-0605 — Service Catalog

**Return**

← Part VII — Reference Catalogs

← EVE-SPEC

---

*"Knowledge should evolve."*