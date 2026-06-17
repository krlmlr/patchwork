## Why

Piece placement on the 9×9 quilt board is the most expensive part of evaluating a Patchwork position, yet the economy (buttons, income, time-track decisions) drives most of the strategic depth. By reducing the quilt board to a single free-space counter (0–81), we unlock a fast, minimal game state that is sufficient for studying the economy, training early RL agents, and completing the rule engine — all before tackling the combinatorial complexity of spatial placement.

## What Changes

- Introduce a **`SimplifiedPlayerState`** type (or a template variant of `PlayerState`) where the 81-bit quilt board is replaced by a single 7-bit integer counting free spaces (0–81); all other fields (time position, buttons, income) remain identical
- Introduce a corresponding **`SimplifiedGameState`** that pairs two `SimplifiedPlayerState` instances with the existing shared state (patch availability, circle marker, 7×7 bonus)
- **Decide the variant strategy**: hard-code a separate `SimplifiedPlayerState` struct, or introduce a `bool Placement` (or `enum Mode`) template parameter on `PlayerState`/`GameState` that switches between the full 81-bit board and the free-space integer; document the chosen approach in `design.md`
- Add **unit tests** for the new type(s) covering construction, field round-trips, and boundary values
- Leave `PlayerState` and `GameState` untouched so existing tests and the `GameSetup` layer remain green

## Capabilities

### New Capabilities

- `simplified-game-state`: `SimplifiedPlayerState` (free-space counter instead of quilt board) and `SimplifiedGameState`; includes unit tests and, if the template approach is chosen, a `PlacementMode` or equivalent compile-time switch

### Modified Capabilities

- (none — `PlayerState`, `GameState`, and `GameSetup` are unchanged)

## Impact

- Unblocks the **Simplified Rules** phase: legal move generation, move application, and terminal detection can all be written against `SimplifiedGameState` first
- No changes to the patch catalog, build system, R scripts, or canonical setup files
- If the template approach is chosen, `PlayerState` gains a template parameter — this is a breaking change to that type's interface but backward-compatible as long as the default parameter matches the current full-board behaviour
- The free-space integer loses spatial information; this is intentional and accepted for this phase
