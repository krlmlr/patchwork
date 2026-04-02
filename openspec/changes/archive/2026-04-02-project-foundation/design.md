## Context

Greenfield project. The repository is empty. All structural decisions are being made here for the first time. The project will eventually support game logic, search algorithms (minimax, MCTS), and reinforcement learning — so early decisions about state representation and build hygiene have long-term consequences.

The target audience is a single developer learning modern C++ and algorithmic techniques. Human-readability of generated code is valued alongside performance.

## Goals / Non-Goals

**Goals:**

- Establish a Meson build system that scales to the full project lifecycle
- Provide a single canonical data source for Patchwork patch definitions
- Define dense, cache-friendly game state types suitable for MCTS/RL workloads
- Ensure tests are present and passing from the first commit

**Non-Goals:**

- Game rules or move logic (next change)
- AI agents of any kind
- GUI or rich TUI
- Rust support (deferred)

## Decisions

### Build system: Meson + wraps (not CMake + vcpkg)

vcpkg is designed for CMake and pairs awkwardly with other build systems. Meson's native wrap system pulls source packages and builds them as subprojects — self-contained, no external package manager installation required. All key dependencies (Catch2, nlohmann/json, fmtlib) have published wraps.

**Alternative considered:** CMake + vcpkg — rejected because vcpkg requires separate installation and the CMake DSL is verbose for a clean greenfield project.

### Test framework: Catch2 v3 via Meson wrap

Catch2 v3 supports CMake and Meson natively, has a clean BDD-style macro syntax, and integrates with `meson test`. It will be present from the first commit so there is never a period of untested code.

**Alternative considered:** doctest — lighter weight but less ecosystem support and fewer matcher features needed later.

### Patch catalog: YAML with ASCII art shapes

YAML is human-editable, diff-friendly, and natively readable by R (`yaml` package). ASCII art grids encode polyomino shapes intuitively and are visually verifiable. The codegen step is explicit and cheap.

Shape encoding example:

```yaml
- id: 1
  buttons: 2
  time: 3
  income: 0
  shape: |
    .X
    XX
    .X
```

Each `.` is an empty cell, each `X` is an occupied cell. The shape is normalised to its bounding box. All rotations/reflections are handled at game logic time, not in the catalog.

**Alternative considered:** JSON — noisier to hand-edit; multiline strings are awkward. CSV — cannot represent nested/2D data cleanly.

### Codegen: R → committed C++ header

R is already in the toolchain for game analysis. Having R emit `src/generated/patches.hpp` makes the pipeline explicit: edit `data/patches.yaml`, run `codegen/generate_patches.R`, commit the result. No build-time R dependency, no hidden generation step.

**Alternative considered:** Python codegen — R is already a project dependency; adding Python for codegen only increases the toolchain surface unnecessarily.

### Game state: dense bitfield structs targeting 128-bit footprint

For MCTS, the engine will copy game states millions of times. A compact representation reduces memory bandwidth. The design targets 128 bits (16 bytes) per `PlayerState`:

| Field               | Bits   | Range                    |
|---------------------|--------|--------------------------|
| Quilt board         | 81     | 9×9 grid, 1 bit per cell |
| Time track position | 6      | 0–53                     |
| Button balance      | 7      | 0–127                    |
| Button income       | 5      | 0–31                     |
| **Total**           | **99** | fits in 128 bits         |

Shared `GameState` wraps two `PlayerState` instances plus:

| Field              | Bits   | Range                                 |
|--------------------|--------|---------------------------------------|
| Patch availability | 33     | one bit per patch                     |
| Circle marker      | 6      | 0–32                                  |
| 7×7 bonus          | 2      | unclaimed / P1 / P2                   |
| **Total**          | **41** | fits in 64 bits                       |

Full game state: 2×128 + 64 = 320 bits = 40 bytes.

**Known gap:** The initial random circular arrangement of the 33 patches (a permutation fixed at game start) is not captured here. This is required for legal move generation and will be added in the next change as a `std::array<uint8_t, 33>` in `GameState`. R will play a role in managing canonical game setups for study (analogous to its role in the patch catalog).

Implementation uses idiomatic C++ (`std::bitset<81>`, bitfield members, or explicit shift/mask helpers) — the compiler is trusted to optimise to efficient 64/128-bit operations.

**Alternative considered:** `uint8_t[9][9]` array for board — readable but 9× larger; kills cache performance at MCTS scale.

## Risks / Trade-offs

- **ASCII art parsing complexity** → Mitigation: R's string manipulation is well-suited; add a validation step that checks shape cell counts match expected patch sizes.
- **Bit layout changes are breaking** → Mitigation: Define layout in a single header; tests that round-trip field values catch regressions immediately.
- **Meson wrap ecosystem is smaller than vcpkg** → Mitigation: All required packages (Catch2, nlohmann/json) already have wraps; risk is low for this scope.
