# EVE-0300

# CLI Interface Specification

> Part IV — CLI Specifications

---

## Document Information

| Property | Value |
|----------|-------|
| **Document ID** | EVE-0300 |
| **Series** | 0300 |
| **Title** | CLI Interface Specification |
| **Part** | IV |
| **Status** | Draft |
| **Authoritative Specification** | EVE-SPEC |
| **Depends On** | Part I — Core Platform Specifications |
| **Referenced By** | EVE-0301 through EVE-0309 |

---

# Overview

This specification defines the official Command Line Interface
(CLI) architecture for the Evolutionary Virtual Engineer
(E.V.E.).

The CLI is an interface layer that exposes the Core Platform
through a deterministic command-line environment.

The CLI does not contain business logic, knowledge retrieval,
reasoning, validation, or AI orchestration.

Instead, it translates command-line input into Platform
Requests and formats Platform Responses for terminal output.

---

# Purpose

The CLI exists to provide:

- Interactive engineering assistance
- Scriptable automation
- Local development tooling
- Administrative platform access
- Testing and debugging capabilities
- Provider-independent user interaction

The CLI shall remain interface-only.

All engineering logic resides within the Core Platform.

---

# Design Philosophy

The CLI follows the architectural principles established by
Part I.

Core principles include:

- Interface Independence
- Platform First
- Capability-Oriented Design
- Provider Independence
- Deterministic Behavior
- Consistent User Experience
- Machine-Friendly Automation

---

# Responsibilities

The CLI is responsible for:

- Parsing command-line arguments
- Constructing Platform Requests
- Invoking the Core Platform
- Formatting Platform Responses
- Returning deterministic exit codes
- Displaying diagnostics

The CLI is **not** responsible for:

- Knowledge retrieval
- Validation logic
- Capability execution
- AI provider selection
- Context construction
- Search
- Ranking
- Citation generation

Those responsibilities belong to the Core Platform.

---

# Architecture

```text
User

↓

Command Line

↓

CLI Parser

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

↓

Terminal Output
```

The CLI is a thin translation layer between terminal input
and the Core Platform.

---

# Execution Pipeline

Every CLI invocation follows the same deterministic flow.

```text
argv[]

↓

Command Parser

↓

Command Validation

↓

PlatformRequest

↓

CorePlatform::process()

↓

PlatformResponse

↓

Formatter

↓

stdout / stderr
```

The CLI shall never bypass the Core Platform.

---

# Operating Modes

The CLI supports multiple execution modes.

## Command Mode

Executes a single command before exiting.

Example:

```text
eve ask "Explain EVE-0012"
```

---

## Interactive Shell

Launches an interactive engineering session.

Example:

```text
eve
```

The interactive shell maintains session state while each
individual command remains deterministic.

---

## Batch Mode

Allows automation from scripts.

Example:

```text
eve docs EVE-0007 --json
```

---

## Non-Interactive Mode

Commands shall operate correctly when stdin/stdout are
redirected.

This allows integration with build systems, CI pipelines,
and shell scripting.

---

# Platform Integration

The CLI communicates exclusively through the Platform
Request and Platform Response contracts defined by Part I.

Every user command ultimately produces:

```text
PlatformRequest
```

Every successful execution returns:

```text
PlatformResponse
```

No interface-specific execution paths shall exist.

---

# Capability Mapping

Commands map directly to capabilities.

Example mappings include:

| CLI Command | Capability |
|-------------|------------|
| ask | Documentation Search |
| docs | Documentation Search |
| search | Documentation Search |
| status | Platform Status |
| provider | Provider Management |
| config | Configuration |
| capability | Capability Management |

Specific mappings are defined by EVE-0301.

---

# AI Provider Independence

The CLI is provider-agnostic.

Users interact with engineering capabilities rather than
individual AI models.

The active provider is selected by the Core Platform.

Switching providers shall not require changes to CLI
behavior.

---

# Error Handling

The CLI shall distinguish between:

- Parsing errors
- Validation failures
- Capability failures
- Provider failures
- Internal platform errors

Errors shall be deterministic and human-readable.

Exit codes are defined by EVE-0307.

---

# Output

The CLI supports multiple output formats.

Examples include:

- Human-readable
- JSON
- YAML
- Plain text

Formatting behavior is defined in EVE-0303.

---

# Interactive Sessions

Interactive sessions provide a conversational command-line
environment while continuing to use Platform Requests for
each command.

Sessions may include:

- Command history
- Context preservation
- Provider status
- Session diagnostics

Session behavior is specified by EVE-0304.

---

# Configuration

The CLI reads configuration from the platform
configuration system.

Examples include:

- Active AI provider
- Default model
- Output format
- Logging level
- Repository locations

Configuration behavior is defined by EVE-0305.

---

# Logging

Diagnostic output is separated from command output.

Supported levels include:

- Error
- Warning
- Information
- Debug
- Trace

Logging behavior is specified by EVE-0306.

---

# Security Considerations

The CLI shall:

- Validate all user input
- Avoid execution of arbitrary commands
- Protect configuration secrets
- Respect provider authentication
- Preserve deterministic execution

---

# Relationship to EVE-SPEC

This specification depends on:

- Part I — Core Platform Specifications

This specification defines the interface consumed by future
CLI implementations.

Implementation guidance is provided by:

- EVE-2000 — Command Line Interface Guide

---

# Future Expansion

Future revisions may introduce:

- Plugin commands
- Shell auto-completion
- Rich terminal UI
- Progress indicators
- Streaming AI responses
- Multi-session management

These capabilities shall not alter the underlying Platform
Request architecture.

---

# Current Status

| Area | Status |
|------|--------|
| CLI Architecture | ✅ Defined |
| Execution Pipeline | ✅ Defined |
| Platform Integration | ✅ Defined |
| Operating Modes | ✅ Defined |
| Provider Independence | ✅ Defined |
| Command Specification | ⏳ EVE-0301 |
| Argument Parsing | ⏳ EVE-0302 |
| Output Formatting | ⏳ EVE-0303 |
| Interactive Shell | ⏳ EVE-0304 |
| Configuration | ⏳ EVE-0305 |
| Logging | ⏳ EVE-0306 |
| Exit Codes | ⏳ EVE-0307 |

---

## Series Navigation

**Previous**

Part III — Website Specifications

**Current**

EVE-0300 — CLI Interface Specification

**Next**

EVE-0301 — Command Specification

---

**Specification:** EVE-SPEC

**Part:** IV — CLI Specifications

**Document:** EVE-0300

**Status:** Draft

*"Knowledge should evolve."*