## 1. `GameSetup` C++ Type

- [ ] 1.1 Create `src/game_setup.hpp` declaring the `GameSetup` struct in namespace `patchwork`:
  - `std::array<uint8_t, 33> circle` — patch IDs in circle order
  - `uint64_t seed` — RNG seed used to produce this permutation
- [ ] 1.2 Add a static factory `GameSetup::from_seed(uint64_t seed)` that fills `circle` with `[0, 1, …, 32]` then calls `std::shuffle` with a seeded `std::mt19937_64`
- [ ] 1.3 Add a static factory `GameSetup::load(const std::string& path)` that opens the YAML file at `path`, parses the `seed:` and `circle: [...]` lines, and returns a `GameSetup`; throw `std::runtime_error` on malformed input
- [ ] 1.4 Add `void GameSetup::to_ndjson(std::ostream& out) const` that writes one JSON line: `{"type":"setup","seed":<seed>,"circle":[<ids>]}`
- [ ] 1.5 Add `GameSetup` to the umbrella header `src/patchwork.hpp`

## 2. R Setup Generator

- [ ] 2.1 Create `codegen/generate_setups.R` that accepts an optional command-line argument `n` (number of setups to generate, default 10) and an optional `seed_start` (default 1)
- [ ] 2.2 For each setup `i` in `1:n`, use `set.seed(seed_start + i - 1)` and `sample(0:32)` to produce the permutation
- [ ] 2.3 Write each result to `data/setups/setup-NNNNN.yaml` (zero-padded five-digit ID) with `id`, `seed`, and `circle` fields; `circle` is emitted as a YAML flow sequence on one line
- [ ] 2.4 Ensure the output directory `data/setups/` is created by the script if it does not exist
- [ ] 2.5 Run the script (`Rscript codegen/generate_setups.R`) and commit the resulting `data/setups/setup-00001.yaml` through `setup-00010.yaml`

## 3. Mise Task

- [ ] 3.1 Add a `[tasks.codegen:setups]` entry to `.mise.toml`:
  - `description = "Generate canonical game setups in data/setups/"`
  - `run = "Rscript codegen/generate_setups.R"`

## 4. Unit Tests

- [ ] 4.1 Create `tests/test_game_setup.cpp` with a Catch2 test section covering `from_seed`:
  - Two calls with the same seed produce identical circles
  - Two calls with different seeds produce different circles
  - The resulting circle is a permutation of `[0, 1, …, 32]` (all 33 patch IDs present exactly once)
- [ ] 4.2 Add a Catch2 test covering `load`: load `data/setups/setup-00001.yaml` (path relative to `PATCHWORK_DATA_DIR` or a compile-time `TEST_DATA_DIR` definition), verify the circle is a valid permutation and the seed matches the YAML value
- [ ] 4.3 Add a Catch2 test covering `to_ndjson`: serialise a `from_seed`-constructed setup, verify the output is valid single-line JSON containing `"type":"setup"`, the correct seed, and all 33 circle IDs
- [ ] 4.4 Register `tests/test_game_setup.cpp` in `tests/meson.build`; pass `-DTEST_DATA_DIR=...` as a compile definition pointing to `data/setups/` so the load test can find the YAML file
- [ ] 4.5 Run `meson test -C build` and verify all tests pass

## 5. Spec

- [ ] 5.1 Create `openspec/specs/game-setup/spec.md` with BDD-style scenarios covering:
  - `from_seed` determinism (same seed → same circle)
  - `from_seed` produces a valid permutation of `[0 … 32]`
  - `load` round-trip (file written by R script, read by C++ loader → same values)
  - `to_ndjson` output is single-line JSON with correct fields
