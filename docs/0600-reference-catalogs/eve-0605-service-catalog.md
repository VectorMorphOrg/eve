# EVE-0605

# Service Catalog

**Project:** Evolutionary Virtual Engineer (E.V.E.)

**Specification:** EVE-SPEC

**Part:** VII — Reference Catalogs

**Document Version:** 1.0.0

**Published:** July 2026

**Authors:** VectorMorph Research Initiative (VMRI)

**Status:** Draft

---

# Purpose

This document provides the authoritative catalog of platform
services supported by the Evolutionary Virtual Engineer
(E.V.E.).

Unlike engineering specifications, this catalog does not
define service behavior.

Instead, it serves as a centralized inventory of the
long-lived services that collectively implement the E.V.E.
platform.

Behavioral requirements are defined within the corresponding
engineering specifications.

---

# Scope

This catalog includes:

- Core Platform Services
- Knowledge Services
- AI Services
- Repository Services
- Development Services
- Platform Services
- Administrative Services

Additional services may be introduced as the platform
evolves.

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
| Core | Platform orchestration services |
| Knowledge | Documentation and knowledge services |
| AI | Artificial intelligence services |
| Repository | Repository integration services |
| Development | Development support services |
| Platform | Shared infrastructure services |
| Administration | Administrative platform services |

---

# Core Platform Services

| ID | Service | Description | Status |
|----|----------|-------------|--------|
| SVC-0001 | Command Dispatcher | Routes platform requests | 🚧 |
| SVC-0002 | Capability Engine | Executes platform capabilities | 🚧 |
| SVC-0003 | Request Processor | Processes Platform Requests | 🚧 |
| SVC-0004 | Response Processor | Produces Response Objects | 🚧 |
| SVC-0005 | Permission Service | Authorization and access control | 🚧 |

---

# Knowledge Services

| ID | Service | Description | Status |
|----|----------|-------------|--------|
| SVC-0100 | Knowledge Service | Access documented knowledge | 🚧 |
| SVC-0101 | Documentation Service | Retrieve documentation | 🚧 |
| SVC-0102 | Search Service | Search platform knowledge | 🚧 |
| SVC-0103 | Citation Service | Generate documentation references | 🚧 |
| SVC-0104 | Index Service | Maintain searchable indexes | 🚧 |

---

# AI Services

| ID | Service | Description | Status |
|----|----------|-------------|--------|
| SVC-0200 | AI Engine | Generate AI responses | 🚧 |
| SVC-0201 | Context Builder | Assemble AI context | 🚧 |
| SVC-0202 | Provider Manager | Select AI providers | 🚧 |
| SVC-0203 | Prompt Builder | Generate provider prompts | 🚧 |
| SVC-0204 | Conversation Service | Manage conversational context | 🚧 |

---

# Repository Services

| ID | Service | Description | Status |
|----|----------|-------------|--------|
| SVC-0300 | Repository Service | Repository abstraction layer | 🚧 |
| SVC-0301 | GitHub Service | GitHub integration | 🚧 |
| SVC-0302 | Issue Service | Issue tracking integration | 🚧 |
| SVC-0303 | Release Service | Release information | 🚧 |
| SVC-0304 | Repository Index Service | Repository discovery | 🚧 |

---

# Development Services

| ID | Service | Description | Status |
|----|----------|-------------|--------|
| SVC-0400 | Roadmap Service | Development roadmap | 🚧 |
| SVC-0401 | ADR Service | Architecture Decision Records | 🚧 |
| SVC-0402 | Build Service | Build metadata | 🚧 |
| SVC-0403 | Testing Service | Testing information | 🚧 |
| SVC-0404 | Project Service | Project metadata | 🚧 |

---

# Platform Services

| ID | Service | Description | Status |
|----|----------|-------------|--------|
| SVC-0500 | Configuration Service | Platform configuration | 🚧 |
| SVC-0501 | Logging Service | Centralized logging | 🚧 |
| SVC-0502 | Metrics Service | Runtime metrics | 🚧 |
| SVC-0503 | Cache Service | Shared caching | 🚧 |
| SVC-0504 | Scheduling Service | Scheduled operations | 🚧 |

---

# Administrative Services

| ID | Service | Description | Status |
|----|----------|-------------|--------|
| SVC-0600 | Authentication Service | Identity management | 🚧 |
| SVC-0601 | Authorization Service | Permission enforcement | 🚧 |
| SVC-0602 | Health Service | Platform health monitoring | 🚧 |
| SVC-0603 | Update Service | Platform updates | 🚧 |
| SVC-0604 | Audit Service | Audit logging | 🚧 |

---

# Reserved Service Identifiers

| Range | Purpose |
|--------|---------|
| SVC-0700–0799 | Future Platform Services |
| SVC-0800–0899 | Future AI Services |
| SVC-0900–0999 | Experimental Services |

Reserved identifiers maintain a stable service numbering
scheme as E.V.E. evolves.

---

# Service Naming Guidelines

Services should:

- Represent a long-lived platform component.
- Expose one primary responsibility.
- Support one or more capabilities.
- Be reusable across interfaces.
- Remain implementation independent whenever possible.

Services describe **who performs the work**.

Capabilities describe **what work is performed**.

---

# Relationship to EVE-SPEC

This catalog complements the Platform Specifications and
Core Platform Specifications.

Individual specifications define service behavior.

This document serves only as the authoritative inventory of
platform services.

---

# Future Expansion

Future revisions of this catalog may include:

- Service dependencies
- Lifecycle management
- Startup order
- Configuration requirements
- Health status
- Interface contracts
- Capability mappings

---

# Summary

The Service Catalog provides a centralized inventory of the
services that comprise the Evolutionary Virtual Engineer
platform.

It exists to improve architectural consistency, establish a
shared engineering vocabulary, and provide a stable
reference for developers, contributors, and future
implementations.

---

## Navigation

**Previous**

EVE-0604 — Capability Catalog

**Part**

VII — Reference Catalogs

**Next**

EVE-0606 — Knowledge Object Catalog

**Return**

← Part VII — Reference Catalogs

← EVE-SPEC

---

## Motto

*"Knowledge should evolve."*