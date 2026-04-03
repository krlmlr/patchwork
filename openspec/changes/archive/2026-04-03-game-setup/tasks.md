## 1. `GameSetup` C++ Type

- [x] 1.1 Create `src/game_setup.hpp` declaring the `GameSetup` struct in namespace `patchwork`:
  - `std::array<uint8_t, 33> circle` — integer patch IDs (0–32) in circle order
  - Constructor `GameSetup(std::string_view sv)` that accepts a 33-char string of single-char patch names and converts each to its integer ID via the patch catalog
- [x] 1.2 Add `void GameSetup::to_ndjson(std::ostream& out) const` that writes one JSON line: `{"type":"setup","circle":"<33-char-string>"}` (converting IDs back to single-char names for output)
- [x] 1.3 Add `GameSetup` to the umbrella header `src/patchwork.hpp`

## 2. R Setup Generator

- [x] 2.1 Create `codegen/generate_setups.R` that:
  - Reads `data/patches.yaml` to obtain the ordered single-char patch names (33 chars)
  - Uses a named constant `N_SETUPS <- 100L`; for each setup `i` in `seq_len(N_SETUPS)`, calls `set.seed(i)` and `sample(33)` to produce the permutation, then maps indices to patch names to form a 33-char string
- [x] 2.2 Write all results to `src/generated/game_setups.hpp` as `constexpr std::array<std::string_view, kNumGameSetups> kGameSetups` with `inline constexpr std::size_t kNumGameSetups = 100`; no seeds or struct definition in the header
- [x] 2.3 Ensure that re-running the script with a larger `N_SETUPS` produces a longer array whose first 100 entries are bit-for-bit identical to the previous output
- [x] 2.4 Run the script (`Rscript codegen/generate_setups.R`) and commit the resulting `src/generated/game_setups.hpp`

## 3. Mise Task

- [x] 3.1 Add a `[tasks.codegen:setups]` entry to `.mise.toml`:
  - `description = "Generate canonical game setups in src/generated/game_setups.hpp"`
  - `run = "Rscript codegen/generate_setups.R"`

## 4. Unit Tests

- [x] 4.1 Create `tests/test_game_setup.cpp` with a Catch2 test section covering construction from string:
  - The resulting circle contains each of the integer IDs 0–32 exactly once
  - Two constructions from the same string produce identical circles
- [x] 4.2 Add a Catch2 test covering `kGameSetups` from the generated header:
  - The array contains exactly 100 entries
  - Each entry is exactly 33 characters
  - Each entry is a permutation of the 33 single-char patch names
  - The last character of every entry is `'2'` (the two-square tile)
- [x] 4.3 Add a Catch2 test covering `to_ndjson`: construct a `GameSetup` from `kGameSetups[0]`, verify the output is valid single-line JSON containing `"type":"setup"` and the circle as a 33-char string
- [x] 4.4 Register `tests/test_game_setup.cpp` in `tests/meson.build`
- [x] 4.5 Run `meson test -C build` and verify all tests pass

## 5. Spec

- [x] 5.1 Create `openspec/specs/game-setup/spec.md` with BDD-style scenarios covering:
  - Construction from string produces a valid permutation of integer IDs 0–32
  - Last tile in every canonical setup is `'2'`
  - `kGameSetups` contains exactly 100 entries, each a valid 33-char permutation; first 100 stable if more are generated
  - `to_ndjson` output is single-line JSON with circle as a string (no seed)
