# EVE-2006

# Service Development Guide

> Contributor Guide for Adding and Maintaining Platform Services

---

## Document Information

| Property | Value |
|----------|-------|
| **Document ID** | EVE-2006 |
| **Series** | 2000 — Developer Guides |
| **Title** | Service Development Guide |
| **Project** | Evolutionary Virtual Engineer (E.V.E.) |
| **Specification** | EVE-SPEC |
| **Version** | 0.1.0 |
| **Status** | Active |
| **Audience** | Contributors |

---

# 1. Purpose / Scope

This guide is for contributors who add or maintain a
**platform service** in the Core Platform library.

In E.V.E., a service is a reusable, interface-independent
platform component that capabilities (and other subsystems)
consume. Services encapsulate shared operations such as
configuration, documentation access, status reporting, and
conversation memory.

A service typically:

- exposes an abstract `I*Service` interface
- provides a concrete implementation
- is constructed during platform bootstrap
- is injected into capability handlers (or other consumers)

Service development does **not** mean inventing Discord/CLI
command parsing, AI provider protocols, or capability
orchestration.

This document is **instructional**. Prefer authoritative
sources over copying their content here.

| Need | Read |
|------|------|
| What services must do | [EVE-0003](../0000-core-platform-specifications/eve-0003-service-layer-specification.md) |
| How the service layer is engineered | [EVE-1004](../1000-implementation-guides/eve-1004-service-layer-implementation-guide.md) |
| SVC-* registry | [eve-0605](../0600-reference-catalogs/eve-0605-service-catalog.md) |
| Capability-oriented rationale | [EVE-3002](../3000-architecture-decision-records/eve-3002-capability-oriented-architecture.md) |
| Capability contributor workflow | [EVE-2005](eve-2005-capability-development-guide.md) |
| Provider contributor workflow | [EVE-2004](eve-2004-provider-development-guide.md) |

---

# 2. Before You Code

Do this before writing C++:

1. Read **EVE-0003** (service principles and boundaries).
2. Read **EVE-1004** (DI, lifetime, composition guidance).
3. Check **eve-0605** for related SVC-* entries and status.
4. Search `include/eve/services/` and `src/eve/services/` for an
   existing interface/implementation.
5. Decide whether the work belongs in a service, a capability,
   or a provider (see Section 5).

**Do**

- Prefer extending an existing `I*Service` when the operation
  is reusable platform functionality.
- Keep services free of interface and provider protocol code.
- Follow bootstrap construction + injection patterns already
  in-tree.

**Don't**

- Invent a new SVC-* ID without catalog authorization.
- Assume every eve-0605 SVC-* row is an `eve::services`
  class (see Section 4).
- Treat a 🚧 catalog row as unimplemented without checking
  the code (catalog drift exists; report it).
- Smuggle CAP-0404, durable memory, TLS, or interface work
  into a “service” change without authorization.

Working examples in this repository:

| Interface | Implementation | Role |
|-----------|----------------|------|
| `IConfigurationService` | `ConfigurationService` | Runtime configuration |
| `ILoggingService` | `LoggingService` | Logging |
| `IDocumentationService` | `DocumentationService` | Documentation lookup |
| `ISearchService` | `SearchService` | Knowledge search access |
| `IStatusService` | `StatusService` | Platform status reporting |
| `IDiagnosticsService` | `DiagnosticsService` | Diagnostics collection |
| `IConversationMemoryService` | `ConversationMemoryService` | Session memory |

---

# 3. Service Architecture Overview

Contributor view of how services participate today:

```text
platform_bootstrap.cpp
        ↓
construct concrete services (shared_ptr / values as needed)
        ↓
inject into CapabilityRegistry handler registration
        ↓
ICapabilityHandler::execute(PlatformRequest)
        ↓
calls I*Service methods
        ↓
PlatformResponse
```

There is **no** separate service-registry class in the current
tree. Construction and wiring happen in
`src/eve/core/platform_bootstrap.cpp`, then handlers receive
services through registration helpers.

Details live elsewhere:

- Service principles → [EVE-0003](../0000-core-platform-specifications/eve-0003-service-layer-specification.md)
- Engineered DI patterns → [EVE-1004](../1000-implementation-guides/eve-1004-service-layer-implementation-guide.md)
- Capability dispatch → [EVE-2005](eve-2005-capability-development-guide.md) / [EVE-1003](../1000-implementation-guides/eve-1003-capability-engine-implementation-guide.md)

---

# 4. Finding the Existing Service Surface

| Surface | Path |
|---------|------|
| Service interfaces | `include/eve/services/service_interfaces.hpp` |
| Service implementations | `include/eve/services/service_implementations.hpp` |
| Implementation sources | `src/eve/services/*.cpp` |
| Bootstrap construction | `src/eve/core/platform_bootstrap.cpp` |
| Handler injection helpers | `include/eve/capability/handlers/capability_handlers.hpp` |
| Catalog | [eve-0605](../0600-reference-catalogs/eve-0605-service-catalog.md) |

