# EVE-2005

# Capability Development Guide

> Contributor Guide for Adding and Maintaining Platform Capabilities

---

## Document Information

| Property | Value |
|----------|-------|
| **Document ID** | EVE-2005 |
| **Series** | 2000 — Developer Guides |
| **Title** | Capability Development Guide |
| **Project** | Evolutionary Virtual Engineer (E.V.E.) |
| **Specification** | EVE-SPEC |
| **Version** | 0.1.0 |
| **Status** | Active |
| **Audience** | Contributors |

---

# 1. Purpose / Scope

This guide is for contributors who add or maintain a
**platform capability** in the Core Platform library.

In E.V.E., a capability is a platform-independent unit of
user intent. Interfaces (Discord, CLI, REST, Website) expose
capabilities; they do not redefine them.

A capability handler:

- receives a validated `PlatformRequest`
- executes capability-specific business logic
- returns a `PlatformResponse` (or `CapabilityError`)

Capability development does **not** mean inventing interface
command parsing, AI provider protocols, or replacing the
Validation Engine / Command Dispatcher.

This document is **instructional**. Prefer authoritative
sources over copying their content here.

| Need | Read |
|------|------|
| What capabilities must do | [EVE-0002](../0000-core-platform-specifications/eve-0002-capability-specification.md) |
| How the Capability Engine is engineered | [EVE-1003](../1000-implementation-guides/eve-1003-capability-engine-implementation-guide.md) |
| Which capability IDs exist | [eve-0604](../0600-reference-catalogs/eve-0604-capability-catalog.md) |
| Which services exist | [eve-0605](../0600-reference-catalogs/eve-0605-service-catalog.md) |
| Provider contributor workflow | [EVE-2004](eve-2004-provider-development-guide.md) |

---

# 2. Before You Code

Do this before writing C++:

1. Read **EVE-0002** (capability contract and categories).
2. Read **EVE-1003** (registry, dispatch, handler rules).
3. Check **eve-0604** for an existing catalog ID and status.
4. Search the tree for an existing handler for that ID.
5. Confirm whether you are implementing a cataloged capability
   or proposing a new catalog entry (requires EM/catalog
   authorization).

**Do**

- Use a catalog `CapabilityId` already assigned (for example
  `CAP-0102`).
- Follow existing handler patterns in-tree.
- Keep handlers interface-agnostic.

**Don't**

- Invent a new `CapabilityId` without catalog authorization.
- Put Discord/CLI/REST parsing inside a handler.
- Treat planned catalog rows (🚧) as implemented merely
  because they appear in eve-0604.
- Implement **CAP-0404** Conversation Memory commands in this
  V0.7.x guide wave unless separately authorized (still deferred).

Working examples in this repository:

| Handler area | Example IDs | Location |
|--------------|-------------|----------|
| Core handlers | `CAP-0001` … `CAP-0005` | `src/eve/capability/handlers/core_handlers.cpp` |
| Documentation search | `CAP-0102` | `src/eve/capability/handlers/documentation_handlers.cpp` |
| Status (registered) | `CAP-0002-STATUS` | same documentation handlers file |
| Aliases | `search` → `CAP-0102`, `status` → `CAP-0002-STATUS` | platform bootstrap |

---

# 3. Capability Architecture Overview

High-level request path (contributor view):

```text
Interface / caller
        ↓
PlatformRequest (capability id + parameters)
        ↓
Validation Engine
        ↓
Command Dispatcher
        ↓
CapabilityEngine
        ↓
ICapabilityHandler::execute()
        ↓
services / reasoning / providers (as needed)
        ↓
PlatformResponse
```

Details live elsewhere — do not re-implement them in the
handler:

- Platform Request / Response → EVE-0010 / EVE-0011
- Validation → EVE-0012 / EVE-1002
- Capability Engine engineering → [EVE-1003](../1000-implementation-guides/eve-1003-capability-engine-implementation-guide.md)
- Core platform flow → [EVE-1001](../1000-implementation-guides/eve-1001-core-platform-implementation-guide.md)

Handlers execute **after** validation. They should not re-parse
or re-validate the request contract.

---

# 4. Finding the Existing Capability Surface

| Surface | Path / document |
|---------|-----------------|
| Handler interface | `include/eve/capability/capability_engine.hpp` (`ICapabilityHandler`) |
| Registry / engine | same header + `src/eve/capability/capability_engine.cpp` |
| Registration helpers | `include/eve/capability/handlers/capability_handlers.hpp` |
| Core handlers | `src/eve/capability/handlers/core_handlers.cpp` |
| Documentation handlers | `src/eve/capability/handlers/documentation_handlers.cpp` |
| Bootstrap registration | `src/eve/core/platform_bootstrap.cpp` |
| Catalog | [eve-0604](../0600-reference-catalogs/eve-0604-capability-catalog.md) |
| Engine tests | `tests/capability_engine_test.cpp` |
| End-to-end CAP-0102 | `tests/integration_test.cpp` |

