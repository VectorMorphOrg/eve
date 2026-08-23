# EVE-0307

# Exit Code Specification

> Part IV — CLI Specifications

---

## Document Information

| Property | Value |
|----------|-------|
| **Document ID** | EVE-0307 |
| **Series** | 0300 |
| **Title** | Exit Code Specification |
| **Part** | IV |
| **Status** | Draft |
| **Authoritative Specification** | EVE-SPEC |
| **Depends On** | EVE-0300, EVE-0306 |
| **Referenced By** | EVE-2000 |

---

# Overview

This specification defines the exit code architecture for the
Evolutionary Virtual Engineer (E.V.E.) Command Line
Interface.

Exit codes provide a deterministic mechanism for communicating
execution results to users, shell environments, automation
scripts, continuous integration systems, and external tools.

Exit codes are part of the interface contract between the CLI
and the operating system.

---

# Purpose

The exit code system provides:

- Deterministic execution status
- Script-friendly automation
- Platform-independent behavior
- Consistent error reporting
- CI/CD integration
- Shell interoperability

---

# Design Principles

Exit codes shall:

- Be deterministic
- Represent execution outcome only
- Never depend on the AI provider
- Never expose implementation details
- Remain stable across compatible releases
- Be suitable for scripting and automation

---

# Architecture

```text
PlatformRequest

↓

Core Platform

↓

PlatformResponse

↓

CLI Formatter

↓

Exit Code

↓

Operating System
```

The exit code is produced after the PlatformResponse has been
validated and formatted.

---

# Success

A successful command shall return:

| Code | Meaning |
|------|---------|
| **0** | Successful execution |

Successful execution indicates that the requested capability
completed without fatal errors.

---

# General Error Codes

| Code | Meaning |
|------|----------|
| 1 | General failure |
| 2 | Invalid command |
| 3 | Invalid arguments |
| 4 | Validation failure |
| 5 | Capability execution failure |
| 6 | Configuration error |
| 7 | Provider error |
| 8 | Internal platform error |

These codes are reserved and shall remain stable.

---

# Validation Errors

Validation failures include:

- Missing required parameters
- Invalid argument values
- Unsupported request versions
- Permission failures
- Capability validation failures

Exit Code:

```text
4
```

---

# Capability Errors

Capability execution failures include:

- Capability not found
- Capability disabled
- Capability execution exception
- Unsupported operation

Exit Code:

```text
5
```

---

# Configuration Errors

Configuration failures include:

- Missing configuration
- Invalid configuration
- Corrupted configuration
- Missing repository
- Invalid provider configuration

Exit Code:

```text
6
```

---

# Provider Errors

Provider failures include:

- Provider unavailable
- Connection timeout
- Authentication failure
- Invalid model
- Provider returned an error

Exit Code:

```text
7
```

Provider failures do not imply that reasoning failed.

---

# Internal Errors

Unexpected platform failures return:

```text
8
```

Examples include:

- Unhandled exceptions
- Internal invariants violated
- Unexpected runtime failures

These indicate defects within the platform rather than user
input.

---

# Exit Code Resolution

When multiple failures occur, the CLI shall return the first
fatal error encountered in execution order.

Example execution order:

```text
Argument Parsing

↓

Validation

↓

Capability

↓

Provider

↓

Formatting
```

Earlier failures take precedence.

---

# Interactive Shell

The Interactive Shell shall not terminate on command
failures.

Instead:

- The command returns an exit code internally
- The error is displayed
- The prompt is shown again

Example:

```text
> docs INVALID

Validation Error

>

```

The shell itself exits with code **0** unless a fatal shell
failure occurs.

---

# Batch Execution

Batch execution shall always return a meaningful exit code.

Example:

```text
eve docs EVE-0012 --json
```

The exit code may be inspected by shell scripts.

Example:

```bash
if eve docs EVE-0012; then
    echo Success
else
    echo Failed
fi
```

---

# Platform Independence

Exit code semantics shall remain identical across supported
platforms.

Supported environments include:

- Windows
- Linux
- macOS

Operating-system-specific conventions shall not alter exit
code meanings.

---

# Reserved Exit Codes

The following ranges are reserved for future expansion.

| Range | Purpose |
|--------|----------|
| 20–39 | Interface-specific extensions |
| 40–59 | Distributed Capability Network |
| 60–79 | Plugin system |
| 80–99 | Future platform services |

Reserved codes shall not be assigned until formally
specified.

---

# Provider Independence

Exit codes shall not reveal provider implementation details.

Example:

Allowed:

```text
7

Provider unavailable
```

Not allowed:

```text
Ollama socket error 10061
```

Provider-specific diagnostics belong in logging, not exit
codes.

---

# Future Extensions

Future specifications may define additional exit codes for:

- Conversation memory
- Distributed execution
- Plugin loading
- Remote repositories
- Authentication
- Licensing
- Multi-node orchestration

These additions shall preserve backward compatibility.

---

# Relationship to Other Specifications

Depends on:

- EVE-0300 — CLI Interface Specification
- EVE-0306 — Logging Specification

Implementation guidance is provided by:

- EVE-2000 — Command Line Interface Guide

---

# Current Status

| Area | Status |
|------|--------|
| Exit Code Architecture | ✅ Defined |
| Success Codes | ✅ Defined |
| Error Codes | ✅ Defined |
| Validation Errors | ✅ Defined |
| Capability Errors | ✅ Defined |
| Provider Errors | ✅ Defined |
| Interactive Shell Behavior | ✅ Defined |
| Reserved Exit Codes | ✅ Defined |

---

## Series Navigation

**Previous**

EVE-0306 — Logging Specification

**Current**

EVE-0307 — Exit Code Specification

**Next**

EVE-0308 — Plugin Command Specification *(Reserved)*

---

**Specification:** EVE-SPEC

**Part:** IV — CLI Specifications

**Document:** EVE-0307

**Status:** Draft

*"Knowledge should evolve."*