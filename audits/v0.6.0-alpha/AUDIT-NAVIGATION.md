# Navigation Audit — v0.6.0-alpha

**Audit Date:** July 9, 2026  
**Scope:** All 89 Markdown files under `docs/`  
**Standard Format (target):**

```markdown
## Navigation

**Previous**
...

**Current**
...

**Next**
...

**Return**

← Part X — ...
← EVE-SPEC
```

**Method:** Automated field extraction plus manual verification of titles, IDs, and chain integrity. No repository files were modified.

---

# Summary

| Metric | Value |
|--------|-------|
| Total files audited | 89 |
| **Files passed** (standard `## Navigation` + all fields) | **25 (28%)** |
| **Files with issues** | **64 (72%)** |
| Missing navigation entirely | 39 |
| Using `## Series Navigation` (non-standard header) | 27 |
| Using legacy `**Part**` instead of `**Current**` | 11 |
| Empty files (0 bytes) | 16 |
| Broken Previous/Next chains | 4 confirmed |

### Correctly Standardized Series (Facts)

- **EVE-1000–EVE-1011** (Implementation Guides): All 12 documents pass.
- **EVE-3001–EVE-3009** (ADRs, except README): All 9 ADR documents pass standard navigation.
- **EVE-0604, EVE-0605, EVE-0607** (Reference Catalogs): Pass standard navigation.
- **EVE-0005, EVE-0007** (Core Platform): Pass standard navigation header and fields.

### README Consistency (Facts)

Every EVE-SPEC series folder under `docs/` contains a `README.md` (15 README files total). All series READMEs use `## Series Navigation` rather than `## Navigation`. None fully match the five-field standard without format exceptions.

---

# Files Passed (25)

| File | Document ID |
|------|-------------|
| `docs/0000-core-platform-specifications/eve-0005-context-builder-specification.md` | EVE-0005 |
| `docs/0000-core-platform-specifications/eve-0007-ai-provider-specification.md` | EVE-0007 |
| `docs/0600-reference-catalogs/eve-0604-capability-catalog.md` | EVE-0604 |
| `docs/0600-reference-catalogs/eve-0605-service-catalog.md` | EVE-0605 |
| `docs/0600-reference-catalogs/eve-0607-ai-provider-catalog.md` | EVE-0607 |
| `docs/1000-implementation-guides/eve-1000-implementation-roadmap.md` | EVE-1000 |
| `docs/1000-implementation-guides/eve-1001-core-platform-implementation-guide.md` | EVE-1001 |
| `docs/1000-implementation-guides/eve-1002-validation-engine-implementation-guide.md` | EVE-1002 |
| `docs/1000-implementation-guides/eve-1003-capability-engine-implementation-guide.md` | EVE-1003 |
| `docs/1000-implementation-guides/eve-1004-service-layer-implementation-guide.md` | EVE-1004 |
| `docs/1000-implementation-guides/eve-1005-knowledge-object-implementation-guide.md` | EVE-1005 |
| `docs/1000-implementation-guides/eve-1006-knowledge-graph-implementation-guide.md` | EVE-1006 |
| `docs/1000-implementation-guides/eve-1007-knowledge-index-implementation-guide.md` | EVE-1007 |
| `docs/1000-implementation-guides/eve-1008-reasoning-pipeline-implementation-guide.md` | EVE-1008 |
| `docs/1000-implementation-guides/eve-1009-context-assembly-implementation-guide.md` | EVE-1009 |
| `docs/1000-implementation-guides/eve-1010-ai-provider-implementation-guide.md` | EVE-1010 |
| `docs/1000-implementation-guides/eve-1011-knowledge-engine-implementation-guide.md` | EVE-1011 |
| `docs/3000-architecture-decision-records/eve-3001-documentation-first-development.md` | EVE-3001 |
| `docs/3000-architecture-decision-records/eve-3002-capability-oriented-architecture.md` | EVE-3002 |
| `docs/3000-architecture-decision-records/eve-3003-knowledge-first-ai-architecture.md` | EVE-3003 |
| `docs/3000-architecture-decision-records/eve-3004-validation-pipeline.md` | EVE-3004 |
| `docs/3000-architecture-decision-records/eve-3005-ai-provider-abstraction.md` | EVE-3005 |
| `docs/3000-architecture-decision-records/eve-3006-repository-as-source-of-truth.md` | EVE-3006 |
| `docs/3000-architecture-decision-records/eve-3008-distributed-capability-network.md` | EVE-3008 |
| `docs/3000-architecture-decision-records/eve-3009-provider-independent-reasoning.md` | EVE-3009 |

