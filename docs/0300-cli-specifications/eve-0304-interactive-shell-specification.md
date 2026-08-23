# EVE-0304

# Interactive Shell Specification

> Part IV — CLI Specifications

---

## Document Information

| Property | Value |
|----------|-------|
| **Document ID** | EVE-0304 |
| **Series** | 0300 |
| **Title** | Interactive Shell Specification |
| **Part** | IV |
| **Status** | Draft |
| **Authoritative Specification** | EVE-SPEC |
| **Depends On** | EVE-0300, EVE-0301, EVE-0302, EVE-0303 |
| **Referenced By** | EVE-2000 |

---

# Overview

This specification defines the interactive shell provided by
the Evolutionary Virtual Engineer (E.V.E.) Command Line
Interface.

The Interactive Shell provides a persistent terminal session
for engineering workflows while continuing to use the Core
Platform for every command.

The shell contains no engineering logic.

Each user command is converted into an independent
PlatformRequest.

---

# Purpose

The Interactive Shell exists to provide:

- Interactive engineering assistance
- Efficient multi-command workflows
- Session diagnostics
- Command history
- Improved developer experience
- Consistent platform access

---

# Design Principles

The shell shall:

- Be provider independent
- Be deterministic
- Never bypass the Core Platform
- Never execute engineering logic internally
- Preserve PlatformRequest boundaries
- Remain script-compatible

---

# Architecture

```text
User

↓

Interactive Shell

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

Formatter

↓

Terminal
```

The shell is a persistent interface.

The platform remains stateless between requests unless a
future specification introduces conversation memory.

---

# Session Lifecycle

An interactive session follows this lifecycle.

```text
Start Shell

↓

Load Configuration

↓

Display Banner

↓

Wait For Command

↓

Execute Platform Request

↓

Display Response

↓

Repeat

↓

Exit
```

---

# Launching the Shell

The shell is started when E.V.E. is executed without a
command.

Example:

```text
eve
```

Example startup:

```text
Evolutionary Virtual Engineer

Version 0.7.0-alpha

Type "help" for available commands.

>
```

---

# Prompt

The default prompt is:

```text
>
```

Future implementations may optionally support richer prompts.

Example:

```text
eve>
```

or

```text
E.V.E. >
```

Prompt customization is defined by future specifications.

---

# Command Execution

Each entered command is treated exactly as if it were
executed from the operating system command line.

Example:

```text
> ask "Explain EVE-0012"
```

is equivalent to:

```text
eve ask "Explain EVE-0012"
```

The shell performs no special routing.

---

# Built-In Commands

The shell provides a minimal set of built-in commands.

These commands do not invoke platform capabilities.

| Command | Description |
|----------|-------------|
| help | Display available commands |
| history | Display command history |
| clear | Clear terminal output |
| exit | Exit the shell |
| quit | Alias for exit |

---

# Platform Commands

All platform commands remain available.

Examples:

```text
> ask "Explain Context Builder"

> docs EVE-0012

> search Validation Engine

> status

> provider

> capability list
```

Each command creates a new PlatformRequest.

---

# Command History

The shell records commands entered during the active session.

History shall preserve:

- Command order
- Original text
- Timestamp (optional)

History shall not modify command execution.

Future implementations may support persistent history.

---

# Command Editing

Future implementations may support:

- Arrow key navigation
- Line editing
- Reverse search
- Multi-line editing
- Auto-completion

These features shall not affect command semantics.

---

# Session State

The shell maintains only interface state.

Examples include:

- Command history
- Prompt configuration
- Terminal preferences

The shell shall not maintain engineering reasoning state.

Conversation memory is defined by future specifications.

---

# Interrupt Handling

Interactive sessions shall gracefully handle interrupts.

Examples:

```text
Ctrl+C
```

Cancels the current command.

The shell remains active.

Example:

```text
Ctrl+D
```

or

```text
exit
```

Terminates the session.

---

# Error Handling

Errors shall never terminate the shell unless fatal.

Example:

```text
> docs ABC123
```

Produces:

```text
Validation Error

Unknown document identifier.

>
```

The prompt immediately returns.

---

# Diagnostics

The shell may expose diagnostic commands.

Examples:

```text
status

provider

capability list
```

Diagnostics remain platform capabilities whenever possible.

---

# Provider Independence

The shell shall never expose provider-specific behavior.

Example:

Correct:

```text
> ask "Explain Knowledge Graph"
```

Incorrect:

```text
> ollama ask ...
```

Provider selection belongs to the platform configuration.

---

# Session Banner

A typical startup banner may include:

```text
Evolutionary Virtual Engineer

Version 0.7.0-alpha

Provider:
Ollama

Repository:
VectorMorph

Type "help" for commands.

>
```

Banner contents are informational only.

---

# Future Extensions

Future shell capabilities may include:

- Conversation memory
- Multiple workspaces
- Repository switching
- Streaming responses
- Rich terminal UI
- Progress indicators
- Plugin commands
- Remote sessions
- Multi-user collaboration

These features shall preserve the PlatformRequest
architecture.

---

# Relationship to Other Specifications

Depends on:

- EVE-0300 — CLI Interface Specification
- EVE-0301 — Command Specification
- EVE-0302 — Argument Parsing Specification
- EVE-0303 — Response Formatting Specification

Referenced by:

- EVE-2000 — Command Line Interface Guide

---

# Current Status

| Area | Status |
|------|--------|
| Interactive Session | ✅ Defined |
| Session Lifecycle | ✅ Defined |
| Built-In Commands | ✅ Defined |
| Platform Commands | ✅ Defined |
| Command History | ✅ Defined |
| Error Recovery | ✅ Defined |
| Interrupt Handling | ✅ Defined |
| Conversation Memory | ⏳ Future |
| Auto-Completion | ⏳ Future |
| Rich Terminal UI | ⏳ Future |

---

## Series Navigation

**Previous**

EVE-0303 — Response Formatting Specification

**Current**

EVE-0304 — Interactive Shell Specification

**Next**

EVE-0305 — Configuration Specification

---

**Specification:** EVE-SPEC

**Part:** IV — CLI Specifications

**Document:** EVE-0304

**Status:** Draft

*"Knowledge should evolve."*