`ICapabilityHandler` today requires:

- `id()` → `CapabilityId`
- `name()` → human-readable string
- `execute(const PlatformRequest&)` →
  `std::expected<PlatformResponse, CapabilityError>`

---

# 5. Capability Development Workflow

Typical contributor sequence:

1. **Identify** the catalog capability (or get a new ID approved).
2. **Read** EVE-0002 / EVE-1003 / existing similar handler.
3. **Decide** whether this is a new handler, an extension of an
   existing handler, or only a documentation/catalog fix.
4. **Implement** the handler (or update the existing one).
5. **Register** it during bootstrap (via the appropriate
   `register_*_handlers` helper or an authorized equivalent).
6. **Add/update tests**.
7. **Update** eve-0604 status / notes when the capability becomes
   real.
8. **Update** PROJECT_STATUS / CHANGELOG only when authorized.

Prefer the smallest change that satisfies the catalog intent.

---

# 6. Designing a Capability Change

Before coding, answer:

- Is there already a catalog ID for this intent?
- Is the catalog status ✅, 🚧, or reserved?
- Does an existing handler already own related behavior
  (for example CAP-0102 already owns documentation search +
  memory attach/persist on that path)?
- Which services does the handler need
  ([eve-0605](../0600-reference-catalogs/eve-0605-service-catalog.md))?
- Does the work belong in a **service** instead of a capability
  (reusable platform functionality without a user-intent ID)?
- Does the work belong in a **provider** instead
  ([EVE-2004](eve-2004-provider-development-guide.md))?

**Capability vs service vs provider**

| Layer | Owns |
|-------|------|
| Capability | User-intent execution behind a `CapabilityId` |
| Service | Reusable platform operations used by handlers |
| Provider | Language generation only |

Do not collapse these layers.

---

# 7. Implementing the Change

Practical checklist aligned with the current repository layout:

## Code

- [ ] Implement `ICapabilityHandler` (or extend an existing
      handler class carefully)
- [ ] Return the catalog `CapabilityId` from `id()`
- [ ] Read parameters only from the validated `PlatformRequest`
- [ ] Call services / pipelines through injected dependencies
- [ ] Map success to `PlatformResponse`
- [ ] Map failures to `CapabilityError` (or structured platform
      failure responses consistent with existing handlers)
- [ ] Keep the handler free of Discord/CLI/REST/Website types

## Registration

- [ ] Register through `CapabilityRegistry::register_handler`
- [ ] Add aliases only when intentional
      (`register_alias`, as bootstrap does for `search` / `status`)
- [ ] Wire construction in platform bootstrap / register helpers
- [ ] Avoid duplicate registration of the same ID

## Build

- [ ] Ensure sources are part of `eve_core` / tests CMake as needed

---

# 8. Handler / Integration Guidance

Follow patterns already present:

**Core handlers** (`register_core_handlers`)

- Thin handlers for foundational platform capabilities
  (`CAP-0001` … `CAP-0005`)
- Depend on configuration / status / diagnostics / context
  builder as already wired

**Documentation handlers** (`register_documentation_handlers`)

- `CAP-0102` Documentation Search is the richest example:
  search → rank → cite → assemble context → optional memory
  load → provider generate → optional memory append → response
- Shows correct use of `ReasoningPipeline`,
  `ProviderManager`, and `IConversationMemoryService`
- Demonstrates: **do not persist memory on provider failure**

When extending CAP-0102 behavior, preserve those boundaries.
Do not move provider protocol parsing into the capability
handler (that belongs in providers; see EVE-2004).

---

# 9. Validation and Testing

Canonical build/test entry points are in the root
[README.md](../../README.md).

For capability work, contributors typically:

1. Build `eve_core` and `eve_tests`.
2. Run focused capability / integration tests.
3. Run the full non-live suite:

```text
eve_tests --gtest_filter=-LiveOllama*
```

Current known non-live baseline (do not "fix" as part of a
capability guide change unless separately authorized):

| Result | Count |
|--------|-------|
| Total | 278 |
| PASS | 275 |
| FAIL | 3 |

The three failures are known `RepositoryDiscoveryTest` cases.

## What to cover in capability tests

- [ ] Handler registration / resolution
- [ ] Successful `execute` path → `PlatformResponse`
- [ ] Missing/invalid parameters (as enforced by validation +
      handler expectations)
- [ ] Alias resolution when aliases are part of the change
- [ ] Failure paths that must not produce side effects
      (see CAP-0102 memory non-persist tests in
      `tests/integration_test.cpp`)
