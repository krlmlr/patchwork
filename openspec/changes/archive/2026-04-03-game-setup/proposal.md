## Why

`GameState` currently has no record of which patch is at each position around the circle — only the current circle marker position. The initial circular arrangement of all 33 patches (a permutation fixed at game start) is the missing piece identified at the end of the Foundation phase. Without it, legal move generation cannot determine which patches a player can see (the three patches ahead of the circle marker). Establishing `GameSetup` as a reproducible, R-generated, logged artefact closes this gap and completes the data model before any rule logic is written.

## What Changes

- Introduce a **`GameSetup` struct** in `src/game_setup.hpp`: stores the starting circle order as a `std::array<uint8_t, 33>` of integer patch IDs; constructor accepts a 33-char string of single-char patch names (as defined in `data/patches.yaml`) and converts to IDs
- Add an **R script** (`codegen/generate_setups.R`) that generates 100 canonical game setups and emits them as `constexpr` string literals into `src/generated/game_setups.hpp`; no YAML files and no runtime I/O are required
- Commit the **initial generated header** (`src/generated/game_setups.hpp`) containing exactly 100 `constexpr` setup entries as a versioned reference for tests and future analysis
- Add a **log helper** that serialises a `GameSetup` to an NDJSON record (33-char circle string), ready for the logging pipeline introduced in a later phase
- Add **unit tests** covering construction, `constexpr` access, and serialisation

## Capabilities

### New Capabilities

- `game-setup`: `GameSetup` struct with string-to-ID constructor and NDJSON serialisation; R script that generates `constexpr` string literals in `src/generated/game_setups.hpp`; committed initial header with 100 setups

### Modified Capabilities

- (none — `GameState` is unchanged; `GameSetup` is a companion, not an extension)

## Impact

- The patch circle is now fully specified at game start, unblocking legal move generation in the next phase
- Canonical setups in `src/generated/game_setups.hpp` give reproducible baselines for game-tree analysis and RL training with zero runtime I/O overhead
- R is the sole authority for generating permutations; C++ only consumes `constexpr` string literals
- No changes to `GameState`, `PlayerState`, the patch catalog, or the build system
