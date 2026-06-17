## Why

The `openspec/specs/` directory had grown to 19 capability-level spec files with no domain grouping. The [OpenSpec documentation](https://github.com/Fission-AI/OpenSpec/blob/main/docs/concepts.md#specs) shows specs organised **by domain** (`auth/spec.md`, `payments/spec.md`), not by individual capability. Without domain-level organisation, it is hard to navigate, requirements are scattered across many small files, and AI agents have no context about which requirements belong together. Without explicit decision rules, new requirements end up in inconsistent places.

## What Changes

- **Restructure `openspec/specs/` to domain-level spec files** — one `spec.md` per domain, following the OpenSpec-documented pattern. The 19 old capability-level specs are merged into 7 domain spec files:
  - `infrastructure/spec.md` (build system, devcontainer, mise tasks, R toolchain)
  - `data/spec.md` (patch catalog, glossary)
  - `game-core/spec.md` (game state types, game setup)
  - `game-logic/spec.md` (move generation, move application, terminal/scoring)
  - `engine/spec.md` (play driver, game logger)
  - `tui/spec.md` (display, input, launch, undo/redo)
  - `agents/spec.md` (random agent)
- **Add `openspec/specs/README.md`** — catalog with domain descriptions, decision rules, and naming convention.
- **Add TUI as an eighth domain** — the four TUI specs (`tui-display`, `tui-input`, `tui-launch`, `tui-undo-redo`) are a coherent interactive UI layer distinct from the engine, matching OpenSpec's `ui/` domain example.

## Capabilities

### New Capabilities

- Requirements about the spec catalog (index, taxonomy, decision rules, naming convention) are added to `infrastructure/spec.md` via the `organize-specs/specs/infrastructure/spec.md` delta.

### Modified Capabilities

- All 19 capability-level spec folders replaced by 7 domain-level spec folders.

## Impact

- The `openspec archive` CLI maps `changes/<change>/specs/<entry>/spec.md` to `openspec/specs/<entry>/spec.md`. Domain-level spec names (`infrastructure`, `data`, `game-core`, etc.) work identically — new changes simply target the domain spec folder instead of a capability folder.
- Active change `r-package-structure` has had its delta specs updated to target the new domain names.
- Every future change MUST target the appropriate domain spec and update `openspec/specs/README.md`.