## Critical catalog distinction

**`eve::services`** is the narrower reusable C++ platform-service
surface contributors extend (`I*Service` + concrete classes).

**eve-0605** is a broader SVC-* reference inventory. It also
lists dispatchers, engines, providers, and other platform
components that are **not** classes under `eve::services`.

Do **not** imply that every SVC-* catalog entry is an
`eve::services` implementation.

---

# 5. Service vs Capability vs Provider Decision

| Layer | Owns | Example |
|-------|------|---------|
| Capability | User-intent ID + orchestration | `CAP-0102` Documentation Search |
| Service | Reusable platform operations | `IConversationMemoryService` |
| Provider | Language generation adapter | `OllamaProvider` / `LMStudioProvider` |

**Choose a service when**

- multiple capabilities (or subsystems) need the same operation
- the logic is infrastructure / persistence / access / status
- the unit should be independently testable without a
  `CapabilityId`

**Choose a capability when**

- the work is a user-intent entry point with a catalog
  `CapabilityId`
- orchestration across services/pipelines is required

**Choose a provider when**

- the work is backend-specific language generation /
  protocol adaptation

Evidence: CAP-0102 orchestrates search → context → provider
generate, and uses `IConversationMemoryService` for load/append.
Memory storage is not embedded inside the capability handler or
inside providers.

---

# 6. Service Development Workflow

Typical contributor sequence:

1. Confirm the operation belongs in the service layer.
2. Check for an existing `I*Service` / implementation.
3. Check eve-0605 for related SVC-* identity/status (and note
   catalog breadth).
4. Add or extend the interface in `service_interfaces.hpp`.
5. Add or extend the concrete class in
   `service_implementations.hpp` + `src/eve/services/`.
6. Construct the service in `platform_bootstrap.cpp`.
7. Inject it into the consumers that need it (usually capability
   registration helpers).
8. Add/update focused tests.
9. Update authorized catalog/status docs.

Prefer the smallest change that preserves existing interfaces.

---

# 7. Designing a Service

Before coding, answer:

- Is this reusable beyond one capability?
- Does an existing service already cover it?
- Should this be configuration-driven?
- What ownership model fits (`shared_ptr` vs reference)?
- Does the service need knowledge/provider dependencies?
- Will adding it force interface or provider protocol code into
  `eve::services`? (If yes, stop — wrong layer.)

Keep services deterministic and independently testable where
practical.

---

# 8. Implementing Interface + Concrete Service

Practical checklist:

## Interface

- [ ] Declare methods on an `I*Service` in
      `service_interfaces.hpp`
- [ ] Keep the interface free of Discord/CLI/REST/Website types
- [ ] Prefer clear, narrow operations over kitchen-sink APIs

## Implementation

- [ ] Implement the concrete class in
      `service_implementations.hpp` / `src/eve/services/`
- [ ] Depend on other platform components through constructors
      (same style as `StatusService`, `SearchService`, etc.)
- [ ] Propagate errors with existing `std::expected` /
      platform error patterns where used
- [ ] Do not parse provider HTTP/SSE/NDJSON here

## Build

- [ ] Ensure the `.cpp` is listed in `eve_core` CMake sources

---

# 9. Bootstrap / Dependency Injection

Current pattern (do not invent a registry):

1. Construct services in `platform_bootstrap.cpp`.
2. Pass them into `register_core_handlers` /
   `register_documentation_handlers` (or an authorized equivalent).
3. Handlers store references/`shared_ptr` members as already done.
4. Capability execution calls service methods during `execute`.

Examples already in bootstrap:

- `ConfigurationService` from config path
- `DocumentationService` / `SearchService` / `DiagnosticsService`
  over knowledge store / reasoning pipeline
- `StatusService` observing configuration, knowledge, and
  `ProviderManager`
- `ConversationMemoryService` from configuration retention limits

There is no global service singleton assumption in the Core
Platform design. Prefer explicit injection.

---

# 10. Service Lifetime and Ownership

Follow existing ownership:

- Bootstrap-owned `shared_ptr` services live for platform lifetime.
- Handlers that need shared ownership take `shared_ptr`
  (for example conversation memory).
- Handlers that only need non-owning access take `const T&`
  (for example configuration / status / diagnostics in core
  handlers).

Do not introduce a new lifetime framework in a service change.
Match the surrounding code.

---

# 11. Testing Workflow

Canonical build/test entry points are in the root
[README.md](../../README.md).

For service work, contributors typically:

