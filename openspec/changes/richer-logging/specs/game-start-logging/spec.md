## ADDED Requirements

### Requirement: game_start event includes patch circle
The `log_game_start` function SHALL emit a `"circle"` field in the `game_start` NDJSON line containing the full initial patch circle as a 33-character string of single-character patch names, in circle order starting from index 0 of the `GameSetup`.

#### Scenario: All 33 patches present at game start
- **WHEN** `log_game_start` is called with a valid `GameSetup` and initial `SimplifiedGameState`
- **THEN** the emitted JSON line SHALL contain a `"circle"` field whose value is a 33-character string matching the patch names of the `GameSetup` circle in order

#### Scenario: Circle string encodes patch names not IDs
- **WHEN** the circle is serialised
- **THEN** each character SHALL be the single-character patch name (as stored in `kPatches[id].name`), not the numeric patch ID
