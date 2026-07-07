# EVE-0600

# Discord Command Catalog

**Project:** Evolutionary Virtual Engineer (E.V.E.)

**Specification:** EVE-SPEC

**Part:** VII — Reference Catalogs

**Document Version:** 1.0.0

**Published:** July 2026

**Authors:** VectorMorph Research Initiative (VMRI)

**Status:** Draft

---

# Purpose

This document provides the authoritative catalog of Discord
commands supported by E.V.E.

Unlike engineering specifications, this catalog does not
define command behavior.

Instead, it serves as a centralized reference describing the
commands currently available or planned for the Discord
interface.

Behavioral requirements are defined within the Discord
Specification (Part II).

---

# Scope

This catalog includes:

- User Commands
- Documentation Commands
- Knowledge Commands
- Repository Commands
- Development Commands
- Diagnostics Commands
- Administration Commands

Future commands may be added as the platform evolves.

---

# Command Status

| Status | Meaning |
|---------|----------|
| ✅ | Implemented |
| 🚧 | Planned |
| 🧪 | Experimental |
| ⚠ | Deprecated |
| ❌ | Removed |

---

# Command Permissions

| Permission | Description |
|------------|-------------|
| Everyone | Any Discord user |
| Developer | Project contributors |
| Maintainer | Repository maintainers |
| Administrator | Platform administrators |

---

# General Commands

| ID | Command | Description | Permission | Status |
|----|---------|-------------|------------|--------|
| CMD-0001 | /help | Display available commands | Everyone | 🚧 |
| CMD-0002 | /about | Information about E.V.E. | Everyone | 🚧 |
| CMD-0003 | /version | Display platform version | Everyone | 🚧 |
| CMD-0004 | /status | Display platform status | Everyone | 🚧 |
| CMD-0005 | /ping | Connectivity check | Everyone | 🚧 |

---

# Documentation Commands

| ID | Command | Description | Permission | Status |
|----|---------|-------------|------------|--------|
| CMD-0100 | /docs | Browse documentation | Everyone | 🚧 |
| CMD-0101 | /spec | Open a specification | Everyone | 🚧 |
| CMD-0102 | /search | Search documentation | Everyone | 🚧 |
| CMD-0103 | /read | Read a document | Everyone | 🚧 |
| CMD-0104 | /part | Browse specification parts | Everyone | 🚧 |
| CMD-0105 | /series | Browse document series | Everyone | 🚧 |
| CMD-0106 | /index | View specification index | Everyone | 🚧 |

---

# Knowledge Commands

| ID | Command | Description | Permission | Status |
|----|---------|-------------|------------|--------|
| CMD-0200 | /knowledge | Browse knowledge base | Everyone | 🚧 |
| CMD-0201 | /capability | Display platform capabilities | Everyone | 🚧 |
| CMD-0202 | /service | Display platform services | Everyone | 🚧 |
| CMD-0203 | /context | Display context information | Everyone | 🚧 |
| CMD-0204 | /provider | Display AI providers | Everyone | 🚧 |
| CMD-0205 | /persona | Display E.V.E. persona information | Everyone | 🚧 |

---

# Repository Commands

| ID | Command | Description | Permission | Status |
|----|---------|-------------|------------|--------|
| CMD-0300 | /repo | Repository information | Everyone | 🚧 |
| CMD-0301 | /repos | List repositories | Everyone | 🚧 |
| CMD-0302 | /branch | Branch information | Everyone | 🚧 |
| CMD-0303 | /issues | View repository issues | Everyone | 🚧 |
| CMD-0304 | /pulls | View pull requests | Everyone | 🚧 |
| CMD-0305 | /releases | View releases | Everyone | 🚧 |

---

# Development Commands

| ID | Command | Description | Permission | Status |
|----|---------|-------------|------------|--------|
| CMD-0400 | /roadmap | View development roadmap | Everyone | 🚧 |
| CMD-0401 | /task | View development tasks | Developer | 🚧 |
| CMD-0402 | /todo | View outstanding work | Developer | 🚧 |
| CMD-0403 | /adr | Browse Architecture Decision Records | Everyone | 🚧 |
| CMD-0404 | /build | Build information | Developer | 🚧 |
| CMD-0405 | /test | Testing information | Developer | 🚧 |

---

# Diagnostics Commands

| ID | Command | Description | Permission | Status |
|----|---------|-------------|------------|--------|
| CMD-0500 | /health | Display platform health | Developer | 🚧 |
| CMD-0501 | /config | Display configuration | Developer | 🚧 |
| CMD-0502 | /metrics | Display platform metrics | Developer | 🚧 |
| CMD-0503 | /cache | Cache information | Developer | 🚧 |
| CMD-0504 | /reload | Reload platform components | Maintainer | 🚧 |
| CMD-0505 | /logs | View platform logs | Maintainer | 🚧 |

---

# Administration Commands

| ID | Command | Description | Permission | Status |
|----|---------|-------------|------------|--------|
| CMD-0600 | /sync | Synchronize platform resources | Administrator | 🚧 |
| CMD-0601 | /update | Perform platform update | Administrator | 🚧 |
| CMD-0602 | /restart | Restart platform services | Administrator | 🚧 |
| CMD-0603 | /shutdown | Shut down platform | Administrator | 🚧 |
| CMD-0604 | /permissions | Manage platform permissions | Administrator | 🚧 |

---

# Reserved Commands

The following command ranges are reserved for future
expansion.

| Range | Purpose |
|--------|---------|
| CMD-0700–0799 | AI & Conversation Commands |
| CMD-0800–0899 | Knowledge Management |
| CMD-0900–0999 | Experimental Commands |

Reserved identifiers help maintain a stable command
numbering scheme as E.V.E. evolves.

---

# Command Naming Guidelines

Discord commands should:

- Be concise.
- Be descriptive.
- Use lowercase.
- Avoid abbreviations unless widely recognized.
- Represent a single primary action.

Commands should remain stable whenever possible to minimize
user disruption.

---

# Relationship to EVE-SPEC

This catalog complements the Discord Specifications.

Behavior is defined within Part II.

This document serves only as the authoritative inventory of
Discord commands.

---

# Future Expansion

Future revisions of this catalog may include:

- Command aliases
- Usage examples
- Slash command options
- Localization support
- Permission inheritance
- Feature availability by platform version

---

# Summary

The Discord Command Catalog provides a centralized inventory
of the commands supported by the Evolutionary Virtual
Engineer Discord interface.

It exists to improve discoverability, maintain consistency,
and provide a stable reference for developers, contributors,
and users.

---

## Navigation

**Previous**

None (First Reference Catalog)

**Part**

VII — Reference Catalogs

**Next**

EVE-0601 — Website Route Catalog

**Return**

← Part VII — Reference Catalogs

← EVE-SPEC

---

## Motto

*"Knowledge should evolve."*