**Note:** `eve-0007-ai-provider-specification.md` passes structural navigation checks but has an **incorrect Next title** (documented under Issues below).

---

# Files With Issues (64)

## A. Missing Navigation Section (39 files)

| File | ID | Issues | Recommended Fix |
|------|----|--------|-----------------|
| `docs/0000-core-platform-specifications/eve-0000-vision-and-design-philosophy.md` | EVE-0000 | No Navigation | Add standard nav; Previous=None, Next=EVE-0001 |
| `docs/0000-core-platform-specifications/eve-0001-system-architecture.md` | EVE-0001 | No Navigation | Add nav chain 0000→0001→0002 |
| `docs/0000-core-platform-specifications/eve-0002-capability-specification.md` | EVE-0002 | No Navigation | Add nav chain |
| `docs/0000-core-platform-specifications/eve-0003-service-layer-specification.md` | EVE-0003 | No Navigation | Add nav chain |
| `docs/0000-core-platform-specifications/eve-0004-knowledge-layer-specification.md` | EVE-0004 | No Navigation | Add nav chain |
| `docs/0000-core-platform-specifications/eve-0006-context-package-specification.md` | EVE-0006 | No Navigation | Add nav chain |
| `docs/0000-core-platform-specifications/eve-0008-personality-specification.md` | EVE-0008 | No Navigation | Add nav chain |
| `docs/0000-core-platform-specifications/eve-0009-documentation-integration-specification.md` | EVE-0009 | No Navigation | Add nav chain |
| `docs/0100-discord-specifications/eve-0100-discord-interface-specification.md` | EVE-0100 | No Navigation | Add Part II nav chain |
| `docs/0100-discord-specifications/eve-0101-discord-command-mapping.md` | EVE-0101 | No Navigation | Add nav chain |
| `docs/0100-discord-specifications/eve-0102-discord-permissions.md` | EVE-0102 | No Navigation | Add nav chain |
| `docs/0100-discord-specifications/eve-0103-discord-embeds-and-formatting.md` | EVE-0103 | No Navigation | Add nav chain |
| `docs/0200-website-specifications/eve-0200-website-interface-specification.md` | EVE-0200 | Empty file; no Navigation | Populate or remove placeholder |
| `docs/0200-website-specifications/eve-0201-web-chat-interface.md` | EVE-0201 | Empty file; no Navigation | Populate or remove placeholder |
| `docs/0300-cli-specifications/eve-0308-plugin-command-specification.md` | EVE-0308 | No Navigation | Add nav: Previous=EVE-0307, Next=EVE-0309 |
| `docs/0300-cli-specifications/eve-0309-cli-session-specification.md` | EVE-0309 | No Navigation | Add nav after EVE-0308 |
| `docs/0400-rest-api-specifications/eve-0400-rest-api-specification.md` | EVE-0400 | Empty file; no Navigation | Populate or remove placeholder |
| `docs/0500-platform-specifications/eve-0500-testing-specification.md` | EVE-0500 | Empty file; no Navigation | Populate or remove placeholder |
| `docs/0500-platform-specifications/eve-0501-deployment-specification.md` | EVE-0501 | Empty file; no Navigation | Populate or remove placeholder |
| `docs/0500-platform-specifications/eve-0502-build-system.md` | EVE-0502 | Empty file; no Navigation | Populate or remove placeholder |
| `docs/2000-developer-guides/eve-2000-command-line-interface-guide.md` | EVE-2000 | Empty file; no Navigation | Populate or remove placeholder |
| `docs/2000-developer-guides/eve-2001-rest-api-developer-guide.md` | EVE-2001 | Empty file; no Navigation | Populate or remove placeholder |
| `docs/2000-developer-guides/eve-2002-discord-integration-guide.md` | EVE-2002 | Empty file; no Navigation | Populate or remove placeholder |
| `docs/2000-developer-guides/eve-2003-website-integration-guide.md` | EVE-2003 | Empty file; no Navigation | Populate or remove placeholder |
| `docs/2000-developer-guides/eve-2004-provider-development-guide.md` | EVE-2004 | Empty file; no Navigation | Populate or remove placeholder |
| `docs/2000-developer-guides/eve-2005-capability-development-guide.md` | EVE-2005 | Empty file; no Navigation | Populate or remove placeholder |
| `docs/2000-developer-guides/eve-2006-service-development-guide.md` | EVE-2006 | Empty file; no Navigation | Populate or remove placeholder |
| `docs/2000-developer-guides/eve-2007-knowledge-repository-guide.md` | EVE-2007 | Empty file; no Navigation | Populate or remove placeholder |
| `docs/2000-developer-guides/eve-2008-testing-guide.md` | EVE-2008 | Empty file; no Navigation | Populate or remove placeholder |
| `docs/2000-developer-guides/eve-2009-release-process-guide.md` | EVE-2009 | Empty file; no Navigation | Populate or remove placeholder |

