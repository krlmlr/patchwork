## Why

`GameState` currently has no record of which patch is at each position around the circle — only the current circle marker position. The initial circular arrangement of all 33 patches (a permutation fixed at game start) is the missing piece identified at the end of the Foundation phase. Without it, legal move generation cannot determine which patches a player can see (the three patches ahead of the circle marker). Establishing `GameSetup` as a reproducible, R-generated, logged artefact closes this gap and completes the data model before any rule logic is written.

## What Changes

- Introduce a **`GameSetup` struct** in `src/game_setup.hpp`: a `std::array<uint8_t, 33>` recording the patch IDs in their starting circle order, plus the `uint64_t` seed used to generate that permutation
- Add an **R script** (`codegen/generate_setups.R`) that produces a requested number of canonical game setups as YAML files in `data/setups/`, each identified by a sequential ID and containing its seed and circle array
- Commit an **initial batch of canonical setups** (`data/setups/setup-00001.yaml` … `setup-00010.yaml`) as a versioned reference set for tests and future analysis
- Add a **C++ loader** (`GameSetup::load(path)`) that reads a setup YAML file and returns a `GameSetup`
- Add a **log helper** that serialises a `GameSetup` to an NDJSON record (seed + circle array), ready for the logging pipeline introduced in a later phase
- Add **unit tests** covering construction, round-trip load/serialise, and boundary values

## Capabilities

### New Capabilities

- `game-setup`: `GameSetup` struct with seeded-RNG construction, YAML file loading, and NDJSON serialisation; R script to generate and version canonical setups; committed initial setup batch

### Modified Capabilities

- (none — `GameState` is unchanged; `GameSetup` is a companion, not an extension)

## Impact

- The patch circle is now fully specified at game start, unblocking legal move generation in the next phase
- Canonical setups in `data/setups/` give reproducible baselines for game-tree analysis and RL training
- R is the sole authority for generating permutations; C++ only consumes them
- No changes to `GameState`, `PlayerState`, the patch catalog, or the build system
