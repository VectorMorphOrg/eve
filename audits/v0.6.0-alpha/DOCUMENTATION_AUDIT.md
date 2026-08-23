# Documentation Audit — v0.6.0-alpha

**Audit Date:** July 9, 2026  
**Scope:** All content under `docs/` (89 Markdown files)  
**Authority:** EVE-SPEC (`docs/README.md`)  
**Method:** Full corpus review; no files modified.

---

# Executive Summary

| Metric | Assessment |
|--------|------------|
| Total Markdown files | 89 |
| Files with substantive content | 73 (82%) |
| Empty placeholder files | 16 (18%) |
| Standard navigation compliance | 28% (25/89) |
| Duplicate document IDs | 0 (correct) |
| Series README coverage | 15/15 folders have README (correct) |

**Overall:** Core specifications (Part I), implementation guides (Part XI), ADRs (Part XIII), and reference catalogs (Part VII) contain substantial, usable content aligned with v0.6.0-alpha implementation. Gaps remain in navigation standardization, empty placeholders, index status drift, and a few stale cross-references.

---

# What Is Already Correct (Facts)

- **Numbering:** No duplicate `EVE-XXXX` IDs; each `eve-XXXX-*.md` maps to one document.
- **Part I (0000):** EVE-0000 through EVE-0012 all exist with full content (13/13).
- **Part II (0100):** EVE-0100 through EVE-0103 exist with full content (4/4).
- **Part IV (0300):** EVE-0300 through EVE-0307 and EVE-0308/0309 exist with content (10/10 files; 0308/0309 reserved but written).
- **Part VII (0600):** EVE-0600 through EVE-0609 all exist with full content (10/10).
- **Part XI (1000):** EVE-1000 through EVE-1011 all exist with full content (12/12).
- **Part XIII (3000):** EVE-3001 through EVE-3009 all exist with full content (9/9).
- **EVE-0007** now documents Provider Formatter responsibilities (updated for v0.6).
- **EVE-0604** and **EVE-0607** catalogs updated with ✅ status for implemented capabilities/providers.
- **CHANGELOG.md** documents v0.1.0-alpha through v0.6.0-alpha release history.
- **Every series folder** has a `README.md`.

---

# Issues by Severity

## Critical

Issues that undermine EVE-SPEC as authoritative source of truth or block contributor navigation.

| # | Issue | Location | Fact / Evidence |
|---|-------|----------|-----------------|
| C1 | **Broken catalog navigation chain** | `docs/0600-reference-catalogs/eve-0603-rest-api-endpoint-catalog.md` | Next=`Reserved` while EVE-0604 exists and references EVE-0603 as Previous |
| C2 | **Incorrect Next document title breaks ID trust** | `docs/0000-core-platform-specifications/eve-0007-ai-provider-specification.md` | Next=`EVE-0008 — Service Layer Specification` but EVE-0003 is Service Layer; EVE-0008 is Persona and Communication |
| C3 | **16 empty placeholder files** listed in series READMEs as planned documents | `docs/0200/`, `docs/0400/`, `docs/0500/`, `docs/2000/` | Zero-byte files create false impression of coverage |
| C4 | **EVE-SPEC index status drift** | `docs/README.md` Parts table | Parts IV (0300), XI (1000), XII (2000), XIII (3000) marked ⏳ Planned despite active content |
| C5 | **ADR cross-reference ID errors** | `docs/3000-architecture-decision-records/eve-3002-capability-oriented-architecture.md`, `eve-3004-validation-pipeline.md` | Cite EVE-0002 as Platform Request and EVE-0003 as Platform Response (actual: Capability Spec, Service Layer) |

---

## High

| # | Issue | Location | Fact / Evidence |
|---|-------|----------|-----------------|
| H1 | **Stale Reserved Next links** | `eve-0011-platform-response-specification.md`, `eve-0012-validation-specification.md` | Next=`Reserved`; EVE-0012 exists; series should link forward |
| H2 | **Legacy Part field instead of Current** | EVE-0010, EVE-0011, EVE-0012; EVE-0600–0603, 0606, 0608, 0609 | 11 files omit `**Current**` document identity |
| H3 | **0300 CLI series uses non-standard navigation** | All `eve-0300`–`eve-0307` | `## Series Navigation` without standard `Return` block |
| H4 | **0300 README incorrect Next reference** | `docs/0300-cli-specifications/README.md` | Next=`Part II — EVE-2000 Implementation Guides` (Part II is Discord; 2000 is Part XII) |
| H5 | **EVE-0010 incorrect Next title** | `eve-0010-platform-request-specification.md` | Next=`Response Object Specification`; EVE-0011 title is Platform Response Specification |
| H6 | **Part I core specs lack navigation** | EVE-0000–0004, 0006, 0008, 0009 | 8 of 13 Part I documents have no Navigation section |
| H7 | **Discord specs lack navigation** | EVE-0100–0103 | All four Part II specs missing Navigation |
| H8 | **EVE-0001 architecture diagram outdated** | `docs/0000-core-platform-specifications/eve-0001-system-architecture.md` | Still shows Response Formatter; omits Search/Rank/Cite/Assemble/ProviderFormatter pipeline |
| H9 | **2000 Developer Guides entirely empty** | `docs/2000-developer-guides/eve-2000`–`eve-2009` | README lists guides as Planned; 10 zero-byte files exist |
| H10 | **Root README incorrect ADR path** | `README.md` | References `docs/adr/`; actual path is `docs/3000-architecture-decision-records/` |