---

## B. Legacy `**Part**` Field Instead of `**Current**` (11 files)

| File | ID | Issues | Recommended Fix |
|------|----|--------|-----------------|
| `docs/0000-core-platform-specifications/eve-0010-platform-request-specification.md` | EVE-0010 | Uses `**Part**`; missing `**Current**`; incorrect Next title | Add Current=`EVE-0010 — Platform Request Specification`; fix Next to `Platform Response Specification` |
| `docs/0000-core-platform-specifications/eve-0011-platform-response-specification.md` | EVE-0011 | Uses `**Part**`; missing `**Current**`; Next=`Reserved` (EVE-0012 exists) | Add Current; set Next=EVE-0012 |
| `docs/0000-core-platform-specifications/eve-0012-validation-specification.md` | EVE-0012 | Uses `**Part**`; missing `**Current**`; Next=`Reserved` | Add Current; set Next=Part II or EVE-0100 |
| `docs/0600-reference-catalogs/eve-0600-discord-command-catalog.md` | EVE-0600 | Uses `**Part**`; missing `**Current**` | Add Current=`EVE-0600 — Discord Command Catalog` |
| `docs/0600-reference-catalogs/eve-0601-website-route-catalog.md` | EVE-0601 | Same | Add Current with document title |
| `docs/0600-reference-catalogs/eve-0602-cli-command-catalog.md` | EVE-0602 | Same | Add Current with document title |
| `docs/0600-reference-catalogs/eve-0603-rest-api-endpoint-catalog.md` | EVE-0603 | Same; Next=`Reserved` breaks chain to EVE-0604 | Add Current; set Next=EVE-0604 |
| `docs/0600-reference-catalogs/eve-0606-knowledge-object-catalog.md` | EVE-0606 | Uses `**Part**`; missing `**Current**` | Add Current |
| `docs/0600-reference-catalogs/eve-0608-repository-catalog.md` | EVE-0608 | Same | Add Current |
| `docs/0600-reference-catalogs/eve-0609-document-catalog.md` | EVE-0609 | Same | Add Current |

---

## C. Non-Standard `## Series Navigation` Header (27 files)

| File | ID | Issues | Recommended Fix |
|------|----|--------|-----------------|
| `docs/README.md` | README | Non-standard nav; missing Previous/Current/Next/Return fields | Adopt five-field Navigation |
| `docs/0000-core-platform-specifications/README.md` | README | Series Navigation; Return uses ``← EVE-SPEC (`/docs`)`` | Rename header; standardize Return |
| `docs/0100-discord-specifications/README.md` | README | Series Navigation | Rename to `## Navigation`; add Return if missing |
| `docs/0200-website-specifications/README.md` | README | Series Navigation | Same |
| `docs/0300-cli-specifications/README.md` | README | Series Navigation; **incorrect Next**=`Part II — EVE-2000 Implementation Guides` | Fix Next to EVE-0300 or Part V; add Return |
| `docs/0300-cli-specifications/eve-0300-cli-interface-specification.md` | EVE-0300 | Series Navigation; missing Return | Convert to standard Navigation + Return |
| `docs/0300-cli-specifications/eve-0301-command-specification.md` | EVE-0301 | Same | Same |
| `docs/0300-cli-specifications/eve-0302-argument-parsing-specification.md` | EVE-0302 | Same | Same |
| `docs/0300-cli-specifications/eve-0303-response-formatting-specification.md` | EVE-0303 | Same | Same |
| `docs/0300-cli-specifications/eve-0304-interactive-shell-specification.md` | EVE-0304 | Same | Same |
| `docs/0300-cli-specifications/eve-0305-configuration-specification.md` | EVE-0305 | Same | Same |
| `docs/0300-cli-specifications/eve-0306-logging-specification.md` | EVE-0306 | Same | Same |
| `docs/0300-cli-specifications/eve-0307-exit-code-specification.md` | EVE-0307 | Same; missing Return; Next skips EVE-0308/0309 | Add Return; chain through 0308→0309 |
| `docs/0400-rest-api-specifications/README.md` | README | Series Navigation | Standardize |
| `docs/0500-platform-specifications/README.md` | README | Series Navigation | Standardize |
| `docs/0600-reference-catalogs/README.md` | README | Series Navigation | Standardize |
| `docs/0700-testing-specifications/README.md` | README | Series Navigation | Standardize |
| `docs/0800-deployment-specifications/README.md` | README | Series Navigation | Standardize |
| `docs/0900-security-specifications/README.md` | README | Series Navigation | Standardize |
| `docs/1000-implementation-guides/README.md` | README | Series Navigation; missing Return | Add Return block |
| `docs/2000-developer-guides/README.md` | README | Series Navigation; missing Return | Add Return block |
| `docs/3000-architecture-decision-records/README.md` | README | Series Navigation; missing Return; Next=`Ideas & Future Concepts` (not a doc) | Add Return; point Next to Part XIV |
| `docs/9000-archive/README.md` | README | Series Navigation; Return uses ``← EVE-SPEC (`/docs`)`` | Standardize Return line |

