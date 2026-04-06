## Context

`PlayerState` currently stores an 81-bit quilt board (9×9 grid), time position (6 bits), button balance (7 bits), and button income (5 bits) packed into 128 bits. `GameState` pairs two `PlayerState` instances with shared state (patch availability bitmask, circle marker, 7×7 bonus). This representation is complete but expensive when the quilt board is irrelevant — spatial placement requires enumerating patch orientations and fit positions, which is combinatorially large compared to the economy-only decision tree.

The roadmap calls for a "Start Without Piece Placement" phase that defers spatial reasoning entirely: the quilt board collapses to a single integer counting free spaces (0–81). The open question left by the roadmap is whether this simplified state should be a **separate hard-coded struct** or a **template variant** of the existing types.

## Goals / Non-Goals

**Goals:**
- Introduce `SimplifiedPlayerState` and `SimplifiedGameState` that replace the 81-bit board with a 7-bit free-space counter
- Keep `PlayerState` and `GameState` untouched — no regressions, no template changes to existing types
- Provide unit tests for the new types with the same coverage standard as `PlayerState`

**Non-Goals:**
- Legal move generation, move application, or terminal detection (those belong to the Simplified Rules phase)
- NDJSON serialisation of the new types (deferred to the logging phase)
- Any spatial/placement logic

## Decisions

### Decision: Hard-coded separate struct, not a template parameter

**Chosen:** Introduce `SimplifiedPlayerState` as a standalone struct in `cpp/simplified_game_state.hpp`.

**Rejected:** A `template <bool Placement> PlayerState` (or similar `PlacementMode` enum parameter).

**Rationale:**
- The two state types diverge in semantics, not just storage. `PlayerState` tracks *which* cells are occupied; `SimplifiedPlayerState` tracks *how many* cells remain free. Collapsing them under a single template creates a leaky abstraction — callers would need the template parameter everywhere, and the full-board interface (cell accessors, row/col coordinates) becomes meaningless in the simplified branch.
- A template parameter would require all downstream code (tests, move generators, agents) to carry the same parameter, making the API viral. A separate named type keeps each path self-contained and easy to delete when spatial placement is added back.
- C++23 concepts/`if constexpr` can reduce duplication between the two types if needed, but the upfront complexity cost is not justified for two small structs at this stage.
- Separate structs make the transition explicit: calling code opts in to `SimplifiedGameState`, and there is no accidental use of the wrong variant.

**Accepted trade-off:** Some duplication of the scalar fields (position, buttons, income) and `GameState` shared fields. This is small (< 60 lines of struct definition) and acceptable.

### Decision: Pack `SimplifiedPlayerState` into 32 bits

**Chosen:** `SimplifiedPlayerState` SHALL fit in 32 bits: free spaces (7 bits, 0–81), time position (6 bits, 0–53), buttons (7 bits, 0–127), income (5 bits, 0–31), padding to 32 bits.

**Rationale:** The 128-bit bound for `PlayerState` was set so two instances plus shared state fit in a cache line. With a 7-bit free-space counter instead of an 81-bit board, the natural packing is 32 bits per player, which is even tighter. Two `SimplifiedPlayerState` instances plus the ~50-bit shared state comfortably fit in 16 bytes total.

### Decision: Free spaces, not occupied spaces

**Chosen:** The counter tracks *free* spaces (cells not yet covered).

**Rationale:** Patches always consume free spaces; free spaces start at 81 and decrease monotonically. A free-space counter is directly useful for computing the end-game score (remaining free spaces are subtracted from the button balance). Occupied spaces would require subtracting from 81 at scoring time.

## Risks / Trade-offs

- **Loss of spatial information** → Accepted; this is the explicit purpose of this phase. Piece placement will be re-introduced in a later phase with the full `PlayerState`.
- **Duplication of scalar fields** → Mitigated by keeping the structs small and co-located in one header. If divergence becomes painful, a `PlayerEconomy` base struct can be extracted later.
- **Naming clarity** → Code that mixes `GameState` and `SimplifiedGameState` must be careful. Mitigated by using distinct type names with no implicit conversions.