---

## Medium

| # | Issue | Location | Fact / Evidence |
|---|-------|----------|-----------------|
| M1 | **Part I Return label inconsistency** | EVE-0005, EVE-0007 vs EVE-0010–0012 | Engineering Specifications vs Core Platform Specifications |
| M2 | **Filename vs title mismatch (EVE-0008)** | `eve-0008-personality-specification.md` | Title: Persona and Communication Specification |
| M3 | **Filename vs title mismatch (EVE-0103)** | `eve-0103-discord-embeds-and-formatting.md` | README: Discord Response and Formatting Specification |
| M4 | **EVE-0308/0309 missing navigation** | Reserved CLI specs with content but no nav | Breaks 0300 series continuity after EVE-0307 |
| M5 | **EVE-0307 Next skips 0308/0309** | `eve-0307-exit-code-specification.md` | Jumps to EVE-2000 series |
| M6 | **docs/README.md non-standard Navigation** | `docs/README.md` | Uses intro text instead of Previous/Current/Next/Return |
| M7 | **All series READMEs use Series Navigation** | 14 README files | Not aligned with latest five-field standard |
| M8 | **No normative Reasoning Pipeline specification** | Part I (0000) | Pipeline documented in guides/ADRs only; no EVE-0013+ spec |
| M9 | **No normative Provider Formatting specification** | Part I (0000) | Covered in EVE-0007 and guides; no dedicated spec ID |
| M10 | **0500 series misplacement** | `eve-0500-testing-specification.md` | Testing content in Platform Specifications folder; Part VIII (0700) is testing series |
| M11 | **0200 README lists EVE-0202/0203** | `docs/0200-website-specifications/README.md` | Files do not exist (only 0200, 0201 placeholders) |
| M12 | **0400 README lists EVE-0401–0403** | `docs/0400-rest-api-specifications/README.md` | Files do not exist |
| M13 | **0700–0900, 9000 READMEs list many planned IDs** | Multiple READMEs | Expected for planned work; creates reference surface without files |
| M14 | **Zero Markdown hyperlinks in docs/** | Entire corpus | Cross-refs are plain-text EVE-IDs only; no `[text](path.md)` links |
| M15 | **3000 README Next points to non-document** | `docs/3000-architecture-decision-records/README.md` | Next=`Ideas & Future Concepts` |

---

## Low

| # | Issue | Location | Fact / Evidence |
|---|-------|----------|-----------------|
| L1 | **Dual H1 headings** | Most EVE-XXXX documents | `# EVE-XXXX` followed by `# Title` |
| L2 | **Motto sections vary** | EVE-0010–0012, some 0600 catalogs | `## Motto` after Navigation; not universal |
| L3 | **0300 CLI footer metadata blocks** | EVE-0300–0307 | Extra `**Specification:**` footer after navigation |
| L4 | **Return path variant** | 0000/README, 9000/README | ``← EVE-SPEC (`/docs`)`` vs `← EVE-SPEC` |
| L5 | **Historical v0.5.x reference** | `eve-1000-implementation-roadmap.md` | Release table row for v0.5.x (historical, acceptable) |
| L6 | **EVE-3000 index document absent** | 3000 series | Series starts at EVE-3001 |
| L7 | **2000 README status** | `docs/2000-developer-guides/README.md` | All guides marked Planned despite file stubs existing |

---

## Suggestion

| # | Suggestion | Rationale |
|---|------------|-----------|
| S1 | Add markdown hyperlinks alongside EVE-ID plain-text references | Enables IDE navigation and automated link checking |
| S2 | Merge duplicate pipeline diagrams into one canonical spec (EVE-0001 or new EVE-0013) | Reduces maintenance across README, PROJECT_STATUS, guides, ADRs |
| S3 | Populate or delete empty placeholder files before v0.7.0 | Improves documentation honesty |
| S4 | Standardize Part I Return label to Core Platform Specifications | Single canonical Part name |
| S5 | Add EVE-0700 Testing Architecture specification | 172 tests exist; no Part VIII specs written |
| S6 | Write EVE-2004 Provider Development Guide first among 2000 series | Highest value for next engineering phase |
| S7 | Add AI-0000 Null Provider as cataloged entry in EVE-0607 | Currently mentioned in prose only |
| S8 | Rename `eve-0008-personality-specification.md` to match Persona title | Filename/title alignment |

---

# Category Detail

## Numbering Consistency

| Series | Range | Status |
|--------|-------|--------|
| 0000 | 0000–0012 | Complete, sequential, no gaps |
| 0100 | 0100–0103 | Complete |
| 0200 | 0200–0201 exist; 0202–0203 in README only | Gap |
| 0300 | 0300–0309 | Complete (0308–0309 reserved) |
| 0400 | 0400 only | 0401–0403 planned |
| 0500 | 0500–0502 placeholders | 0503–0507 planned |
| 0600 | 0600–0609 | Complete |
| 0700–0900 | README only | Entire series planned |
| 1000 | 1000–1011 | Complete |
| 2000 | 2000–2009 stubs | Files exist, empty |
| 3000 | 3001–3009 | Complete; no EVE-3000 index |
| 9000 | README only | Planned |

**Duplicate IDs:** None found.

---

## Terminology Consistency

| Term A | Term B | Locations | Severity |
|--------|--------|-----------|----------|
| Context Builder | Context Assembler | EVE-0005 vs EVE-1008/1009 | Medium — roles clarified in guides; EVE-0005 still broad |
| Persona | Personality | EVE-0008 title vs filename | Medium |
| Response Formatter | Platform Response + interface formatters | EVE-0001, EVE-0303 | High — Response Formatter not implemented |
| Prompt Builder (SVC-0203) | Provider Formatter | EVE-0605 catalog vs code | Low — catalog updated in CAP-0400 |
| Engineering Specifications | Core Platform Specifications | Part I Return lines | Medium |

---

## Contradictory Specifications

| Conflict | Documents | Current Resolution |
|----------|-----------|-------------------|
| EVE-0002/0003 ID meanings in ADRs | EVE-3002, EVE-3004 vs actual 0000 specs | ADRs wrong; specs correct |
| Response Formatter in architecture | EVE-0001 vs implementation | Not implemented; guides omit it |
| CHANGELOG claims navigation standardized | AUDIT-NAVIGATION (28% pass) | Aspirational / partial |

**Note:** EVE-0007 vs Provider Formatter conflict from prior releases appears **resolved** in current `eve-0007-ai-provider-specification.md`.

---

## Duplicate Content

| Content | Locations | Severity |
|---------|-----------|----------|
| Reasoning pipeline flow diagram | README.md, PROJECT_STATUS.md, ARCHITECTURE.md, EVE-1001, 1008, 1010, multiple ADRs | Suggestion |
| Provider Formatter responsibilities | EVE-0007, EVE-1010, EVE-3005, EVE-3009 | Low (consistent intent) |
| Engineering principles lists | EVE-0000, docs/README, AGENTS.md | Low |

---

## Missing README Files

**None.** All 14 series folders plus `docs/README.md` have README files.

---

## Missing Navigation / Summaries / Relationships

| Gap | Count | Severity |
|-----|-------|----------|
| Missing Navigation | 39 files | High |
| Missing Summary sections | Most specs include Summary or equivalent; empty files have none | Critical for placeholders |
| Missing relationship sections | 0200, 0400, 0500, 2000 empty files | Critical |

---

## Diagrams

| Document | Status |
|----------|--------|
| EVE-0001 | Outdated layered diagram (pre-pipeline-split) |
| EVE-1008, EVE-3009 | Current pipeline diagrams (correct) |
| ARCHITECTURE.md | Updated for v0.6.0-alpha (correct per PROJECT_STATUS) |
| Most interface specs | No runtime sequence diagrams |

---

## Markdown Issues

- No broken `.md` hrefs (none used).
- Tables in README index files: well-formed.
- Empty files: 16 (invalid as documents).
- Encoding: em dash (`—`) used consistently in navigation blocks.

---

# Repository Health Score (Documentation)

| Dimension | Score (0–100) |
|-----------|---------------|
| Content completeness | 78 |
| Navigation standardization | 28 |
| Cross-reference integrity | 72 |
| Index/README accuracy | 65 |
| Terminology consistency | 75 |
| Catalog accuracy | 88 |
| **Weighted overall** | **68** |

---

*End of Documentation Audit — v0.6.0-alpha*