1. Build `eve_core` and `eve_tests`.
2. Run focused service / configuration / memory / integration
   tests.
3. Run the full non-live suite:

```text
eve_tests --gtest_filter=-LiveOllama*
```

Current known non-live baseline (do not “fix” as part of a
service guide change unless separately authorized):

| Result | Count |
|--------|-------|
| Total | 278 |
| PASS | 275 |
| FAIL | 3 |

The three failures are known `RepositoryDiscoveryTest` cases.

## Existing patterns to copy

| Pattern | Example |
|---------|---------|
| Direct service unit tests | `tests/conversation_memory_test.cpp` |
| Configuration fixtures | `tests/configuration_memory_test.cpp` |
| Capability integration with services | `tests/integration_test.cpp` |
| Provider tests needing config | LM Studio / provider formatter tests constructing `ConfigurationService` |

## What to cover when adding/changing a service

- [ ] Construction defaults / configuration wiring
- [ ] Successful operation paths
- [ ] Failure / empty-input behavior
- [ ] Isolation between sessions/resources when relevant
- [ ] Handler integration if capabilities consume the service
- [ ] Side-effect rules (for example memory append only on
      successful generation remains a capability concern)

Prefer deterministic fixtures. No sleeps. No live network in
the default suite.

---

# 12. Documentation / Catalog Updates

Before calling a service change “done,” verify only what changed:

- [ ] **eve-0605** — update or add SVC-* entries when authorized;
      keep catalog breadth vs `eve::services` clear
- [ ] **EVE-0003** — only if the normative service contract changed
- [ ] **EVE-1004** — only if engineered guidance changed
- [ ] **PROJECT_STATUS / CHANGELOG** — when authorized

Do not blindly edit every document for every service tweak.

## Catalog drift (report; do not silently rewrite)

Step 21 identified existing drift examples, including:

- some implemented services still marked 🚧 in eve-0605
- `ConversationMemoryService` not represented as an SVC-* entry

Contributors should **identify and report** such drift for an
authorized catalog synchronization step. Do **not** silently
rewrite eve-0605 while implementing unrelated service work.

---

# 13. Common Mistakes / Anti-Patterns

- Putting provider protocol parsing (HTTP/SSE/NDJSON) in a service
- Writing conversation memory from a provider
- Embedding user-intent orchestration (`CapabilityId` flow) inside
  a service
- Adding Discord/CLI/REST types under `eve/services`
- Assuming every SVC-* catalog row maps 1:1 to `eve::services`
- Creating a new global service singleton
- Inventing a service registry that the platform does not use
- Marking catalog status ✅/🚧 without checking the code
- Expanding durable memory / CAP-0404 under a service PR without
  authorization

---

# 14. V0.7.x Constraints

Honest limits for contributors after v0.7.0-alpha:

- Core Platform remains library-oriented; interface binaries are
  **v0.8.x**
- Provider-layer streaming exists; interface streaming remains
  deferred
- CAP-0404 / durable memory / streaming memory remain deferred
- OpenAI / Anthropic / AI-0200 / TLS are not prerequisites for
  service-guide work
- EVE-2000–EVE-2003 remain Planned until interfaces exist

---

# 15. Contributor Completion Checklist

- [ ] Confirmed service vs capability vs provider decision
- [ ] EVE-0003 / EVE-1004 / eve-0605 reviewed
- [ ] `I*Service` + concrete implementation added/updated
- [ ] Bootstrap construction + consumer injection wired
- [ ] No provider-protocol or interface parsing in the service
- [ ] Focused tests + non-live suite green aside from the three
      known `RepositoryDiscoveryTest` failures
- [ ] Catalog/status/changelog updated when authorized
- [ ] Catalog drift reported rather than silently rewritten

---

# 16. Authoritative Cross-References

- [EVE-0003](../0000-core-platform-specifications/eve-0003-service-layer-specification.md)
- [EVE-1004](../1000-implementation-guides/eve-1004-service-layer-implementation-guide.md)
- [eve-0605](../0600-reference-catalogs/eve-0605-service-catalog.md)
- [EVE-3002](../3000-architecture-decision-records/eve-3002-capability-oriented-architecture.md)
- [EVE-2005](eve-2005-capability-development-guide.md)
- [EVE-2004](eve-2004-provider-development-guide.md)
- [EVE-1003](../1000-implementation-guides/eve-1003-capability-engine-implementation-guide.md)
- [Part XII README](README.md)

---

## Navigation

**Previous**

[EVE-2005 — Capability Development Guide](eve-2005-capability-development-guide.md)

**Current**

EVE-2006 — Service Development Guide

**Next**

EVE-2008 — Testing Guide (planned)

**Return**

← [Part XII — Developer Guides](README.md)

← EVE-SPEC

---

*"Knowledge should evolve."*
