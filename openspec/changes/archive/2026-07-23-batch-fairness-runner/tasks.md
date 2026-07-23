## 1. Batch runner core

- [x] 1.1 Factor the single-game loop out of `cpp/play_driver.cpp` into a reusable function (setup + seed → terminal state + ply count) so both drivers share it
- [x] 1.2 Add a SplitMix64-based `derive_seed(master_seed, setup_id, game_index)` helper (header + tests) producing order- and size-independent per-game seeds
- [x] 1.3 Create `cpp/batch_driver.cpp` that parses the setup set, `--games N`, `--master-seed`, `--output`, and `--full` / summary flags
- [x] 1.4 Implement the in-process `(setup × game_index)` grid loop, seeding each game via `derive_seed` and reusing the shared game loop
- [x] 1.5 Emit the default `game_summary` NDJSON record (`setup_id`, `seed`, `score_p0`, `score_p1`, `winner`, `plies`); wire `--full` to the existing per-move logger
- [x] 1.6 Add usage/error handling (missing or non-numeric setups, game count, or `--master-seed` → stderr + non-zero exit)
- [x] 1.7 Register the `batch_driver` executable target in `cpp/meson.build`
- [x] 1.8 Support both single ids and inclusive ranges, combined, in `--setups` (e.g. `0,2,5-9`) via `parse_setups` in `cpp/setup_range.{hpp,cpp}`

## 2. Batch runner tests (Catch2 + end-to-end)

- [x] 2.1 `derive_seed` is deterministic and order/size-independent for fixed `(master_seed, setup_id, game_index)`
- [x] 2.2 Full grid produces exactly `setups × N` summary records, each with `winner ∈ {0,1}` and positive `plies`
- [x] 2.3 Whole batch is byte-for-byte reproducible for a fixed `--master-seed`
- [x] 2.4 A recorded `(setup_id, seed)` replayed through the single-game play driver yields matching `game_end` scores and winner
- [x] 2.5 Summary-mode scores/winner equal `--full`-mode `game_end` values for the same game
- [x] 2.6 `parse_setups` unit tests: singles, ranges, combined, whitespace, and malformed-input rejection

## 3. R fairness analysis

- [x] 3.1 Create `analysis/fairness_analysis.R` with `pkgload::load_all()` and a documented dependency comment block (incl. `duckdb`)
- [x] 3.2 Ingest a `game_summary` NDJSON file into DuckDB; expose a per-game table with derived `margin = score_p1 − score_p0`
- [x] 3.3 Compute P1 win rate + binomial CI and mean margin + CI (first-player-advantage report)
- [x] 3.4 Compute between-setup vs within-setup `margin` variance decomposition with shares of total
- [x] 3.5 Write `analysis/output/fairness_by_setup.csv` (per-setup n, win rate, CI bounds, mean margin)
- [x] 3.6 Produce win-rate-per-setup and score-margin-distribution plots under `analysis/output/`
- [x] 3.7 Verify idempotence: re-running on unchanged input reproduces identical outputs

## 4. Engine robustness (discovered while running batches)

- [x] 4.1 Fix `legal_moves`: a patch is buyable only if it also fits (`free_spaces >= num_cells`), not just affordable — random play used to buy oversized patches and underflow free spaces
- [x] 4.2 Fix leather-patch award: clamp `free_spaces` at 0 so crossing a threshold on a full board does not underflow
- [x] 4.3 Regression tests for both underflow paths; 50 000-game stress run with assertions enabled completes cleanly

## 5. Docs & specs

- [x] 5.1 Add batch runner and fairness terms to `docs/glossary.md`
- [x] 5.2 Run `openspec validate batch-fairness-runner` and resolve any issues
- [ ] 5.3 Fold the delta specs into `openspec/specs/engine/spec.md` and `openspec/specs/analysis/spec.md` at archive time (handled by `/opsx:archive`)

## 6. Native DuckDB JSON ingest via duckplyr

- [x] 6.1 Confirmed an explicit `INSTALL json` reaches `extensions.duckdb.org` and succeeds in this environment (the earlier failure was autoload not fetching, not a blocked host). Ingest now installs + loads the extension unconditionally.
- [x] 6.2 Switch `analysis/fairness_analysis.R` ingest to native reading via `duckplyr` (`read_json_duckdb` / `read_parquet_duckdb`) with prudence `"stingy"`; push all aggregation into DuckDB and collect only the small result tables. Dropped the `regexp_extract`-over-VARCHAR fallback.
- [x] 6.3 One-time NDJSON → Parquet conversion (cached; rebuilt only when stale) via `duckplyr::compute_parquet`; subsequent runs read the Parquet cache.
- [x] 6.4 Note the `duckplyr` / `ggplot2` analysis dependencies in `DESCRIPTION`; gitignore the Parquet cache.
- [ ] 6.5 Document the DuckDB `json` extension install in the toolchain/setup (devcontainer + `scripts/install-tools.sh` / CI) and pin a persistent `DUCKDB_EXTENSION_DIRECTORY` so a fresh checkout/CI runner installs it once instead of per session.
