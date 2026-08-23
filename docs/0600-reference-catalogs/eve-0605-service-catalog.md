# EVE-0605

# Service Catalog

> Authoritative Registry of E.V.E. Platform Services

---

## Document Information

| Property | Value |
|----------|-------|
| **Document ID** | EVE-0605 |
| **Series** | 0600 — Reference Catalogs |
| **Title** | Service Catalog |
| **Project** | Evolutionary Virtual Engineer (E.V.E.) |
| **Version** | 2.0.0 |
| **Status** | Active |
| **Published** | July 2026 |

---

# Purpose

This document is the authoritative registry of long-lived
platform services used by the Evolutionary Virtual Engineer
(E.V.E.).

Services are reusable platform components that expose
well-defined functionality to Capabilities and other
subsystems.

Unlike Engineering Specifications, this catalog does not
define behavior.

It exists solely to document the platform's reusable service
inventory.

---

# Service Status

| Status | Meaning |
|---------|----------|
| ✅ | Implemented |
| 🚧 | Planned |
| 🧪 | Experimental |
| ⚠ | Deprecated |
| ❌ | Removed |

---

# Service Categories

| Category | Description |
|----------|-------------|
| Core | Core platform orchestration |
| Knowledge | Repository and documentation services |
| AI | AI Provider infrastructure |
| Repository | Repository management |
| Development | Engineering support |
| Platform | Shared infrastructure |
| Administration | Administrative services |

---

# Core Services

| ID | Service | Primary Responsibility | Status |
|----|----------|-----------------------|--------|
| SVC-0001 | Command Dispatcher | Dispatch Platform Requests | ✅ |
| SVC-0002 | Capability Engine | Execute Capabilities | ✅ |
| SVC-0003 | Validation Engine | Validate Requests and Responses | ✅ |
| SVC-0004 | Core Platform | Coordinate platform execution | ✅ |
| SVC-0005 | Platform Bootstrap | Construct platform dependencies | ✅ |

---

# Knowledge Services

| ID | Service | Primary Responsibility | Status |
|----|----------|-----------------------|--------|
| SVC-0100 | Knowledge Engine | Repository knowledge management | ✅ |
| SVC-0101 | Search Engine | Retrieve candidate Knowledge Objects | ✅ |
| SVC-0102 | Ranking Engine | Rank search results | ✅ |
| SVC-0103 | Citation Engine | Produce engineering citations | ✅ |
| SVC-0104 | Context Assembler | Build Context Packages | ✅ |

---

# AI Services

| ID | Service | Primary Responsibility | Status |
|----|----------|-----------------------|--------|
| SVC-0200 | Provider Manager | Manage AI Providers | ✅ |
| SVC-0201 | Provider Formatter | Build Provider Requests | ✅ |
| SVC-0202 | Null Provider | Stub AI Provider | ✅ |
| SVC-0203 | Ollama Provider | Local AI Provider | ✅ |
| SVC-0204 | HTTP Transport | Provider networking | ✅ |

---

# Repository Services

| ID | Service | Primary Responsibility | Status |
|----|----------|-----------------------|--------|
| SVC-0300 | Repository Discovery | Discover repositories | ✅ |
| SVC-0301 | Markdown Parser | Parse Markdown documents | ✅ |
| SVC-0302 | Metadata Extractor | Extract document metadata | ✅ |
| SVC-0303 | Knowledge Object Builder | Construct Knowledge Objects | ✅ |
| SVC-0304 | Relationship Builder | Build Knowledge Graph | ✅ |

---

# Development Services

| ID | Service | Primary Responsibility | Status |
|----|----------|-----------------------|--------|
| SVC-0400 | Documentation Service | Documentation access | 🚧 |
| SVC-0401 | Build Information Service | Build metadata | 🚧 |
| SVC-0402 | Test Information Service | Test metadata | 🚧 |
| SVC-0403 | Roadmap Service | Project roadmap | 🚧 |
| SVC-0404 | ADR Service | Architecture decision lookup | 🚧 |

---

# Platform Services

| ID | Service | Primary Responsibility | Status |
|----|----------|-----------------------|--------|
| SVC-0500 | Configuration Service | Runtime configuration | ✅ |
| SVC-0501 | Logging Service | Structured logging | 🚧 |
| SVC-0502 | Metrics Service | Runtime metrics | 🚧 |
| SVC-0503 | Cache Service | Shared caching | 🚧 |
| SVC-0504 | Scheduling Service | Scheduled execution | 🚧 |

---

# Administrative Services

| ID | Service | Primary Responsibility | Status |
|----|----------|-----------------------|--------|
| SVC-0600 | Authentication Service | Identity management | 🚧 |
| SVC-0601 | Authorization Service | Permission enforcement | 🚧 |
| SVC-0602 | Health Service | Platform health monitoring | ✅ |
| SVC-0603 | Update Service | Platform updates | 🚧 |
| SVC-0604 | Audit Service | Audit logging | 🚧 |

---

# Reserved Service Ranges

| Range | Purpose |
|--------|---------|
| SVC-0700–0799 | Future Platform Services |
| SVC-0800–0899 | Future AI Services |
| SVC-0900–0999 | Experimental Services |

Reserved ranges preserve long-term identifier stability.

---

# Service Definition

A Service is a long-lived reusable platform component.

Services:

- encapsulate infrastructure
- expose reusable functionality
- may support multiple Capabilities
- are independent of user interfaces
- are constructed during platform bootstrap

Capabilities invoke Services.

Interfaces invoke Capabilities.

---

# Relationship to EVE-SPEC

Behavior is defined by:

- Part I — Engineering Specifications

Implementation guidance is provided by:

- Part XI — Implementation Guides

This catalog serves only as the authoritative registry of
platform services.

---

# Current Implementation Status

As of **v0.6.0-alpha**, the implemented service layer
includes:

- Core Platform
- Validation Engine
- Capability Engine
- Knowledge Engine
- Search Engine
- Ranking Engine
- Citation Engine
- Context Assembler
- Provider Formatter
- Provider Manager
- Ollama Provider
- HTTP Transport
- Configuration Service

Additional services will be added while preserving existing
service identifiers.

---

## Navigation

**Previous**

EVE-0604 — Capability Catalog

**Current**

EVE-0605 — Service Catalog

**Next**

EVE-0606 — Knowledge Object Catalog

**Return**

← Part VII — Reference Catalogs

← EVE-SPEC

---

*"Knowledge should evolve."*