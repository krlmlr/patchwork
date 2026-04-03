## Why

With `GameSetup` encoding the initial patch-circle arrangement, the engine now has all the data it
needs to compute legal moves. Piece placement is the core game action in Patchwork — choosing one
of the three visible patches, orienting it (rotate + flip), placing it on the 9×9 quilt board, and
advancing the time track. Implementing this unlocks the ability to simulate complete game sequences,
which is the direct prerequisite for MCTS and RL training.

## What Changes

- Add a **patch orientation engine** that transforms any patch's cell list through all 8 canonical
  orientations (4 rotations × 2 flips) and de-duplicates symmetric variants
- Add a **placement validator** (`fits_at`) that checks whether an oriented patch placed at a
  given (row, col) anchor fits on a `PlayerState` board (in-bounds + no overlap)
- Define **move types** `PlacePatch` (circle offset, orientation, anchor row/col) and
  `AdvanceAndReceive` (no data), plus a `Move` variant combining both
- Add `visible_patches` to query the three patches currently ahead of the circle marker in a
  `GameSetup` + `GameState`
- Add `legal_moves` to enumerate every `Move` (all valid `PlacePatch` × orientation × anchor
  combinations, plus the always-legal `AdvanceAndReceive`)
- Add `apply` to apply a `Move` to a `GameState`: deduct buttons, advance time, stamp board cells,
  advance circle marker past the chosen patch, and award **button income** when the player's time
  token crosses any income space
- Add **unit tests** covering the orientation engine, fit checking, move generation, and move
  application (including button-income triggering)
- Add a `[tasks.codegen:setups]` mise task (carries forward from `game-setup` scope)

**Simplified scope** — the following are intentionally deferred to a later phase:
- The special 1×1 leather patch awarded at position 26 (or the neutral-token position)
- Game-end detection (both players at position 53) and scoring
- The 7×7 bonus tile (detection and point award)

## Capabilities

### New Capabilities

- `piece-placement`: patch orientation engine (rotate/flip, dedup), placement validator, `PlacePatch`
  and `AdvanceAndReceive` move types, `legal_moves` generator, `apply` move-application function
  (including button-income awards when crossing income spaces)

### Modified Capabilities

- (none — `GameSetup` and `GameState` data layouts are unchanged; this change only adds functions
  that consume them)

## Impact

- New header `src/moves.hpp` with all move logic; included in `src/patchwork.hpp`
- `src/game_setup.hpp` is consumed but not modified (depends on `game-setup` being implemented)
- No changes to `GameState`, `PlayerState`, the patch catalog, or the build system
- The button income spaces (the 9 time-track positions that trigger income) are encoded as a
  `constexpr` array in `src/moves.hpp`; their exact positions match the published Patchwork rules
