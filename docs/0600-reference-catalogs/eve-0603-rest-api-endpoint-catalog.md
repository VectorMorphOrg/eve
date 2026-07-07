# EVE-0603

# REST API Endpoint Catalog

**Project:** Evolutionary Virtual Engineer (E.V.E.)

**Specification:** EVE-SPEC

**Part:** VII — Reference Catalogs

**Document Version:** 1.0.0

**Published:** July 2026

**Authors:** VectorMorph Research Initiative (VMRI)

**Status:** Draft

---

# Purpose

This document provides the authoritative catalog of REST API
endpoints supported by the Evolutionary Virtual Engineer
(E.V.E.) platform.

Unlike engineering specifications, this catalog does not
define endpoint behavior.

Instead, it serves as a centralized reference describing the
API endpoints currently available or planned for the E.V.E.
platform.

Behavioral requirements are defined within Part V —
REST API Specifications.

---

# Scope

This catalog includes:

- System Endpoints
- Documentation Endpoints
- Repository Endpoints
- Knowledge Endpoints
- Development Endpoints
- Diagnostics Endpoints
- Administration Endpoints

Future endpoints may be added as the platform evolves.

---

# Endpoint Status

| Status | Meaning |
|---------|----------|
| ✅ | Implemented |
| 🚧 | Planned |
| 🧪 | Experimental |
| ⚠ | Deprecated |
| ❌ | Removed |

---

# Access Levels

| Access | Description |
|---------|-------------|
| Public | No authentication required |
| Authenticated | Valid user authentication required |
| Developer | Project contributors |
| Administrator | Platform administrators |

---

# System Endpoints

| ID | Endpoint | Description | Access | Status |
|----|----------|-------------|--------|--------|
| API-0001 | GET /api/v1/status | Platform status | Public | 🚧 |
| API-0002 | GET /api/v1/version | Platform version | Public | 🚧 |
| API-0003 | GET /api/v1/about | Platform information | Public | 🚧 |
| API-0004 | GET /api/v1/health | Health check | Public | 🚧 |

---

# Documentation Endpoints

| ID | Endpoint | Description | Access | Status |
|----|----------|-------------|--------|--------|
| API-0100 | GET /api/v1/docs | Documentation index | Public | 🚧 |
| API-0101 | GET /api/v1/specifications | Specification catalog | Public | 🚧 |
| API-0102 | GET /api/v1/specifications/{id} | Specification document | Public | 🚧 |
| API-0103 | GET /api/v1/search | Documentation search | Public | 🚧 |

---

# Knowledge Endpoints

| ID | Endpoint | Description | Access | Status |
|----|----------|-------------|--------|--------|
| API-0200 | GET /api/v1/knowledge | Knowledge index | Public | 🚧 |
| API-0201 | GET /api/v1/capabilities | Capability catalog | Public | 🚧 |
| API-0202 | GET /api/v1/services | Service catalog | Public | 🚧 |
| API-0203 | GET /api/v1/providers | AI provider catalog | Public | 🚧 |
| API-0204 | GET /api/v1/context | Context package information | Developer | 🚧 |

---

# Repository Endpoints

| ID | Endpoint | Description | Access | Status |
|----|----------|-------------|--------|--------|
| API-0300 | GET /api/v1/repositories | Repository index | Public | 🚧 |
| API-0301 | GET /api/v1/repositories/{name} | Repository information | Public | 🚧 |
| API-0302 | GET /api/v1/issues | Repository issues | Public | 🚧 |
| API-0303 | GET /api/v1/releases | Repository releases | Public | 🚧 |

---

# Development Endpoints

| ID | Endpoint | Description | Access | Status |
|----|----------|-------------|--------|--------|
| API-0400 | GET /api/v1/roadmap | Development roadmap | Public | 🚧 |
| API-0401 | GET /api/v1/tasks | Development tasks | Developer | 🚧 |
| API-0402 | GET /api/v1/adr | Architecture Decision Records | Public | 🚧 |
| API-0403 | GET /api/v1/build | Build information | Developer | 🚧 |
| API-0404 | POST /api/v1/test | Execute platform tests | Developer | 🚧 |

---

# Diagnostics Endpoints

| ID | Endpoint | Description | Access | Status |
|----|----------|-------------|--------|--------|
| API-0500 | GET /api/v1/metrics | Runtime metrics | Developer | 🚧 |
| API-0501 | GET /api/v1/logs | Platform logs | Maintainer | 🚧 |
| API-0502 | GET /api/v1/config | Platform configuration | Developer | 🚧 |
| API-0503 | POST /api/v1/validate | Validate installation | Developer | 🚧 |

---

# Administration Endpoints

| ID | Endpoint | Description | Access | Status |
|----|----------|-------------|--------|--------|
| API-0600 | POST /api/v1/sync | Synchronize resources | Administrator | 🚧 |
| API-0601 | POST /api/v1/update | Update platform | Administrator | 🚧 |
| API-0602 | POST /api/v1/restart | Restart services | Administrator | 🚧 |
| API-0603 | POST /api/v1/shutdown | Shut down platform | Administrator | 🚧 |
| API-0604 | GET /api/v1/users | User management | Administrator | 🚧 |

---

# Reserved Endpoints

The following identifier ranges are reserved for future
expansion.

| Range | Purpose |
|--------|---------|
| API-0700–0799 | AI Conversation Services |
| API-0800–0899 | Plugin Services |
| API-0900–0999 | Experimental Endpoints |

Reserved identifiers maintain a stable numbering scheme as
the REST API evolves.

---

# Endpoint Naming Guidelines

REST API endpoints should:

- Follow RESTful design principles.
- Use lowercase resource names.
- Be versioned.
- Represent resources rather than actions.
- Remain stable whenever possible.

Endpoint behavior should remain consistent across API
versions.

---

# Relationship to EVE-SPEC

This catalog complements Part V — REST API Specifications.

Behavior is defined within the engineering specifications.

This document serves only as the authoritative inventory of
REST API endpoints.

---

# Future Expansion

Future revisions of this catalog may include:

- Request schemas
- Response schemas
- Authentication methods
- Rate limits
- Pagination
- Error codes
- OpenAPI references

---

# Summary

The REST API Endpoint Catalog provides a centralized
inventory of the endpoints supported by the Evolutionary
Virtual Engineer platform.

It exists to improve discoverability, maintain consistency,
and provide a stable reference for developers, contributors,
and third-party integrations.

---

## Navigation

**Previous**

EVE-0602 — CLI Command Catalog

**Part**

VII — Reference Catalogs

**Next**

Reserved

**Return**

← Part VII — Reference Catalogs

← EVE-SPEC

---

## Motto

*"Knowledge should evolve."*