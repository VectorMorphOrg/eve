# EVE-0303

# Response Formatting Specification

> Part IV — CLI Specifications

---

## Document Information

| Property | Value |
|----------|-------|
| **Document ID** | EVE-0303 |
| **Series** | 0300 |
| **Title** | Response Formatting Specification |
| **Part** | IV |
| **Status** | Draft |
| **Authoritative Specification** | EVE-SPEC |
| **Depends On** | EVE-0300, EVE-0302 |
| **Referenced By** | EVE-2000 |

---

# Overview

This specification defines how the E.V.E. Command Line
Interface formats responses returned by the Core Platform.

Formatting is presentation only.

The formatter never changes, summarizes, ranks, or filters
the underlying PlatformResponse.

---

# Purpose

The Response Formatter provides:

- Human-readable output
- Machine-readable output
- Consistent formatting
- Deterministic presentation
- Script-friendly behavior
- Provider-independent output

---

# Design Principles

The formatter shall:

- Never modify PlatformResponse
- Never invoke capabilities
- Never communicate with AI providers
- Never perform reasoning
- Preserve evidence and citations
- Produce deterministic output

---

# Architecture

```text
PlatformResponse

↓

Response Formatter

↓

Console Output

↓

User
```

The formatter is the final stage of CLI execution.

---

# Formatting Modes

The CLI supports multiple output modes.

## Standard

Default human-readable output.

Example:

```text
Answer

Supporting Evidence

References

Execution Time
```

---

## JSON

Machine-readable output.

Example:

```text
eve docs EVE-0010 --json
```

Returns the complete PlatformResponse serialized to JSON.

---

## Pretty JSON

Indented JSON suitable for debugging.

Example:

```text
eve docs EVE-0010 --pretty
```

---

## Plain Text

Outputs only the response text.

Example:

```text
eve ask "Explain Context Builder" --plain
```

No metadata or diagnostics are displayed.

---

## Quiet

Suppresses informational output.

Example:

```text
eve docs EVE-0010 --quiet
```

Only essential results or errors are printed.

---

## Verbose

Displays additional execution information.

Example:

```text
eve docs EVE-0010 --verbose
```

May include:

- Execution trace
- Capability information
- Provider information
- Timing
- Diagnostics

---

# Standard Output Layout

The default format is:

```text
==================================================

Response

==================================================

<response text>

--------------------------------------------------

References

--------------------------------------------------

...

--------------------------------------------------

Diagnostics

--------------------------------------------------

...

--------------------------------------------------

Execution

--------------------------------------------------

...

```

The exact styling may evolve without changing the underlying
information.

---

# References

Evidence shall be displayed after the response.

Example:

```text
References

• EVE-0010 Platform Request
• EVE-0011 Platform Response
• EVE-1011 Knowledge Engine
```

References preserve the deterministic reasoning pipeline.

---

# Diagnostics

Diagnostics may include:

- Active provider
- Context package ID
- Knowledge objects used
- Search result count
- Citation bundle count
- Execution warnings

Diagnostics never alter the response.

---

# Execution Trace

Verbose mode may display the reasoning pipeline.

Example:

```text
Validation Engine

Capability Engine

Search Engine

Ranking Engine

Citation Engine

Context Assembler

Provider Formatter

Ollama Provider

Response Validation
```

Execution order shall match the actual pipeline.

---

# Errors

Errors shall be clearly separated from normal output.

Example:

```text
ERROR

Unknown command

Use:

eve help
```

Errors include:

- Invalid arguments
- Validation failures
- Capability failures
- Provider failures
- Configuration errors
- Internal platform errors

---

# Exit Codes

The formatter shall preserve platform exit codes.

Typical values:

| Code | Meaning |
|------|---------|
| 0 | Success |
| 1 | General error |
| 2 | Invalid arguments |
| 3 | Validation failure |
| 4 | Provider unavailable |
| 5 | Internal error |

Additional exit codes may be defined by future specifications.

---

# Color Support

When supported by the terminal, the formatter may use color.

Recommended usage:

| Color | Meaning |
|--------|----------|
| Green | Success |
| Yellow | Warning |
| Red | Error |
| Blue | Information |

Color shall never be required to understand output.

---

# Machine Readability

JSON output shall preserve:

- PlatformResponse
- AIResponse
- References
- Diagnostics
- Trace
- Metadata

No information is omitted in JSON mode.

---

# Provider Independence

Output formatting shall never expose provider-specific
implementation details unless explicitly requested.

The formatter displays responses consistently regardless of
whether they originate from:

- Null Provider
- Ollama
- OpenAI
- Anthropic
- LM Studio

---

# Future Extensions

The formatter is designed to support future formats,
including:

- Markdown
- HTML
- Rich terminal output
- YAML
- XML
- CSV
- Interactive TUI rendering

---

# Relationship to Other Specifications

Depends on:

- EVE-0300 — CLI Interface Specification
- EVE-0302 — Argument Parsing Specification

Referenced by:

- EVE-2000 — Command Line Interface Guide

---

# Current Status

| Area | Status |
|------|--------|
| Standard Output | ✅ Defined |
| JSON Output | ✅ Defined |
| Plain Text Output | ✅ Defined |
| Verbose Mode | ✅ Defined |
| Error Formatting | ✅ Defined |
| Exit Codes | ✅ Defined |
| Execution Trace | ✅ Defined |
| Rich Terminal Output | ⏳ Future |

---

## Series Navigation

**Previous**

EVE-0302 — Argument Parsing Specification

**Current**

EVE-0303 — Response Formatting Specification

**Next**

EVE-0304 — Interactive Shell Specification

---

**Specification:** EVE-SPEC

**Part:** IV — CLI Specifications

**Document:** EVE-0303

**Status:** Draft

*"Knowledge should evolve."*