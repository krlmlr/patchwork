## Context

The `patchwork` engine records every game as a sequence of NDJSON lines written by `game_logger.hpp/cpp`. Three event types exist: `game_start`, `move`, and `game_end`. The current implementation omits key fields needed by downstream R analysis:

- `game_start` records `seed` and `setup_id` but **not** the actual 33-patch circle arrangement — R cannot reconstruct the circle without joining against a separate setups file.
- `move` records only `position` and `buttons` for the active player — income, free_spaces, total board value, and patch circle state are absent, making it hard to compute patch economics from logs alone.
- `game_end` records scores and winner but omits final `income` and `free_spaces`, so R must replay moves to derive them.

`GameSetup` already has `to_ndjson()` and stores the circle as a 33-element array of patch IDs. `SimplifiedGameState` exposes `patch_available(idx)` and `circle_marker()`. All the data needed is already in memory at log time.

## Goals / Non-Goals

**Goals:**
- Emit the full initial patch circle (33-char patch-name string) in `game_start`
- Emit post-move per-player summary (`income`, `free_spaces`, `board_value`) and current patch circle snapshot in every `move` event
- Emit final per-player `income` and `free_spaces` in `game_end`
- Update all call sites (`play_driver.cpp` and `tui/tui_main.cpp`) to supply the new parameters
- Update existing Catch2 tests that assert NDJSON output

**Non-Goals:**
- Full spatial quilt-board logging (deferred to a later phase with full board state)
- Log schema versioning or migration tooling
- Changes to the R analysis scripts (those belong to the Tile Analysis phase)

## Decisions

### Pass `GameSetup` by const-ref to logging functions

`log_game_start` and `log_move` both need the `GameSetup` circle to reconstruct the patch circle string. The natural approach is to add a `const GameSetup&` parameter to each function.

**Alternatives considered:**
- Pre-compute a circle string and pass `std::string_view`: avoids the `GameSetup` header dependency in `game_logger.hpp` but forces callers to build the string themselves; adds ceremony without benefit.
- Cache the circle string in a logging context object: premature abstraction for a single-game-per-process driver.

**Decision:** Pass `const GameSetup&` directly. It's the simplest, zero-overhead option and the dependency is already present transitively in `play_driver.cpp` and `tui/tui_main.cpp`.

### Represent patch circle as a 33-char patch-name string in JSON

`GameSetup` already uses this encoding in `to_ndjson()`. Consistent representation across `game_start` and `move` events makes R parsing uniform.

**Alternatives considered:**
- Array of integer patch IDs: more compact as a diff, but less human-readable and inconsistent with `GameSetup::to_ndjson()`.

**Decision:** 33-char string of patch names, matching existing `GameSetup` convention.

### Define `board_value` as the current game score proxy (`buttons - 2 × free_spaces`)

The roadmap calls for "total board value" in move logs. In Patchwork the final score for a player is `buttons - 2 × free_spaces`. Logging this same formula per-move gives an immediate, self-contained signal of how a player's board is performing relative to a fully covered board. For a fresh player (5 buttons, 81 free spaces) this yields −157, close to −2×9×9 = −162, reflecting the starting "debt" of an empty quilt.

**Alternatives considered:**
- Add a new `cumulative_income_received` field to `SimplifiedPlayerState`: correct but invasive; deferred.
- `buttons - 5` (buttons above starting amount): easy to compute but lacks the spatial component; reviewed and rejected in favour of the game-score formula.

**Decision:** Log `board_value` as `buttons - 2 * free_spaces` — the same formula used at end of game.

### Include `patch_symbol` for buy-patch moves

When a player buys a patch, downstream R analysis needs to identify the patch without joining against a separate patch table. Including the single-character patch name (e.g. `"t"`) directly in the move line avoids that join and keeps the log self-contained.

**Decision:** Emit `"patch_symbol":"<char>"` immediately after `"patch_index"` in `buy_patch` move lines.

### Emit circle snapshot as remaining available patches in circle order

After each move the circle has been modified (one patch removed if a buy occurred, marker advanced). The snapshot should show the available patches starting from the circle marker position, wrapping around. This mirrors what a player sees on the table. We iterate `circle_marker()` to `circle_marker() + 32` (mod 33), emitting patch name if `patch_available(id)` is true.

**Alternatives considered:**
- Emit all 33 entries with a present/absent flag: verbose; R can reconstruct but the log grows.
- Emit only the next 3 purchasable patches: too narrow; loses circle context needed for analysis.

**Decision:** Emit all currently available patches in circle order starting from the marker, as a compact string of patch-name characters.

## Risks / Trade-offs

- **Existing log consumers break on new fields** → JSON libraries that use strict key validation would reject new fields; NDJSON is additive by convention and R's `jsonlite`/`ndjson` ignores unknown fields. No migration needed.
- **Test assertions need updating** → Any Catch2 test comparing exact NDJSON strings will fail after this change. Mitigation: update all such tests as part of the implementation tasks.
- **`board_value` uses game-score formula** → `buttons - 2 × free_spaces` matches the end-game scoring rule exactly and produces a meaningful negative value at game start (~−157), unlike `buttons - 5` which ignores the quilt board state.
