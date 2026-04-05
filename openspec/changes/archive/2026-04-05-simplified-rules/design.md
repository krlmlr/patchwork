## Context

The previous phase ("Start Without Piece Placement") introduced `SimplifiedGameState`: a compact 32-bit-per-player representation that replaces the 81-bit quilt board with a `free_spaces` counter. The state struct is complete and tested, but there is no logic to actually play a game. This change adds the rule layer on top of that state.

The Patchwork "simplified" ruleset is the full game minus spatial quilt-board reasoning. Each turn the active player either buys one of up to three patches visible ahead of the circle marker, or advances (earning 1 button per space advanced). Patches cost buttons and time; time position determines who moves next (the player further back goes first). The game ends when both players have exhausted the time track (position ≥ 53). Score = buttons − 2 × free_spaces + 7×7 bonus if held.

The `kPatches` array (generated from `data/patches.yaml`) is the single source of truth for patch costs, time, income, and cell count. The rule layer reads it directly.

## Goals / Non-Goals

**Goals:**
- Legal move enumeration for the active player (buy patch or advance)
- Move application: new state after buying a patch or advancing
- Leather patch award at five time thresholds (positions 26, 32, 38, 44, 50 on the time track); mandatory placement
- Button income payout at nine income spaces (positions 5, 11, 17, 23, 29, 35, 41, 47, 53)
- 7×7 bonus: claimed by first player to reach 56+ occupied cells (81 − free_spaces ≥ 56)
- Terminal detection and final score calculation
- NDJSON game logging: game-start, move, game-end events
- Random agent: uniformly selects a legal move; seeded for reproducibility
- Play driver: loops the game with two agents, writing log to a file

**Non-Goals:**
- Spatial quilt-board representation or placement reasoning (deferred to "Piece Placement Agent" phase)
- Non-random agents (deferred to "Random Sampling Agents" and later phases)
- UI, TUI, or interactive mode

## Decisions

### 1. Move as a tagged union value type

A move is either `BuyPatch{patch_index}` (index into `kPatches`, 0-based) or `Advance{}`. A `std::variant<BuyPatch, Advance>` (or a small discriminated struct) keeps the type closed and avoids heap allocation. Alternatives: plain integer encoding, polymorphic hierarchy. Plain integer is compact but opaque; a variant is self-documenting and zero-overhead at the call sites we care about.

### 2. Non-mutating move application returning new state

`apply_move(SimplifiedGameState state, Move move) → SimplifiedGameState` takes by value and returns a new state. This matches the MCTS use-case (build a search tree) and avoids surprising aliasing bugs. The state is only 80 bytes; copies are cheap. Alternative: mutate in place. Rejected because future MCTS phases will hold many states simultaneously.

### 3. Active-player rule: tracked in state via a 1-bit `next_player` field

The Patchwork rule is: the active player stays active until their position *strictly exceeds* the inactive player's position. When positions are equal the active player has not yet overtaken and therefore remains active. This rule cannot be derived from positions alone at a tie — a 1-bit `next_player` field is added to `SimplifiedGameState`'s shared word (bit 41, currently unused). It stores which player acts next and is updated by `apply_move` as follows: after a move the moved player's new position is compared to the opponent's position. If strictly greater, the opponent becomes `next_player`; otherwise the moved player remains `next_player`. At game start, player 0 is `next_player`.

A free function `active_player(const SimplifiedGameState&) → int` reads this field.

### 4. Leather patches: five thresholds, no new state, derived from positions

The time track has five 1×1 leather patch squares at positions 26, 32, 38, 44, and 50. The first player to reach or pass each threshold claims the leather patch for that square (mandatory placement: `free_spaces` is decremented by 1). No new state flags are needed: whether a threshold has been claimed is derived from the players' current positions at the time of the move — if either player's *pre-move* position is already ≥ the threshold, the patch was already taken; otherwise the first player to cross it now claims it. This derivation is evaluated inside `apply_move` for each threshold the moving player crosses, so claim status is never stale.

### 5. Positions may exceed 53

Position 53 is the last active square; positions up to 63 are representable in the existing 6-bit field. Allowing positions greater than 53 avoids a cap branch in `Advance` (the moving player can land at `opponent.position + 1` even when that exceeds 53). A player is "done" when their position ≥ 53; the game is terminal when both players are done. This simplifies move application and reduces branching in the game loop.

### 6. No draws: first-to-finish tiebreaker

The rulebooks (both original and anniversary edition) explicitly state: "In case of a tie, the player who reached the final space of the time board first wins." This makes draws structurally impossible — on equal scores there is always a winner. No `-1` sentinel is needed.

To implement the tiebreaker, `SimplifiedGameState`'s shared word gains a 1-bit `first_to_finish` field at bit 42 (currently unused). `apply_move` sets it exactly once: the first time a player's position transitions from < 53 to ≥ 53 while the other player is still < 53. `winner` reads this bit when scores are equal and returns the player recorded there.

### 7. NDJSON logging via `<fstream>` + nlohmann/json or hand-rolled

The codebase has no JSON dependency. Two options: (a) add `nlohmann/json` as a Meson subproject, (b) hand-roll the small fixed-schema log lines. The log schema is simple and stable (four event types, fixed fields). Hand-rolled formatting keeps the dependency graph minimal and is consistent with the existing codebase style. If the schema grows, a proper JSON library can be added then.

### 8. Random agent as a free function, not a class

`random_move(const SimplifiedGameState&, std::mt19937&) → Move` is a pure function parameterised by an rng. This is simpler than a class hierarchy at this stage; a polymorphic `Agent` interface can be introduced when a second agent type is added.

### 9. Play driver as a standalone `main` in `cpp/`

A small `play_driver.cpp` with its own `main` entry point, compiled as a separate Meson executable target. Input: `--seed <n> --setup <file>` (or positional). Output: NDJSON log to stdout or `--output <file>`. This matches the roadmap's "reproducible random play" goal without coupling game logic to I/O.

## Risks / Trade-offs

- **first_to_finish bit in shared word** — Adding 1 bit (bit 42) requires the shared word to have room. Bits 0–41 are used (33 patch + 6 circle + 2 bonus + 1 next_player), leaving 22 bits free. [Risk: bit-layout regression] → Mitigation: static_assert on field positions; existing round-trip tests catch regressions.
- **Derived leather-patch claim state** — Leather patch availability is derived from pre-move positions rather than stored flags. This is always correct for sequential single-move play (the only supported mode now), but must be re-evaluated if batch state mutations are ever added. [Risk: none practical for this phase]
- **Score overflow** — `free_spaces` max is 81, so worst-case score penalty is 162. `buttons` max visible in the test is 127. Using `int` throughout is safe. [Risk: none practical]
- **NDJSON log size** — Each move line is ~200 bytes; a full game is ~50–100 moves → ~20 KB per game. Not a concern.
- **Simplified vs. full rules divergence** — This implementation intentionally ignores spatial placement. The 7×7 bonus uses `free_spaces` as a proxy (56+ cells occupied), which is correct under the simplification. When spatial rules are added later, the bonus check will need to inspect the actual board, not `free_spaces`. [Risk: confusion] → Mitigation: document the simplification in the spec and in code comments.

## Open Questions

- Should `apply_move` assert on legality, or return an `expected<SimplifiedGameState, Error>`? Starting with assert + documented precondition; can relax later.
- Does the play driver need a `--count` flag to play multiple games in one invocation? Deferred — out of scope for this phase but easy to add.