- [ ] Engine dispatch via `CapabilityEngine` /
      `tests/capability_engine_test.cpp` pattern

Prefer deterministic fixtures. No sleeps. No live network in
the default suite.

---

# 10. Documentation and Catalog Updates

Before calling a capability change “done,” verify:

- [ ] **eve-0604** — ID, status, and description match reality
- [ ] **EVE-0002 / EVE-1003** — update only when behavior or
      engineered guidance actually changed
- [ ] **PROJECT_STATUS / CHANGELOG** — when authorized
- [ ] This guide (EVE-2005) — only if contributor workflow changed

Avoid parallel write-ups that restate EVE-0002 or EVE-1003.
EVE-2005 is the contributor workflow entry point.

---

# 11. Common Mistakes / Anti-Patterns

- Inventing capability IDs not present in eve-0604
- Marking a 🚧 catalog row ✅ without an implementation
- Putting interface parsing or UX formatting into handlers
- Calling providers without going through `ProviderManager`
- Writing conversation memory from a provider (forbidden) or
  on failed generation (CAP-0102 must not)
- Duplicating search/rank/cite logic instead of using the
  reasoning pipeline
- Adding Discord/CLI/REST code under `src/eve/capability/`
- Implementing CAP-0404 “because memory exists” without
  authorization
- Assuming CAP-0403 Streaming Responses is a capability
  delivery surface today (provider-layer streaming exists;
  interface streaming remains deferred)

---

# 12. Relationship to Services

Capabilities **use** services; they should not re-implement
service responsibilities.

Examples already in-tree:

- Configuration / status / diagnostics services
- Documentation / search services
- Conversation memory service (consumed by CAP-0102)

For contributor workflow on services themselves, see
[EVE-2006](eve-2006-service-development-guide.md)
and the authoritative
[EVE-1004](../1000-implementation-guides/eve-1004-service-layer-implementation-guide.md) /
[eve-0605](../0600-reference-catalogs/eve-0605-service-catalog.md).

---

# 13. Relationship to Providers / AI Layer

Capabilities may invoke language generation through
`ProviderManager` after context is assembled.

Providers:

- do not own capability dispatch
- do not write conversation memory
- do not replace CAP-0102

See [EVE-2004](eve-2004-provider-development-guide.md) and
[EVE-1010](../1000-implementation-guides/eve-1010-ai-provider-implementation-guide.md).

CAP-0102 currently uses synchronous
`ProviderManager::generate`. Provider-layer streaming exists,
but capability/interface streaming delivery remains deferred.

---

# 14. V0.7.x Constraints and Deferred Areas

Honest limits for contributors working after v0.7.0-alpha:

- Core Platform is library-oriented; interface binaries are
  **v0.8.x**
- CAP-0404 explicit memory-management capabilities remain
  deferred
- Durable memory and streaming-specific memory persistence
  remain deferred
- OpenAI / Anthropic / AI-0200 / TLS are not prerequisites for
  capability guide work
- EVE-2000–EVE-2003 interface guides remain Planned until
  interfaces exist

---

# 15. Contributor Completion Checklist

- [ ] Catalog ID confirmed; no invented `CapabilityId`
- [ ] EVE-0002 / EVE-1003 / eve-0604 reviewed
- [ ] Handler implements `ICapabilityHandler` correctly
- [ ] Registration + aliases wired through bootstrap helpers
- [ ] Services used instead of duplicated platform logic
- [ ] Providers used only through `ProviderManager` when needed
- [ ] Focused tests + non-live suite green aside from the three
      known `RepositoryDiscoveryTest` failures
- [ ] Catalog / status / changelog updated when authorized
- [ ] No interface, TLS, CAP-0404, or provider-protocol work
      smuggled into the capability change

---

# 16. Authoritative Cross-References

- [EVE-0002](../0000-core-platform-specifications/eve-0002-capability-specification.md)
- [EVE-1003](../1000-implementation-guides/eve-1003-capability-engine-implementation-guide.md)
- [EVE-1001](../1000-implementation-guides/eve-1001-core-platform-implementation-guide.md)
- [EVE-1004](../1000-implementation-guides/eve-1004-service-layer-implementation-guide.md)
- [eve-0604](../0600-reference-catalogs/eve-0604-capability-catalog.md)
- [eve-0605](../0600-reference-catalogs/eve-0605-service-catalog.md)
- [EVE-2004](eve-2004-provider-development-guide.md)
- [Part XII README](README.md)

---

## Navigation

**Previous**

[EVE-2004 — Provider Development Guide](eve-2004-provider-development-guide.md)

**Current**

EVE-2005 — Capability Development Guide

**Next**

EVE-2006 — Service Development Guide

**Return**

← [Part XII — Developer Guides](README.md)

← EVE-SPEC

---

*"Knowledge should evolve."*
