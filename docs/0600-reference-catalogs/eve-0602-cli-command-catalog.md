# EVE-0602

# CLI Command Catalog

**Project:** Evolutionary Virtual Engineer (E.V.E.)

**Specification:** EVE-SPEC

**Part:** VII — Reference Catalogs

**Document Version:** 1.0.0

**Published:** July 2026

**Authors:** VectorMorph Research Initiative (VMRI)

**Status:** Draft

---

# Purpose

This document provides the authoritative catalog of
command-line interface (CLI) commands supported by E.V.E.

Unlike engineering specifications, this catalog does not
define command behavior.

Instead, it serves as a centralized reference describing the
commands currently available or planned for the CLI
interface.

Behavioral requirements are defined within Part IV —
CLI Specifications.

---

# Scope

This catalog includes:

- General Commands
- Documentation Commands
- Repository Commands
- Development Commands
- System Commands
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

# Access Levels

| Access | Description |
|---------|-------------|
| User | Any local user |
| Developer | Project contributors |
| Maintainer | Repository maintainers |
| Administrator | System administrators |

---

# General Commands

| ID | Command | Description | Access | Status |
|----|---------|-------------|--------|--------|
| CLI-0001 | eve help | Display available commands | User | 🚧 |
| CLI-0002 | eve about | Display project information | User | 🚧 |
| CLI-0003 | eve version | Display version information | User | 🚧 |
| CLI-0004 | eve status | Display platform status | User | 🚧 |
| CLI-0005 | eve ping | Connectivity check | User | 🚧 |

---

# Documentation Commands

| ID | Command | Description | Access | Status |
|----|---------|-------------|--------|--------|
| CLI-0100 | eve docs | Browse documentation | User | 🚧 |
| CLI-0101 | eve spec | Open a specification | User | 🚧 |
| CLI-0102 | eve search | Search documentation | User | 🚧 |
| CLI-0103 | eve read | Read a document | User | 🚧 |
| CLI-0104 | eve part | Browse specification parts | User | 🚧 |
| CLI-0105 | eve series | Browse document series | User | 🚧 |
| CLI-0106 | eve index | View documentation index | User | 🚧 |

---

# Repository Commands

| ID | Command | Description | Access | Status |
|----|---------|-------------|--------|--------|
| CLI-0200 | eve repo | Repository information | User | 🚧 |
| CLI-0201 | eve repos | List repositories | User | 🚧 |
| CLI-0202 | eve branch | Branch information | User | 🚧 |
| CLI-0203 | eve issues | View repository issues | User | 🚧 |
| CLI-0204 | eve pulls | View pull requests | User | 🚧 |
| CLI-0205 | eve releases | View releases | User | 🚧 |

---

# Development Commands

| ID | Command | Description | Access | Status |
|----|---------|-------------|--------|--------|
| CLI-0300 | eve roadmap | Display roadmap | User | 🚧 |
| CLI-0301 | eve task | View project tasks | Developer | 🚧 |
| CLI-0302 | eve todo | View outstanding work | Developer | 🚧 |
| CLI-0303 | eve adr | Browse Architecture Decision Records | User | 🚧 |
| CLI-0304 | eve build | Build project | Developer | 🚧 |
| CLI-0305 | eve test | Execute test suite | Developer | 🚧 |

---

# System Commands

| ID | Command | Description | Access | Status |
|----|---------|-------------|--------|--------|
| CLI-0400 | eve config | Display configuration | Developer | 🚧 |
| CLI-0401 | eve cache | Cache management | Developer | 🚧 |
| CLI-0402 | eve plugins | Display installed plugins | Developer | 🚧 |
| CLI-0403 | eve providers | Display configured AI providers | Developer | 🚧 |
| CLI-0404 | eve context | Inspect generated context package | Developer | 🚧 |

---

# Diagnostics Commands

| ID | Command | Description | Access | Status |
|----|---------|-------------|--------|--------|
| CLI-0500 | eve health | Platform health check | Developer | 🚧 |
| CLI-0501 | eve metrics | Display runtime metrics | Developer | 🚧 |
| CLI-0502 | eve logs | View application logs | Maintainer | 🚧 |
| CLI-0503 | eve doctor | Diagnose common issues | Developer | 🚧 |
| CLI-0504 | eve validate | Validate installation | Developer | 🚧 |

---

# Administration Commands

| ID | Command | Description | Access | Status |
|----|---------|-------------|--------|--------|
| CLI-0600 | eve sync | Synchronize resources | Administrator | 🚧 |
| CLI-0601 | eve update | Update platform | Administrator | 🚧 |
| CLI-0602 | eve restart | Restart services | Administrator | 🚧 |
| CLI-0603 | eve shutdown | Shut down platform | Administrator | 🚧 |
| CLI-0604 | eve permissions | Manage permissions | Administrator | 🚧 |

---

# Reserved Commands

The following identifier ranges are reserved for future
expansion.

| Range | Purpose |
|--------|---------|
| CLI-0700–0799 | AI Conversation Commands |
| CLI-0800–0899 | Project Management |
| CLI-0900–0999 | Experimental Commands |

Reserved identifiers maintain a stable numbering scheme as
the CLI evolves.

---

# Command Naming Guidelines

CLI commands should:

- Use lowercase.
- Follow the `eve <command>` convention.
- Be descriptive.
- Represent a single primary action.
- Remain stable whenever possible.

Subcommands should follow consistent naming conventions
throughout the platform.

---

# Relationship to EVE-SPEC

This catalog complements Part IV — CLI Specifications.

Behavior is defined within the engineering specifications.

This document serves only as the authoritative inventory of
CLI commands.

---

# Future Expansion

Future revisions of this catalog may include:

- Command arguments
- Global flags
- Exit codes
- Shell completion
- Configuration profiles
- Interactive mode

---

# Summary

The CLI Command Catalog provides a centralized inventory of
the commands supported by the E.V.E. command-line interface.

It exists to improve discoverability, maintain consistency,
and provide a stable reference for developers,
contributors, and users.

---

## Navigation

**Previous**

EVE-0601 — Website Route Catalog

**Part**

VII — Reference Catalogs

**Next**

EVE-0603 — REST API Endpoint Catalog

**Return**

← Part VII — Reference Catalogs

← EVE-SPEC

---

## Motto

*"Knowledge should evolve."*