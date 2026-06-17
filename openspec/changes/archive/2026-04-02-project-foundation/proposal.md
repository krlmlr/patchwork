## Why

This is a greenfield hobby project using Uwe Rosenberg's Patchwork as a case study for learning game engine development, modern C++ idioms, and AI techniques up to reinforcement learning. A solid foundation — build system, canonical data model, and core state types — must be established before any game logic or AI work can begin.

## What Changes

- Introduce a **Meson** build system with `wraps` for dependency management
- Add **Catch2** (via Meson wrap) as the test framework, present from day one
- Create a **YAML patch catalog** as the single source of truth for all 33 Patchwork patches, encoding shapes as ASCII art grids
- Add an **R codegen script** that reads the catalog and emits a committed C++ header with patch definitions
- Define **core game state types** as dense 128-bit-friendly structs: per-player state (quilt board + position + buttons + income) and shared game state (patch availability + circle marker + bonus flags)
- Establish **project directory structure**: `cpp/`, `tests/`, `data/`, `codegen/`, `logs/`

## Capabilities

### New Capabilities

- `build-system`: Meson project setup with Catch2 wrap, directory layout, and baseline passing test
- `patch-catalog`: YAML catalog of all 33 patches (id, buttons, time cost, income, ASCII art shape) with R codegen producing a committed C++ header
- `game-state`: Core C++ types for PlayerState and GameState using dense bit-packing, with unit tests

### Modified Capabilities

## Impact

- Creates the entire project from scratch (empty repo)
- R is a dev-time dependency for codegen only (not a build dependency)
- Generated C++ header is committed to Git; R script is the source of truth mechanism
- No external C++ dependencies beyond Catch2 in this change
