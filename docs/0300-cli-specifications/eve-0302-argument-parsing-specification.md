# EVE-0302

# Argument Parsing Specification

> Part IV — CLI Specifications

---

## Document Information

| Property | Value |
|----------|-------|
| **Document ID** | EVE-0302 |
| **Series** | 0300 |
| **Title** | Argument Parsing Specification |
| **Part** | IV |
| **Status** | Draft |
| **Authoritative Specification** | EVE-SPEC |
| **Depends On** | EVE-0300, EVE-0301 |
| **Referenced By** | EVE-0303, EVE-2000 |

---

# Overview

This specification defines how the E.V.E. Command Line
Interface parses command-line arguments.

The parser converts user input into a validated command
structure before a PlatformRequest is created.

Argument parsing is deterministic and independent of
capabilities, providers, or platform logic.

---

# Purpose

The parser provides:

- Consistent command interpretation
- Deterministic behavior
- Helpful error reporting
- Stable scripting support
- Cross-platform compatibility
- Future extensibility

---

# Design Principles

The parser shall:

- Be deterministic
- Never execute commands
- Never invoke capabilities
- Never modify configuration
- Produce identical output for identical input
- Preserve argument ordering where required

---

# Architecture

```text
Command Line

↓

Tokenizer

↓

Argument Parser

↓

Validation

↓

Command Structure

↓

Platform Request

↓

Core Platform
```

Argument parsing is complete before any platform processing
begins.

---

# Parsing Stages

The parser executes the following stages.

## Stage 1

Read raw command line.

Example:

```text
eve docs EVE-0010
```

---

## Stage 2

Split into tokens.

Produces:

```text
eve

docs

EVE-0010
```

Quoted strings remain intact.

Example:

```text
eve ask "Explain Context Builder"
```

Produces:

```text
ask

Explain Context Builder
```

---

## Stage 3

Identify the command.

Example:

```text
docs
```

---

## Stage 4

Parse positional arguments.

Example:

```text
EVE-0010
```

---

## Stage 5

Parse options.

Example:

```text
--json
```

---

## Stage 6

Parse flags.

Example:

```text
-v
```

---

## Stage 7

Validate command structure.

Only after successful validation may a PlatformRequest be
constructed.

---

# Argument Types

The parser recognizes several argument types.

## Command

The primary operation.

Example:

```text
docs
```

---

## Positional Arguments

Ordered values supplied after the command.

Example:

```text
eve docs EVE-0010
```

Positional argument:

```
EVE-0010
```

---

## Named Options

Options begin with:

```text
--
```

Example:

```text
--provider
```

Named options accept values.

Example:

```text
--provider AI-0100
```

---

## Flags

Flags are boolean switches.

Examples:

```text
-v

--verbose

--json
```

Flags do not require values.

---

# Quoted Strings

Quoted strings are treated as a single argument.

Example:

```text
eve ask "Explain the Validation Engine"
```

Produces one argument:

```
Explain the Validation Engine
```

Nested quotes are escaped according to platform rules.

---

# Option Parsing

Options shall support both formats.

Long form:

```text
--provider AI-0100
```

Equals form:

```text
--provider=AI-0100
```

Both produce identical parsed output.

---

# Flag Parsing

Flags may be combined when compatible.

Example:

```text
-v
```

or

```text
--verbose
```

Multiple flags may appear in any order.

---

# Positional Rules

Positional arguments preserve ordering.

Example:

```text
eve compare EVE-0010 EVE-0011
```

Argument 1:

```
EVE-0010
```

Argument 2:

```
EVE-0011
```

The parser shall never reorder positional arguments.

---

# Unknown Options

Unknown options shall generate a validation error.

Example:

```text
--banana
```

Produces:

- Unknown option
- Suggested alternatives
- Non-zero exit code

---

# Missing Values

Required option values shall be enforced.

Example:

```text
--provider
```

Produces:

```
Missing value for option --provider
```

---

# Duplicate Options

Duplicate options shall follow specification-defined
behavior.

Unless otherwise documented:

- Last value wins
- A warning may be emitted

Example:

```text
--provider AI-0000

--provider AI-0100
```

Effective value:

```
AI-0100
```

---

# Validation

The parser validates:

- Command exists
- Option names
- Required arguments
- Required values
- Argument count
- Flag compatibility

Semantic validation belongs to the Validation Engine.

---

# Platform Independence

Argument parsing shall behave identically on supported
platforms.

Supported environments include:

- Windows
- Linux
- macOS

Platform-specific shell behavior shall be normalized before
parsing.

---

# Error Reporting

Parser errors include:

- Unknown command
- Unknown option
- Missing argument
- Missing option value
- Invalid syntax
- Invalid quoting

Errors shall be deterministic and descriptive.

---

# Output Model

Successful parsing produces a structured command model.

Example:

```text
Command

Arguments

Options

Flags

Raw Input
```

This structure is then converted into a PlatformRequest.

---

# Future Extensions

The parser architecture supports future additions:

- Environment variables
- Configuration defaults
- Command aliases
- Interactive completion
- Plugin-defined arguments
- Workspace arguments

---

# Relationship to Other Specifications

Depends on:

- EVE-0300 — CLI Interface Specification
- EVE-0301 — Command Specification

Referenced by:

- EVE-0303 — Response Formatting Specification
- EVE-2000 — Command Line Interface Guide

---

# Current Status

| Area | Status |
|------|--------|
| Tokenization | ✅ Defined |
| Command Parsing | ✅ Defined |
| Positional Arguments | ✅ Defined |
| Options | ✅ Defined |
| Flags | ✅ Defined |
| Validation Rules | ✅ Defined |
| Error Reporting | ✅ Defined |
| Response Formatting | ⏳ EVE-0303 |

---

## Series Navigation

**Previous**

EVE-0301 — Command Specification

**Current**

EVE-0302 — Argument Parsing Specification

**Next**

EVE-0303 — Response Formatting Specification

---

**Specification:** EVE-SPEC

**Part:** IV — CLI Specifications

**Document:** EVE-0302

**Status:** Draft

*"Knowledge should evolve."*
```