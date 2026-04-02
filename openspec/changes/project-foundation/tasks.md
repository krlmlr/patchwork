## 1. Meson Build System

- [x] 1.1 Create `meson.build` at the project root with project name, C++23 standard, and default warning flags
- [x] 1.2 Add `subprojects/` directory and fetch Catch2 v3 wrap (`meson wrap install catch2`)
- [x] 1.3 Create `src/meson.build` declaring the engine library target (initially empty or with a placeholder header)
- [x] 1.4 Create `tests/meson.build` declaring a Catch2 test executable linked against the engine library
- [x] 1.5 Add a trivial baseline test in `tests/test_main.cpp` that asserts `true` — verify `meson setup build && meson test -C build` passes
- [x] 1.6 Create `logs/` directory with a `.gitkeep` and add `logs/*.ndjson` to `.gitignore`

## 2. Patch Catalog

- [x] 2.1 Create `data/patches.yaml` with all 33 Patchwork patches, each with `id`, `buttons`, `time`, `income`, and ASCII art `shape` field (`.` / `X` grid)
- [x] 2.2 Validate catalog: confirm exactly 33 entries, each shape row has equal length, and `X` cell counts match expected patch sizes

## 3. R Codegen

- [x] 3.1 Create `codegen/generate_patches.R` that reads `data/patches.yaml`, parses ASCII art shapes into `(row, col)` offset lists, and writes `src/generated/patches.hpp`
- [x] 3.2 Define the output format: a `PatchData` struct and a `constexpr std::array<PatchData, 33> kPatches` in the generated header, with all cell offsets embedded as `constexpr` initialiser lists
- [x] 3.3 Run `codegen/generate_patches.R` and commit the resulting `src/generated/patches.hpp`
- [x] 3.4 Add a compile-time check (`static_assert`) in the generated header that `kPatches.size() == 33`

## 4. Game State Types

- [ ] 4.1 Create `src/player_state.hpp` defining `PlayerState` with: `std::bitset<81>` board, and packed scalar fields for position (0–53), buttons (0–127), income (0–31)
- [ ] 4.2 Implement `PlayerState` default constructor initialising board empty, position 0, buttons 5, income 0
- [ ] 4.3 Implement cell accessors: `bool cell(int row, int col) const` and `void set_cell(int row, int col, bool v)` with bounds assertions
- [ ] 4.4 Add a `static_assert(sizeof(PlayerState) <= 16)` to enforce the 128-bit layout budget
- [ ] 4.5 Create `src/game_state.hpp` defining `GameState` with two `PlayerState` members and packed shared state: patch availability (33-bit mask), circle marker (0–32), 7×7 bonus (2-bit enum)
- [ ] 4.6 Implement `GameState` default constructor: both players default-constructed, all 33 patches available, circle marker 0, bonus unclaimed
- [ ] 4.7 Implement `GameState` accessors: `patch_available(int idx)`, `set_patch_available(int idx, bool)`, `circle_marker()`, `set_circle_marker(int)`, `bonus_status()`, `set_bonus_status(...)`

## 5. Unit Tests

- [ ] 5.1 Add `tests/test_player_state.cpp` with Catch2 tests covering: default construction values, cell set/get round-trip, all scalar field round-trips across their full ranges, `sizeof` assertion
- [ ] 5.2 Add `tests/test_game_state.cpp` with Catch2 tests covering: default construction values, patch availability round-trip for boundary indices (0 and 32), circle marker range, bonus status transitions
- [ ] 5.3 Register all test files in `tests/meson.build` and verify `meson test -C build` passes with all tests green
