## 1. Batch runner core

- [ ] 1.1 Factor the single-game loop out of `cpp/play_driver.cpp` into a reusable function (setup + seed → terminal state + ply count) so both drivers share it
- [ ] 1.2 Add a SplitMix64-based `derive_seed(master_seed, setup_id, game_index)` helper (header + tests) producing order- and size-independent per-game seeds
- [ ] 1.3 Create `cpp/batch_driver.cpp` that parses the setup set, `--games N`, `--master-seed`, `--output`, and `--full` / summary flags
- [ ] 1.4 Implement the in-process `(setup × game_index)` grid loop, seeding each game via `derive_seed` and reusing the shared game loop
- [ ] 1.5 Emit the default `game_summary` NDJSON record (`setup_id`, `seed`, `score_p0`, `score_p1`, `winner`, `plies`); wire `--full` to the existing per-move logger
- [ ] 1.6 Add usage/error handling (missing or non-numeric setups, game count, or `--master-seed` → stderr + non-zero exit)
- [ ] 1.7 Register the `batch_driver` executable target in `cpp/meson.build`

## 2. Batch runner tests (Catch2)

- [ ] 2.1 `derive_seed` is deterministic and order/size-independent for fixed `(master_seed, setup_id, game_index)`
- [ ] 2.2 Full grid produces exactly `setups × N` summary records, each with `winner ∈ {0,1}` and positive `plies`
- [ ] 2.3 Whole batch is byte-for-byte reproducible for a fixed `--master-seed`
- [ ] 2.4 A recorded `(setup_id, seed)` replayed through the single-game play driver yields matching `game_end` scores and winner
- [ ] 2.5 Summary-mode scores/winner equal `--full`-mode `game_end` values for the same game

## 3. R fairness analysis

- [ ] 3.1 Create `analysis/fairness_analysis.R` with `pkgload::load_all()` and a documented dependency comment block (incl. `duckdb`)
- [ ] 3.2 Ingest a `game_summary` NDJSON file into DuckDB; expose a per-game table with derived `margin = score_p1 − score_p0`
- [ ] 3.3 Compute P1 win rate + binomial CI and mean margin + CI (first-player-advantage report)
- [ ] 3.4 Compute between-setup vs within-setup `margin` variance decomposition with shares of total
- [ ] 3.5 Write `analysis/output/fairness_by_setup.csv` (per-setup n, win rate, CI bounds, mean margin)
- [ ] 3.6 Produce win-rate-per-setup and score-margin-distribution plots under `analysis/output/`
- [ ] 3.7 Verify idempotence: re-running on unchanged input reproduces identical outputs

## 4. Docs & specs

- [ ] 4.1 Add batch runner and fairness terms to `docs/glossary.md` if not already covered
- [ ] 4.2 Run `openspec validate batch-fairness-runner` and resolve any issues
- [ ] 4.3 Fold the delta specs into `openspec/specs/engine/spec.md` and `openspec/specs/analysis/spec.md` at archive time (handled by `/opsx:archive`)
