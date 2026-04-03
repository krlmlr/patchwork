## Context

The previous phase ("Start Without Piece Placement") introduced `SimplifiedGameState`: a compact 32-bit-per-player representation that replaces the 81-bit quilt board with a `free_spaces` counter. The state struct is complete and tested, but there is no logic to actually play a game. This change adds the rule layer on top of that state.

The Patchwork "simplified" ruleset is the full game minus spatial quilt-board reasoning. Each turn the active player either buys one of up to three patches visible ahead of the circle marker, or advances (and earns buttons proportional to income). Patches cost buttons and time; time position determines who moves next (the player further back goes first). The game ends when both players have exhausted the time track (position ≥ 53). Score = buttons − 2 × free_spaces + 7×7 bonus if held.

The `kPatches` array (generated from `data/patches.yaml`) is the single source of truth for patch costs, time, income, and cell count. The rule layer reads it directly.

## Goals / Non-Goals

**Goals:**
- Legal move enumeration for the active player (buy patch or advance)
- Move application: new state after buying a patch or advancing
- Leather patch award at time thresholds (positions 26 and 53 on the time track)
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

### 3. Active-player rule: lower time position goes first; ties → player 0

The active player is determined by `player(0).position() < player(1).position()` with ties broken in favour of player 0. This is a documented Patchwork rule. Encoding it in a free function `active_player(const SimplifiedGameState&) → int` keeps it testable and avoids duplicating the logic across move generation and application.

### 4. Leather patches via threshold crossing

The time track has two special squares (26 and 53) where a 1×1 leather patch is awarded to the first player to reach or pass that position. The rule: if a player's new position crosses a threshold AND the threshold patch has not yet been claimed, award it (decrement `free_spaces` by 1, mark threshold as claimed in shared state). Two threshold flags are added to `SimplifiedGameState`'s shared 64-bit word (2 bits, currently unused). Alternative: derive claim state from positions — rejected because two players can both pass a threshold in the same game without either having claimed it (if they both jump over it).

### 5. NDJSON logging via `<fstream>` + nlohmann/json or hand-rolled

The codebase has no JSON dependency. Two options: (a) add `nlohmann/json` as a Meson subproject, (b) hand-roll the small fixed-schema log lines. The log schema is simple and stable (four event types, fixed fields). Hand-rolled formatting keeps the dependency graph minimal and is consistent with the existing codebase style. If the schema grows, a proper JSON library can be added then.

### 6. Random agent as a free function, not a class

`random_move(const SimplifiedGameState&, std::mt19937&) → Move` is a pure function parameterised by an rng. This is simpler than a class hierarchy at this stage; a polymorphic `Agent` interface can be introduced when a second agent type is added.

### 7. Play driver as a standalone `main` in `src/`

A small `play_driver.cpp` with its own `main` entry point, compiled as a separate Meson executable target. Input: `--seed <n> --setup <file>` (or positional). Output: NDJSON log to stdout or `--output <file>`. This matches the roadmap's "reproducible random play" goal without coupling game logic to I/O.

## Risks / Trade-offs

- **Threshold flag bits in shared word** — Adding 2 bits to `SimplifiedGameState::shared_` requires checking that the existing bit layout has room. The shared word is `uint64_t`; bits 0–40 are currently used (33 patch + 6 circle + 2 bonus), leaving 23 bits free. Two threshold bits at positions 41–42 fit comfortably. [Risk: bit-layout regression] → Mitigation: static_assert on field positions, existing round-trip tests catch regressions.
- **Score overflow** — `free_spaces` max is 81, so worst-case score penalty is 162. `buttons` max visible in the test is 127. Using `int` throughout is safe. [Risk: none practical]
- **NDJSON log size** — Each move line is ~200 bytes; a full game is ~50–100 moves → ~20 KB per game. Not a concern.
- **Simplified vs. full rules divergence** — This implementation intentionally ignores spatial placement. The 7×7 bonus uses `free_spaces` as a proxy (56+ cells occupied), which is correct under the simplification. When spatial rules are added later, the bonus check will need to inspect the actual board, not `free_spaces`. [Risk: confusion] → Mitigation: document the simplification in the spec and in code comments.

## Open Questions

- Should `apply_move` assert on legality, or return an `expected<SimplifiedGameState, Error>`? Starting with assert + documented precondition; can relax later.
- Does the play driver need a `--count` flag to play multiple games in one invocation? Deferred — out of scope for this phase but easy to add.
