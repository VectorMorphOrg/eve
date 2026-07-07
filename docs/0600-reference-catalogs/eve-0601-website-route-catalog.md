# EVE-0601

# Website Route Catalog

**Project:** Evolutionary Virtual Engineer (E.V.E.)

**Specification:** EVE-SPEC

**Part:** VII — Reference Catalogs

**Document Version:** 1.0.0

**Published:** July 2026

**Authors:** VectorMorph Research Initiative (VMRI)

**Status:** Draft

---

# Purpose

This document provides the authoritative catalog of website
routes supported by the official E.V.E. web interface.

Unlike engineering specifications, this catalog does not
define website behavior.

Instead, it serves as a centralized reference describing the
public routes currently available or planned for the website.

Behavioral requirements are defined within Part III —
Website Specifications.

---

# Scope

This catalog includes:

- Public Pages
- Documentation Routes
- Repository Routes
- Project Information
- Search
- Authentication
- Administration
- Developer Pages

Future routes may be added as the platform evolves.

---

# Route Status

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
| Public | Available to all visitors |
| Authenticated | Logged-in users |
| Developer | Project contributors |
| Administrator | Platform administrators |

---

# Public Routes

| ID | Route | Description | Access | Status |
|----|-------|-------------|--------|--------|
| WEB-0001 | / | Home page | Public | 🚧 |
| WEB-0002 | /about | About VectorMorph | Public | 🚧 |
| WEB-0003 | /download | Downloads | Public | 🚧 |
| WEB-0004 | /news | Project news | Public | 🚧 |
| WEB-0005 | /roadmap | Public roadmap | Public | 🚧 |

---

# Documentation Routes

| ID | Route | Description | Access | Status |
|----|-------|-------------|--------|--------|
| WEB-0100 | /docs | Documentation index | Public | 🚧 |
| WEB-0101 | /docs/spec | Specification browser | Public | 🚧 |
| WEB-0102 | /docs/search | Documentation search | Public | 🚧 |
| WEB-0103 | /docs/part | Browse specification parts | Public | 🚧 |
| WEB-0104 | /docs/document | View specification document | Public | 🚧 |

---

# Repository Routes

| ID | Route | Description | Access | Status |
|----|-------|-------------|--------|--------|
| WEB-0200 | /repositories | Repository index | Public | 🚧 |
| WEB-0201 | /repositories/vectormorph | VectorMorph repository | Public | 🚧 |
| WEB-0202 | /repositories/eve | E.V.E. repository | Public | 🚧 |
| WEB-0203 | /repositories/website | Website repository | Public | 🚧 |

---

# Search Routes

| ID | Route | Description | Access | Status |
|----|-------|-------------|--------|--------|
| WEB-0300 | /search | Global search | Public | 🚧 |
| WEB-0301 | /search/docs | Documentation search | Public | 🚧 |
| WEB-0302 | /search/repositories | Repository search | Public | 🚧 |

---

# User Routes

| ID | Route | Description | Access | Status |
|----|-------|-------------|--------|--------|
| WEB-0400 | /login | User login | Public | 🚧 |
| WEB-0401 | /logout | User logout | Authenticated | 🚧 |
| WEB-0402 | /profile | User profile | Authenticated | 🚧 |
| WEB-0403 | /settings | User settings | Authenticated | 🚧 |

---

# Developer Routes

| ID | Route | Description | Access | Status |
|----|-------|-------------|--------|--------|
| WEB-0500 | /developer | Developer portal | Developer | 🚧 |
| WEB-0501 | /developer/api | API information | Developer | 🚧 |
| WEB-0502 | /developer/specifications | Specification resources | Developer | 🚧 |
| WEB-0503 | /developer/contributing | Contribution resources | Developer | 🚧 |

---

# Administration Routes

| ID | Route | Description | Access | Status |
|----|-------|-------------|--------|--------|
| WEB-0600 | /admin | Administration dashboard | Administrator | 🚧 |
| WEB-0601 | /admin/users | User management | Administrator | 🚧 |
| WEB-0602 | /admin/system | System management | Administrator | 🚧 |
| WEB-0603 | /admin/logs | Platform logs | Administrator | 🚧 |

---

# Reserved Routes

The following identifier ranges are reserved for future
expansion.

| Range | Purpose |
|--------|---------|
| WEB-0700–0799 | Community Features |
| WEB-0800–0899 | Future Web Applications |
| WEB-0900–0999 | Experimental Routes |

Reserved identifiers maintain a stable numbering scheme as
the website evolves.

---

# Route Naming Guidelines

Website routes should:

- Be lowercase.
- Be descriptive.
- Use hyphens where appropriate.
- Remain stable whenever possible.
- Represent a single logical destination.

Routes should prioritize readability and long-term
maintainability.

---

# Relationship to EVE-SPEC

This catalog complements Part III — Website Specifications.

Behavior is defined within the engineering specifications.

This document serves only as the authoritative inventory of
website routes.

---

# Future Expansion

Future revisions of this catalog may include:

- Route parameters
- Authentication requirements
- HTTP methods
- Breadcrumb navigation
- SEO metadata
- Deprecation schedules

---

# Summary

The Website Route Catalog provides a centralized inventory of
the routes supported by the official E.V.E. website.

It exists to improve discoverability, maintain consistency,
and provide a stable reference for developers, contributors,
and users.

---

## Navigation

**Previous**

EVE-0600 — Discord Command Catalog

**Part**

VII — Reference Catalogs

**Next**

EVE-0602 — CLI Command Catalog

**Return**

← Part VII — Reference Catalogs

← EVE-SPEC

---

## Motto

*"Knowledge should evolve."*