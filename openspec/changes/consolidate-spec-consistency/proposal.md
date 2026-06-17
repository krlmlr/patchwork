## Why

A cross-spec consistency review (one reviewer per domain) surfaced a set of
contradictions between specs, between specs and the code, and within the
glossary. None of them are feature work — they are corrections that keep the
specs trustworthy as the single design record. Several were found independently
by multiple reviewers (e.g. the bonus-tile cell count and the impossible
`winner = -1` draw case), which raises confidence that they are real.

This change consolidates those corrections into one reviewable pass. PR-specific
findings (#13, #26, #27, #28) are explicitly out of scope and will be handled
when those PRs are reviewed.

## What Changes

For each item we decided whether the **spec** or the **code/docs** was wrong:

- **Bonus tile (49 vs 56 cells)** — *both numbers are correct in different rule
  sets.* The simplified rules (current implementation, tile placement ignored)
  claim the bonus at **56 occupied cells**; the final game (with placement)
  claims it for a filled **7×7 area (49 cells)**. The glossary and `game-logic`
  spec conflated the two. Make the distinction crystal-clear in both, and state
  that the 49-cell final-game rule is deferred until piece placement lands.
- **`winner = -1` draw case** — *spec wrong.* The `first_to_finish` tiebreaker
  makes draws structurally impossible; `winner()` only returns 0 or 1. Remove
  the draw case and its scenario from the `engine` `game_end` requirement.
- **`make_setup` divergent source of truth** — *code wrong.* `make_setup(id)`
  builds the circle by rotating `kPatches`, producing a different permutation
  than the canonical `kGameSetups` used by the TUI, so the same `setup_id`
  yields different circles in the driver vs the TUI. Re-point `make_setup` at
  `kGameSetups` and add a `game-core` requirement fixing it as the single
  source.
- **`random_move` / `legal_moves` signatures** — *spec wrong.* Both take a
  `GameSetup` parameter in code; the `agents` spec omits it. Add it.
- **TUI spec drift** — *spec wrong.* `generate_moves` → `legal_moves`;
  `std::mt19937_64` → `std::mt19937`; document the `HistoryEntry.log_entries`
  field; `History` snapshots `SimplifiedGameState`, not `GameState`.
- **Data spec drift** — *spec wrong.* The generated `PatchData` has a
  `num_cells` field the spec omits, and `cells` is a fixed 8-element array; the
  canonical-form "equivalently…" clause contradicts the lexicographic-minimum
  rule the R codegen actually implements; and the "no patch data hardcoded in
  C++" rule contradicts itself (the committed generated header is C++ holding
  patch data) — restate it as "generated from the YAML, never hand-written."
- **Infrastructure** — `mise` PATH is `build/cpp`, not `build` (*spec wrong*);
  `install-tools.sh` invokes `sudo` internally and must be run **without**
  `sudo bash`, but `README.md`/`BUILD.md` say `sudo bash` (*docs wrong*); the
  spec-catalog governance trigger is narrower than the index's own rule
  (*spec wrong*).
- **Glossary enhancement** — add canonical entries for terms used by *merged*
  specs that are missing: **free spaces** and **button-income space** (payout
  space), plus the corrected **bonus tile** entry.

## Capabilities

### New Capabilities

- (none)

### Modified Capabilities

- `game-logic`: bonus-tile requirement now distinguishes the simplified-rules
  56-cell threshold from the final-game 49-cell 7×7 area.
- `engine`: `game_end` `winner` field is `0` or `1` only; the draw/`-1` case is
  removed.
- `game-core`: `make_setup` is required to source circles from canonical
  `kGameSetups` (single source of truth).
- `agents`: `random_move` and `legal_moves` signatures include the `GameSetup`
  parameter.
- `tui`: correct `legal_moves` name, `std::mt19937` RNG type, documented
  `HistoryEntry.log_entries`, and `SimplifiedGameState` history snapshots.
- `data`: `PatchData` field list includes `num_cells` and fixed-size `cells`;
  canonical-form rule restated to match the implemented lexicographic minimum.
- `infrastructure`: `mise` PATH `build/cpp`; catalog-governance trigger aligned
  with the index's own maintenance rule.

## Impact

- **Code**: `cpp/game_setups.hpp` (`make_setup` delegates to `kGameSetups`).
- **Docs**: `README.md`, `BUILD.md` (drop `sudo` from the install invocation);
  `docs/glossary.md` (bonus tile, free spaces, button-income space).
- **Specs**: delta files for the seven domains above.
- **Tests**: add a test asserting `make_setup(id)` equals
  `kGameSetups[id % kNumGameSetups]` so the driver and TUI can no longer
  diverge.
- **Out of scope**: open PRs #13/#26/#27/#28 and any terms they introduce
  (advance weight, biased random agent, board value, circumference, corners…).
