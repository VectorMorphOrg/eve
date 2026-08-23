# EVE-3006

# Repository as Source of Truth

> Architecture Decision Record

---

## Document Information

| Property | Value |
|----------|-------|
| **Document ID** | EVE-3006 |
| **Series** | 3000 — Architecture Decision Records |
| **Title** | Repository as Source of Truth |
| **Status** | Accepted |
| **Version** | 2.0.0 |
| **Published** | July 2026 |
| **Last Updated** | July 2026 |

---

# Decision

The Evolutionary Virtual Engineer (E.V.E.) adopts the
software repository as the authoritative source of all
engineering knowledge.

Engineering knowledge shall always be derived from
version-controlled repository contents.

The Knowledge Engine constructs Knowledge Objects,
relationships, indexes, and Context Packages from repository
resources.

No external database, AI memory, or manually maintained
knowledge store shall become authoritative over repository
contents.

---

# Context

Many engineering assistants maintain independent knowledge
stores separate from the software repository.

These systems commonly experience:

- Stale documentation
- Out-of-date indexes
- Synchronization failures
- Duplicate sources of truth
- Version inconsistencies
- Manual maintenance

E.V.E. is intended to answer engineering questions exactly
as the repository exists at a specific revision.

The repository therefore becomes the authoritative
engineering knowledge source.

---

# Decision Drivers

Repository as Source of Truth supports:

- Deterministic knowledge
- Version consistency
- Automatic synchronization
- Git-native workflows
- Strong engineering provenance
- Reduced maintenance
- Reproducible reasoning

---

# Architecture

Engineering knowledge is derived directly from repository
contents.

```text
Repository

↓

Repository Discovery

↓

Document Parser

↓

Metadata Extraction

↓

Knowledge Object Builder

↓

Relationship Builder

↓

Knowledge Graph

↓

Knowledge Index

↓

Reasoning Pipeline
```

Knowledge is regenerated from repository contents whenever
necessary.

---

# Repository Discovery

Repository Discovery identifies engineering artifacts
eligible for indexing.

Typical resources include:

- Engineering Specifications
- Reference Catalogs
- Implementation Guides
- Developer Guides
- Architecture Decision Records
- Source Code
- Tests
- Configuration
- Build Files
- Project Assets

Discovery is deterministic.

---

# Knowledge Construction

Every discovered engineering resource becomes one or more
Knowledge Objects.

Knowledge Objects preserve:

- Document identity
- Metadata
- Sections
- Structure
- Relationships
- Search terms
- Repository provenance

Every Knowledge Object remains traceable to its originating
repository resource.

---

# Repository Provenance

Repository provenance is preserved throughout the
engineering pipeline.

Typical provenance includes:

- Repository identifier
- Repository path
- Document identifier
- Source file
- Repository revision
- Version information

Engineering responses remain traceable to documented
repository artifacts.

---

# Version Control Integration

Because the repository is authoritative:

- Git history becomes engineering history.
- Documentation evolves with implementation.
- Branches naturally produce independent engineering
  knowledge.
- Historical revisions remain reproducible.

No synchronization between documentation and external
knowledge stores is required.

---

# Derived Knowledge

Knowledge Graphs, Knowledge Indexes, and cached Knowledge
Objects are derived artifacts.

Derived artifacts improve performance but never become
authoritative.

If they are deleted or corrupted, they can always be rebuilt
from repository contents.

---

# Separation of Responsibilities

The repository is responsible for storing engineering
artifacts.

The Knowledge Engine is responsible for:

- Discovery
- Parsing
- Metadata extraction
- Relationship analysis
- Knowledge Object construction
- Knowledge Graph construction
- Knowledge Index construction

The Reasoning Pipeline consumes the Knowledge Engine.

AI Providers consume Provider Requests.

Each subsystem has one clear responsibility.

---

# Multi-Repository Support

Future versions of E.V.E. may support multiple repositories.

Each repository remains authoritative for its own
engineering artifacts.

Knowledge Graphs may span repositories while preserving
repository ownership and provenance.

---

# Benefits

Repository as Source of Truth provides:

- Deterministic engineering knowledge
- Automatic synchronization
- Strong provenance
- Git-native workflows
- Version-aware reasoning
- Reduced maintenance
- Simplified contributor onboarding

---

# Trade-Offs

Advantages include:

- No duplicated knowledge stores
- Reliable provenance
- Simpler architecture
- Easier verification
- Consistent documentation

Costs include:

- Repository indexing
- Incremental rebuilds after repository changes
- Larger repositories require additional processing

These costs are acceptable because repository contents
remain authoritative.

---

# Consequences

Every engineering answer produced by E.V.E. ultimately
originates from repository contents.

Changing repository contents naturally changes the
engineering knowledge available to the platform.

No hidden engineering knowledge exists outside the
repository.

The Knowledge Engine remains a deterministic projection of
repository state.

---

# Alternatives Considered

## External Knowledge Database

Maintain an independent engineering database.

Rejected because synchronization becomes difficult and the
database may diverge from repository contents.

---

## AI Memory as Source of Truth

Allow language models to remember engineering knowledge.

Rejected because AI memory is incomplete, non-deterministic,
and cannot guarantee engineering provenance.

---

## Manual Knowledge Maintenance

Maintain engineering indexes manually.

Rejected because duplicated maintenance does not scale and
introduces unnecessary engineering risk.

---

# Scope

This Architecture Decision applies to:

- Repository Discovery
- Knowledge Engine
- Knowledge Objects
- Knowledge Graph
- Knowledge Index
- Search Engine
- Citation Engine
- Context Builder
- Reasoning Pipeline
- Future repository federation

---

# Relationship to EVE-SPEC

This Architecture Decision supports:

- EVE-0004 — Knowledge Layer Specification
- EVE-0005 — Context Builder Specification
- EVE-0006 — Context Package Specification
- EVE-0009 — Documentation Integration Specification

Implementation guidance is provided by:

- EVE-1005 — Knowledge Objects Implementation Guide
- EVE-1006 — Knowledge Graph Implementation Guide
- EVE-1007 — Knowledge Index Implementation Guide
- EVE-1011 — Knowledge Engine Implementation Guide

---

# Related Architecture Decisions

- EVE-3001 — Documentation First Development
- EVE-3003 — Knowledge-First AI Architecture
- EVE-3005 — AI Provider Abstraction
- EVE-3007 — Knowledge Graph Architecture
- EVE-3009 — Provider-Independent Reasoning

---

# Review

This Architecture Decision should be reviewed whenever the
Repository Discovery process, Knowledge Engine
architecture, or multi-repository architecture undergoes
significant changes.

---

# Summary

Repository as Source of Truth establishes the
version-controlled repository as the authoritative
engineering knowledge source for E.V.E.

The Knowledge Engine deterministically derives Knowledge
Objects, relationships, indexes, and Context Packages from
repository contents while preserving complete engineering
provenance.

This architecture eliminates duplicate knowledge stores,
maintains deterministic engineering behavior, and ensures
every engineering response can be traced directly back to
its originating repository artifact.

---

## Navigation

**Previous**

EVE-3005 — AI Provider Abstraction

**Current**

EVE-3006 — Repository as Source of Truth

**Next**

EVE-3007 — Knowledge Graph Architecture

**Return**

← Part XIII — Architecture Decision Records

← EVE-SPEC

---

*"Knowledge should evolve."*