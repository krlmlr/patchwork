## MODIFIED Requirements

### Requirement: GameSetup encodes the initial patch circle arrangement

`GameSetup` SHALL hold the initial circular arrangement of all 33 patches as a `std::array<char, 33>` of single-character patch names (as defined in `data/patches.yaml`) and the `uint64_t` seed used to generate it.

#### Scenario: from_seed produces a valid permutation

- **WHEN** `GameSetup::from_seed(s, patch_names)` is called for any seed `s` and ordered array of 33 single-char patch names
- **THEN** the resulting circle contains each of the 33 distinct patch name characters exactly once

#### Scenario: from_seed is deterministic

- **WHEN** `GameSetup::from_seed(s, patch_names)` is called twice with the same seed `s`
- **THEN** both results have identical circle arrays

#### Scenario: from_seed produces distinct permutations for different seeds

- **WHEN** `GameSetup::from_seed(s1, patch_names)` and `GameSetup::from_seed(s2, patch_names)` are called with `s1 ≠ s2`
- **THEN** the two circle arrays are not equal (with overwhelming probability)

## REMOVED Requirements

### Requirement: GameSetup can be loaded from a YAML setup file

**Reason**: Setups are now embedded as `constexpr` string literals in `src/generated/game_setups.hpp`; runtime file loading is no longer needed or provided.
**Migration**: Access canonical setups via `patchwork::kGameSetups[i]` from the generated header instead of calling `GameSetup::load(path)`.

## MODIFIED Requirements

### Requirement: GameSetup serialises to an NDJSON record

`GameSetup::to_ndjson` SHALL emit exactly one JSON line to the provided `std::ostream` containing the type tag, seed, and circle as a 33-character string.

#### Scenario: NDJSON output is a single line with correct fields

- **WHEN** `to_ndjson(out)` is called on a `GameSetup`
- **THEN** the output contains a single newline-terminated JSON object
- **AND** the object has `"type": "setup"`, `"seed": <value>`, and `"circle": "<33-char-string>"`

### Requirement: Canonical setups are embedded in `src/generated/game_setups.hpp`

The repository SHALL contain a committed generated header `src/generated/game_setups.hpp` produced by `codegen/generate_setups.R`, defining a `constexpr std::array<GameSetupEntry, 100>` named `kGameSetups` in namespace `patchwork`. Each `GameSetupEntry` pairs a `std::string_view circle` (exactly 33 characters) with a `uint64_t seed`.

#### Scenario: Generated header is present and contains 100 entries

- **WHEN** the repository is cloned
- **THEN** `src/generated/game_setups.hpp` exists and `patchwork::kGameSetups` has exactly 100 entries

#### Scenario: Each entry is a valid permutation of patch names

- **WHEN** any entry `e` in `patchwork::kGameSetups` is examined
- **THEN** `e.circle` is exactly 33 characters
- **AND** `e.circle` contains each of the 33 single-char patch names exactly once

### Requirement: GameSetup is unit tested

All `GameSetup` behaviours SHALL have Catch2 unit tests covering `from_seed`, access via `kGameSetups`, and `to_ndjson`.

#### Scenario: Tests exist and pass

- **WHEN** `meson test -C build` is run
- **THEN** all game-setup tests pass with exit code 0
