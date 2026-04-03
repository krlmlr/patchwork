## 1. `GameSetup` C++ Type

- [ ] 1.1 Create `src/game_setup.hpp` declaring the `GameSetup` struct in namespace `patchwork`:
  - `std::array<char, 33> circle` — single-char patch names in circle order
  - `uint64_t seed` — RNG seed used to produce this permutation
- [ ] 1.2 Add a static factory `GameSetup::from_seed(uint64_t seed, const std::array<char, 33>& patch_names)` that fills a local index array `[0, 1, …, 32]`, shuffles it with a seeded `std::mt19937_64`, then maps each index to `patch_names[index]`
- [ ] 1.3 Add `void GameSetup::to_ndjson(std::ostream& out) const` that writes one JSON line: `{"type":"setup","seed":<seed>,"circle":"<33-char-string>"}`
- [ ] 1.4 Add `GameSetup` to the umbrella header `src/patchwork.hpp`

## 2. R Setup Generator

- [ ] 2.1 Create `codegen/generate_setups.R` that:
  - Reads `data/patches.yaml` to obtain the ordered single-char patch names (33 chars)
  - Accepts an optional command-line argument `n` (number of setups to generate, default 100) and an optional `seed_start` (default 1)
- [ ] 2.2 For each setup `i` in `1:n`, use `set.seed(seed_start + i - 1)` and `sample(33)` to produce the permutation, then map indices to patch names to form a 33-char string
- [ ] 2.3 Write all results to `src/generated/game_setups.hpp` as a `constexpr std::array` of `GameSetupEntry` values (each pairing a `std::string_view` circle with its `uint64_t` seed), with a `GameSetupEntry` struct defined in the same header
- [ ] 2.4 Run the script (`Rscript codegen/generate_setups.R`) and commit the resulting `src/generated/game_setups.hpp`

## 3. Mise Task

- [ ] 3.1 Add a `[tasks.codegen:setups]` entry to `.mise.toml`:
  - `description = "Generate canonical game setups in src/generated/game_setups.hpp"`
  - `run = "Rscript codegen/generate_setups.R"`

## 4. Unit Tests

- [ ] 4.1 Create `tests/test_game_setup.cpp` with a Catch2 test section covering `from_seed`:
  - Two calls with the same seed produce identical circles
  - Two calls with different seeds produce different circles
  - The resulting circle contains each of the 33 single-char patch names exactly once
- [ ] 4.2 Add a Catch2 test covering `kGameSetups` from the generated header:
  - The array contains exactly 100 entries
  - Each entry's `circle` is exactly 33 characters
  - Each entry's `circle` is a permutation of the 33 single-char patch names
- [ ] 4.3 Add a Catch2 test covering `to_ndjson`: serialise a `from_seed`-constructed setup, verify the output is valid single-line JSON containing `"type":"setup"`, the correct seed, and the circle as a 33-char string
- [ ] 4.4 Register `tests/test_game_setup.cpp` in `tests/meson.build`
- [ ] 4.5 Run `meson test -C build` and verify all tests pass

## 5. Spec

- [ ] 5.1 Create `openspec/specs/game-setup/spec.md` with BDD-style scenarios covering:
  - `from_seed` determinism (same seed → same circle)
  - `from_seed` produces a valid permutation of the 33 single-char patch names
  - `kGameSetups` contains exactly 100 entries, each a valid 33-char permutation
  - `to_ndjson` output is single-line JSON with circle as a string
