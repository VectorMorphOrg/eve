# EVE-0301

# Command Specification

> Part IV — CLI Specifications

---

## Document Information

| Property | Value |
|----------|-------|
| **Document ID** | EVE-0301 |
| **Series** | 0300 |
| **Title** | Command Specification |
| **Part** | IV |
| **Status** | Draft |
| **Authoritative Specification** | EVE-SPEC |
| **Depends On** | EVE-0300 |
| **Referenced By** | EVE-0302, EVE-2000 |

---

# Overview

This specification defines the command architecture for the
E.V.E. Command Line Interface.

Commands provide the user-facing interface to the Core
Platform.

Every command ultimately constructs a Platform Request and
invokes the deterministic reasoning pipeline.

Commands themselves contain no engineering logic.

---

# Purpose

The command system exists to provide:

- A consistent user experience
- Discoverable capabilities
- Scriptable automation
- Stable interfaces across releases
- Deterministic execution
- Provider-independent interaction

---

# Design Principles

The command architecture follows these principles:

- One command = one capability
- Commands never bypass the Core Platform
- Stable syntax
- Human readable
- Machine friendly
- Extensible without breaking existing commands
- Provider independent

---

# Architecture

```text
User

↓

CLI Command

↓

Command Parser

↓

Platform Request

↓

Validation Engine

↓

Capability Engine

↓

Reasoning Pipeline

↓

AI Provider

↓

Platform Response

↓

CLI Formatter
```

Commands are translated into Platform Requests.

No command performs engineering work directly.

---

# Command Categories

Commands are grouped by responsibility.

## Engineering

General engineering assistance.

Examples:

```text
eve ask
eve search
eve docs
```

---

## Platform

Platform administration.

Examples:

```text
eve status
eve version
eve info
```

---

## Configuration

Configuration management.

Examples:

```text
eve config
eve provider
```

---

## Development

Developer utilities.

Examples:

```text
eve capability
eve validate
eve test
eve benchmark
```

---

# Core Commands

## ask

General engineering assistant.

Example:

```text
eve ask "Explain the Validation Engine."
```

Maps to:

```
CAP-0102
```

---

## docs

Search engineering documentation.

Example:

```text
eve docs EVE-0010
```

Maps to:

```
CAP-0102
```

---

## search

Performs deterministic knowledge search.

Example:

```text
eve search "Knowledge Graph"
```

Maps to:

```
CAP-0102
```

---

## status

Displays platform status.

Example:

```text
eve status
```

Returns:

- Active provider
- Indexed repositories
- Loaded capabilities
- Configuration summary
- Platform diagnostics

---

## version

Displays build information.

Example:

```text
eve version
```

Returns:

- Version
- Git revision
- Build date
- Specification version

---

## provider

Displays or changes the active AI provider.

Examples:

```text
eve provider

eve provider list

eve provider use AI-0100
```

The Core Platform remains provider independent.

---

## config

Reads or updates configuration.

Examples:

```text
eve config

eve config get

eve config set
```

---

## capability

Displays registered capabilities.

Examples:

```text
eve capability list

eve capability info CAP-0102
```

---

## validate

Runs validation against platform inputs.

Example:

```text
eve validate request.json
```

---

## test

Executes diagnostic or validation tests.

Example:

```text
eve test
```

---

## benchmark

Runs platform performance measurements.

Example:

```text
eve benchmark
```

---

# Command Format

General syntax:

```text
eve <command> [arguments] [options]
```

Examples:

```text
eve ask "Explain EVE-0012"

eve docs EVE-0007

eve search "Knowledge Graph"

eve provider list
```

---

# Command Resolution

Command execution follows a deterministic pipeline.

```text
Parse

↓

Validate

↓

Lookup Capability

↓

Create PlatformRequest

↓

CorePlatform::process()

↓

PlatformResponse
```

Every command follows this sequence.

---

# Aliases

Commands may provide aliases.

Example:

| Command | Alias |
|----------|-------|
| ask | query |
| docs | document |
| search | find |
| version | -v |

Aliases must behave identically to their canonical command.

---

# Help System

Every command shall provide built-in help.

Examples:

```text
eve help

eve ask --help

eve provider --help
```

Help output shall include:

- Description
- Syntax
- Parameters
- Options
- Examples
- Exit codes

---

# Unknown Commands

Unknown commands shall never be ignored.

Example:

```text
eve abc123
```

Produces:

- Unknown command
- Suggested alternatives
- Exit code

---

# Reserved Commands

The following command namespaces are reserved for future
expansion:

- plugin
- session
- memory
- workspace
- workflow
- remote

Reserved commands shall not be implemented until formally
specified.

---

# Provider Independence

Commands shall never expose provider-specific behavior.

Examples:

Correct:

```text
eve ask "Explain Context Builder"
```

Incorrect:

```text
eve ask --ollama
```

Provider selection belongs to platform configuration.

---

# Scripting

Commands shall support scripting through:

- Deterministic output
- Stable exit codes
- JSON output
- Non-interactive execution

Example:

```text
eve docs EVE-0010 --json
```

---

# Extensibility

New commands may be introduced without affecting existing
commands.

Future command groups include:

- plugins
- workflows
- repositories
- deployment
- distributed execution

---

# Relationship to Other Specifications

Depends on:

- EVE-0300 — CLI Interface Specification

Referenced by:

- EVE-0302 — Argument Parsing Specification
- EVE-2000 — Command Line Interface Guide

---

# Current Status

| Area | Status |
|------|--------|
| Command Architecture | ✅ Defined |
| Command Categories | ✅ Defined |
| Core Commands | ✅ Defined |
| Command Resolution | ✅ Defined |
| Help System | ✅ Defined |
| Aliases | ✅ Defined |
| Argument Parsing | ⏳ EVE-0302 |
| Response Formatting | ⏳ EVE-0303 |

---

## Series Navigation

**Previous**

EVE-0300 — CLI Interface Specification

**Current**

EVE-0301 — Command Specification

**Next**

EVE-0302 — Argument Parsing Specification

---

**Specification:** EVE-SPEC

**Part:** IV — CLI Specifications

**Document:** EVE-0301

**Status:** Draft

*"Knowledge should evolve."*