# EVE-0103

# Discord Response and Formatting Specification

**Project:** E.V.E. (Evolutionary Virtual Engineer)

**Document Version:** 1.0.0

**Published:** July 2026

**Authors:** VectorMorph Research Initiative (VMRI)

**Status:** Foundational Specification

---

# Purpose

This document defines how E.V.E. presents information through
Discord.

Formatting improves readability and user experience.

Formatting shall never modify factual content.

Knowledge originates from the Core Platform.

Presentation belongs exclusively to the Discord Interface.

---

# Design Philosophy

Presentation is separate from information.

The Core Platform determines:

- Facts
- Citations
- Warnings
- Metadata

The Discord Interface determines:

- Layout
- Embeds
- Buttons
- Colors
- Pagination

Discord formatting should never alter the meaning of
platform responses.

---

# Response Flow

```
Response Object

↓

Discord Formatter

↓

Discord Embed

↓

Discord API

↓

User
```

The Discord Formatter never changes the Response Object.

---

# Response Types

The Discord Interface shall support multiple response types.

Examples include:

- Plain Text
- Rich Embeds
- Multi-Page Responses
- Code Blocks
- Tables
- File Attachments

Future Discord features may be supported without modifying
the Core Platform.

---

# Embed Design

Embeds should remain clean and consistent.

An embed may contain:

- Title
- Description
- Fields
- Footer
- Timestamp
- Author
- Thumbnail
- Image

Embeds should prioritize readability.

---

# Message Structure

Responses should generally follow this structure.

```
Title

↓

Summary

↓

Detailed Information

↓

References

↓

Suggested Next Steps
```

The structure may adapt to the response type.

---

# Code Formatting

Technical information should use Discord code blocks when
appropriate.

Examples include:

- Commands
- JSON
- Configuration
- Shell Commands
- Source Code

Language identifiers should be included whenever practical.

---

# Citations

Documentation references should remain visible.

Examples include:

- EVE-0001
- EVE-0004
- AEG-0000
- RFC-0003

References should appear near the end of the response.

---

# Pagination

Large responses should support pagination.

Examples include:

- Long documentation
- Search results
- Repository listings
- Command catalogs

Pagination should preserve response order.

---

# Interactive Components

The Discord Interface may support:

- Buttons
- Select Menus
- Modals
- Context Menus

Interactive components should improve navigation.

They should never replace documentation.

---

# Error Messages

Errors should remain consistent.

Error responses should include:

- Error Summary
- Explanation
- Suggested Resolution

Internal implementation details should never be exposed.

---

# Warnings

Warnings should remain visually distinct.

Examples include:

- Experimental Features
- Draft Specifications
- Deprecated Commands
- Missing Documentation

Warnings should not obscure primary information.

---

# Success Responses

Successful operations should clearly indicate completion.

Examples include:

- Search Completed
- Documentation Loaded
- Repository Indexed
- Configuration Updated

Success responses should remain concise.

---

# Accessibility

Responses should prioritize accessibility.

Guidelines include:

- Readable formatting
- Clear headings
- Consistent terminology
- Logical ordering

Formatting should improve readability for all users.

---

# Attachments

The Discord Interface may provide attachments including:

- Markdown Documents
- JSON
- Logs
- Reports
- Generated Files

Attachments should remain secondary to the primary response.

---

# Performance

Formatting should remain efficient.

Presentation should never become a bottleneck for platform
performance.

---

# Future Expansion

Future revisions may support:

- Rich Media
- Voice Responses
- Interactive Documentation
- Live Progress Indicators
- Collaborative Sessions
- Embedded Diagrams

Future additions should preserve compatibility with the Core
Platform.

---

# Summary

The Discord Formatter transforms standardized platform
responses into Discord-native presentations.

Formatting improves usability.

It never changes facts.

Knowledge remains authoritative.

Presentation remains interface-specific.

---

# Motto

> "Knowledge should evolve."

---

© 2026 VectorMorph Research Initiative (VMRI)

Released under the MIT License unless otherwise noted.