---

## D. Incorrect Titles, IDs, Parts, or Links

| File | Issue (Fact) | Recommended Fix |
|------|--------------|-----------------|
| `docs/0000-core-platform-specifications/eve-0007-ai-provider-specification.md` | **Next** reads `EVE-0008 — Service Layer Specification`; EVE-0003 is Service Layer; EVE-0008 is Persona and Communication | Change Next to `EVE-0008 — Persona and Communication Specification` |
| `docs/0000-core-platform-specifications/eve-0010-platform-request-specification.md` | **Next** reads `EVE-0011 — Response Object Specification`; actual EVE-0011 title is Platform Response Specification | Fix Next title |
| `docs/0300-cli-specifications/README.md` | **Next**=`Part II — EVE-2000 Implementation Guides`; Part II is Discord; EVE-2000 is Part XII | Fix to `EVE-0300 — CLI Interface Specification` or Part V |
| `docs/0600-reference-catalogs/eve-0603-rest-api-endpoint-catalog.md` | **Next**=`Reserved` but EVE-0604 exists and lists EVE-0603 as Previous | Set Next=EVE-0604 |
| `docs/0000-core-platform-specifications/eve-0011-platform-response-specification.md` | **Next**=`Reserved` but EVE-0012 exists | Set Next=EVE-0012 |
| `docs/0000-core-platform-specifications/eve-0005-context-builder-specification.md` | Return uses `Part I — Engineering Specifications` | Align with `Part I — Core Platform Specifications` used in EVE-0010–0012 |
| `docs/0000-core-platform-specifications/eve-0007-ai-provider-specification.md` | Return uses `Part I — Engineering Specifications` | Same alignment |
| `docs/0100-discord-specifications/eve-0103-discord-embeds-and-formatting.md` | Filename `embeds-and-formatting`; README title is Discord Response and Formatting Specification | Rename file or update README title for consistency |

### Broken Links (Facts)

- **No Markdown hyperlinks** (`[text](*.md)`) exist anywhere under `docs/`. Navigation is plain-text EVE-ID based only. Broken links are therefore **reference/ID mismatches**, not dead `.md` hrefs.
- `README.md` (repository root) references `docs/adr/` which **does not exist**; ADRs are in `docs/3000-architecture-decision-records/`.

### EVE-3007 Next (Informational)

`eve-3007-knowledge-graph-architecture.md` Next points to `EVE-3008 — Distributed Capability Network *(Reserved)*`. File `eve-3008-distributed-capability-network.md` exists. Navigation is **correct**; reserved status is explicit.

---

# Formatting Differences

| Pattern | Standard | Observed In |
|---------|----------|-------------|
| Section header | `## Navigation` | 27 files use `## Series Navigation` |
| Current field | `**Current**` + EVE-ID + title | 11 files use `**Part**` only |
| Return EVE-SPEC | `← EVE-SPEC` | `0000/README.md`, `9000/README.md` use ``← EVE-SPEC (`/docs`)`` |
| Part I label | Single canonical name | Split: Engineering Specifications vs Core Platform Specifications |
| Post-nav footer | None required | 0300 CLI docs append `**Specification:**`, `**Part:**`, `**Document:**` blocks |
| Motto section | Optional | EVE-0010–0012, several 0600 catalogs use `## Motto` after Navigation |

---

# Recommended Fix Priority

1. Fix broken chains: EVE-0603→0604, EVE-0011→0012, EVE-0307→0308→0309
2. Fix incorrect Next titles: EVE-0007, EVE-0010
3. Fix `docs/0300-cli-specifications/README.md` incorrect Next Part reference
4. Replace `**Part**` with `**Current**` in 11 documents
5. Migrate `## Series Navigation` → `## Navigation` across 27 files
6. Add Navigation to EVE-0000–0004, 0006, 0008, 0009, EVE-0100–0103
7. Add Navigation to EVE-0308, EVE-0309
8. Standardize Part I Return label across Part I documents

---

*End of Navigation Audit — v0.6.0-alpha*
