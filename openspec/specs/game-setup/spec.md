### Requirement: GameSetup encodes the initial patch circle arrangement

`GameSetup` SHALL hold the initial circular arrangement of all 33 patches as a `std::array<uint8_t, 33>` of integer patch IDs (0–32). The constructor SHALL accept a 33-character `std::string_view` of single-char patch names and convert each character to its integer ID by looking up the patch catalog.

#### Scenario: Construction from string produces a valid permutation

- **WHEN** `GameSetup` is constructed from a 33-char string of patch names
- **THEN** the internal circle contains each of the integer IDs `0` through `32` exactly once

#### Scenario: Last tile in every canonical setup is the two-square patch

- **WHEN** any canonical setup string from `patchwork::kGameSetups` is examined
- **THEN** the last character (position 32) is `'2'` — the name of the two-square tile

### Requirement: GameSetup serialises to an NDJSON record

`GameSetup::to_ndjson` SHALL emit exactly one JSON line to the provided `std::ostream` containing the type tag and circle as a 33-character string.

#### Scenario: NDJSON output is a single line with correct fields

- **WHEN** `to_ndjson(out)` is called on a `GameSetup`
- **THEN** the output contains a single newline-terminated JSON object
- **AND** the object has `"type": "setup"` and `"circle": "<33-char-string>"`

### Requirement: Canonical setups are embedded in `cpp/generated/game_setups.hpp`

The repository SHALL contain a committed generated header `cpp/generated/game_setups.hpp` produced by `codegen/generate_setups.R`, defining a `constexpr std::array<std::string_view, kNumGameSetups>` named `kGameSetups` in namespace `patchwork`, where `kNumGameSetups` is a named constant matching the value hard-coded in the R script (initially `100`). Generating more setups in future SHALL leave the first 100 entries unchanged.

#### Scenario: Generated header is present and contains 100 entries

- **WHEN** the repository is cloned
- **THEN** `cpp/generated/game_setups.hpp` exists and `patchwork::kGameSetups` has exactly 100 entries

#### Scenario: Each entry is a valid permutation of patch names

- **WHEN** any entry in `patchwork::kGameSetups` is examined
- **THEN** it is exactly 33 characters
- **AND** it contains each of the 33 single-char patch names exactly once

### Requirement: GameSetup is unit tested

All `GameSetup` behaviours SHALL have Catch2 unit tests covering construction from string, access via `kGameSetups`, and `to_ndjson`.

#### Scenario: Tests exist and pass

- **WHEN** `meson test -C build` is run
- **THEN** all game-setup tests pass with exit code 0
