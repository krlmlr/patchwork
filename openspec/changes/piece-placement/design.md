## Context

The Foundation phase established `GameState` (two `PlayerState` instances + 64-bit shared field)
and `GameSetup` (the initial patch circle arrangement + seed). The next logical step is to expose
the game's core action: choosing one of the three visible patches, orienting it on the quilt board,
and advancing the time track. The design.md for `game-setup` explicitly documents the calling
convention: `(const GameSetup&, GameState&)` for all legal-move functions.

This change adds `src/moves.hpp` — a single header with no new external dependencies — containing
the orientation engine, fit checker, move types, move generator, and move applicator.

## Goals / Non-Goals

**Goals:**

- Provide `visible_patches` to query the 3 patches ahead of the circle marker
- Provide an orientation engine that enumerates all distinct orientations for any patch (up to 8)
- Provide `fits_at` for board-placement validation
- Define `PlacePatch` and `AdvanceAndReceive` move types, plus a `Move` variant
- Provide `legal_moves` to enumerate every legal `Move` from a given position
- Provide `apply` to update a `GameState` (buttons, time, board, circle marker, button income)
- Fully unit-test all of the above

**Non-Goals:**

- The special 1×1 leather patch awarded at position 26 (deferred)
- Game-end detection (both players at 53) and scoring (deferred)
- The 7×7 bonus tile (detection and award deferred)
- Turn-order tie-breaking beyond "lower position goes next; ties → player 0"
- A GUI or any visualisation

## Decisions

### Single header `src/moves.hpp` (no new source file)

All piece-placement logic is pure functions that operate on `GameSetup`, `GameState`, and
`PlayerState` by reference or value. There is no mutable state to encapsulate, so a header-only
module keeps things simple. The pattern matches `game_state.hpp` (header-only).

**Alternative considered:** A `src/moves.cpp` translation unit — would reduce recompilation on
large builds, but unnecessary at this project's scale.

### Orientations encoded as index 0–7 (bit 0 = flip, bits 1–2 = rotation)

There are exactly 4 rotations (0°/90°/180°/270°) and 2 flip states (identity/horizontal flip),
giving 8 candidates. These are enumerated as a `constexpr std::array<uint8_t, 8>` of indices.
`orient_cells` transforms the patch cell list for a given index and normalises the result (min row
and col shifted to 0). Symmetry deduplication is performed by comparing normalised sorted cell
vectors; `unique_orientations` returns only distinct results.

**Alternative considered:** Precomputing and storing all unique orientations for every patch in
the generated header — saves runtime work but couples code generation to a specific game-logic
design and bloats `patches.hpp`; rejected.

### Cell list representation: `std::vector<CellOffset>`

Oriented cells are returned as a `std::vector<CellOffset>` (row/col pairs relative to the
placement anchor). This is small (≤ 8 elements for any Patchwork patch) and avoids the fixed-size
array complexity of `kPatches.cells`.

**Alternative considered:** `std::array<CellOffset, 8>` with a count — eliminates allocation but
makes the API more cumbersome; the vector cost is negligible for legal-move enumeration.

### `PlacePatch` stores circle offset (0/1/2), orientation index (0–7), and anchor (row, col)

`circle_offset` identifies *which* of the three visible patches is chosen (0 = nearest, 1 = second,
2 = third). Storing the offset rather than the patch ID keeps the move type independent of
`GameSetup`; the ID can be recovered via `visible_patches`.

**Alternative considered:** Store the patch ID directly — more self-contained but ties the move to
a specific setup permutation; the offset is the canonical game-rule representation.

### `apply` takes an explicit `int player_idx`

The current player is the one with the lower time-track position. `apply` takes an explicit
`player_idx` (0 or 1) so the caller controls turn order. A companion free function
`current_player(const GameState&) → int` returns the lower-position player (ties → player 0, an
acknowledged simplification).

**Alternative considered:** Encode active-player in `GameState` with a spare bit — more accurate
for tie-breaking but changes the data model; deferred until turn-order tie-breaking is specified.

### Button income spaces encoded as `constexpr std::array`

The 9 button-income spaces on the time track (at positions 5, 11, 17, 23, 29, 35, 41, 47, 53 in
the 0-indexed 54-space track) are stored as a `constexpr` array in `src/moves.hpp`. When a
player's position advances, `apply` counts how many income spaces lie strictly between old and new
positions and awards `income × count` buttons.

**Alternative considered:** Compute income spaces procedurally (5 + 6k) — the formula is not
exact for the real game board, which has irregular spacing; the constant array is the ground truth.

### `AdvanceAndReceive` — advance to `other_pos + 1`, receive one button per space crossed

When a player advances and receives buttons, they move their token to exactly one space *ahead* of
the other player's token and collect one button per space they moved. This is per the standard
Patchwork rules. Button income from crossed income spaces is also awarded.

## Risks / Trade-offs

- **Orientation deduplication at runtime** → Mitigation: deduplication runs once per
  `legal_moves` call, over ≤ 8 candidate orientations with ≤ 8 cells each; negligible cost.
- **`fits_at` called for every (orientation × anchor) combination** → Mitigation: at most
  8 orientations × 81 anchors × 3 patches = ~2 000 fit checks per `legal_moves` call; each is
  O(cells) ≤ 8 operations — well within the MCTS node-expansion budget.
- **Tie-breaking (player 0) is not strictly correct** → Mitigation: documented as a known
  approximation; the correct rule (last-mover goes again) requires additional state and is deferred.
- **Button-income space positions hard-coded** → Mitigation: positions come from the published
  Patchwork rulebook and are verified by a unit test that counts 9 entries.
