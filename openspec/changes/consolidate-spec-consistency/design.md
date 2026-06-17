## Context

A per-domain consistency review of the eight OpenSpec specs found contradictions
across specs, between specs and code, and within `docs/glossary.md`. Each finding
was triaged to decide whether the **spec** or the **code/docs** is the source of
truth. This change applies the corrections; it ships no new behaviour.

## Goals / Non-Goals

**Goals:**

- Make every corrected spec match the committed code and the other specs.
- Eliminate the one genuine code defect (`make_setup` divergence) so the driver
  and TUI can no longer produce different circles for the same `setup_id`.
- Clearly separate the simplified-rules 56-cell bonus proxy from the final-game
  49-cell rule in spec and glossary.
- Add glossary entries for terms used by already-merged specs.

**Non-Goals:**

- Open PRs #13, #26, #27, #28 and any terminology they introduce (advance
  weight, biased random agent, board value, projected score, circumference,
  corners, …). Those are reviewed as a follow-up.
- Any change to game behaviour, scoring, or the NDJSON schema's field set.

## Decisions

Per item, which side was wrong and how it is fixed:

1. **Bonus 49 vs 56 — neither wrong.** 56 occupied cells is the simplified-rules
   proxy (placement not modelled); 49 is the final-game 7×7-area rule. Keep the
   code at 56, and make the `game-logic` requirement and the glossary state both
   thresholds explicitly, marking the 49-cell rule as deferred.
2. **`winner = -1` draw — spec wrong.** `winner()` is the `first_to_finish`
   tiebreaker and returns only 0 or 1. Remove the draw case and the "records
   draw" scenario from the `engine` `game_end` requirement.
3. **`make_setup` — code wrong.** It rotates `kPatches`, diverging from the
   canonical `kGameSetups` the TUI uses. Re-point it at
   `kGameSetups[id mod kNumGameSetups]` (header-only, `constexpr`-friendly), and
   add a `game-core` requirement fixing `kGameSetups` as the single source. A
   new test asserts `make_setup(id)` equals `kGameSetups[id mod kNumGameSetups]`.
4. **`random_move` / `legal_moves` signatures — spec wrong.** Both take a
   `const GameSetup&`; the `agents` spec omits it. Add the parameter.
5. **TUI drift — spec wrong.** `generate_moves` → `legal_moves`;
   `std::mt19937_64` → `std::mt19937`; document the `log_entries` snapshot;
   `History` stores `SimplifiedGameState`, not `GameState`. The history
   requirement is renamed so its title is accurate.
6. **Data drift — spec wrong.** `PatchData` has a `num_cells` field and a
   fixed-size `cells` array; the canonical-form scenario's "widest bounding
   box / horizontal preferred" clause contradicts the lexicographic-minimum rule
   the R codegen implements, so it is dropped.
7. **Infrastructure.** mise PATH is `build/cpp`, not `build` — *spec wrong*,
   fix the spec. `install-tools.sh` invokes `sudo` internally and the
   devcontainer runs it as `bash scripts/install-tools.sh`; the spec already
   requires no-`sudo` invocation, so `README.md`/`BUILD.md` (`sudo bash`) are
   *docs wrong* — fix the docs. The catalog-governance trigger in the infra spec
   was narrower than the index's own maintenance rule; align both wordings.
8. **Glossary enhancement.** Add entries for **free spaces** and
   **button-income space** (used by merged specs) and correct the **bonus tile**
   entry. PR-bound terms are excluded.

## Risks / Trade-offs

- **`make_setup` overlaps PR #26**, which also re-points `make_setup` at
  `kGameSetups`. Whoever merges second rebases; the fix here is the minimal,
  spec-backed version and is correct independent of PR #26.
- **Renaming the TUI history requirement** uses the OpenSpec `RENAMED` operation
  (documented in the spec schema but not yet exercised in this repo). If
  `openspec validate`/archive mishandles it, fall back to a `MODIFIED`-only edit
  that keeps the original title.
- These are documentation/consistency edits; behavioural risk is limited to the
  one-line `make_setup` change, covered by the new equality test.
