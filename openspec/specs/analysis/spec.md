# Analysis Specification

## Purpose
Defines the offline analysis pipeline: R scripts that read the patch catalog and produce committed plots and tables that quantify every patch's strategic value, guiding both human decision-making and the design of machine agents.

## Requirements

### Requirement: Shape features are extracted for every patch

The analysis script SHALL call `pkgload::load_all()` at startup to load the project R package utilities, then read `data/patches.yaml` and compute the following shape features for each of the 33 patches: cell count (number of `X` cells), bounding-box rows, bounding-box columns, bounding-box area (rows × columns), density (cell count / bounding-box area), and perimeter (number of exposed cell edges — edges adjacent to `.` cells or outside the bounding box). The script SHALL reuse `parse_cells()` and `count_x()` from `R/patches.R` rather than reimplementing them.

#### Scenario: Shape features computed for all patches

- **WHEN** the analysis script is executed
- **THEN** it produces a data frame with exactly 33 rows, one per patch, containing columns `id`, `name`, `cells`, `bbox_rows`, `bbox_cols`, `bbox_area`, `density`, and `perimeter`

#### Scenario: Density is within (0, 1]

- **WHEN** shape features are extracted for any patch
- **THEN** `density` equals `cells / bbox_area` and lies in the range (0, 1]

### Requirement: Patch gain model is computed for every patch at three key positions

The analysis script SHALL compute patch gain for each patch at three representative time-track positions: `pos = 0` (early), `pos = 18` (mid), `pos = 36` (late), using the definitions from the glossary. **Placement gain** = 2 × cells − button cost. **Projected income at pos** = button income × `reachable_payouts(pos)`. **Patch gain at pos** = placement gain + projected income at pos. **Patch gain per time cost at pos** = patch gain at pos / time cost. Patches with `time == 0` SHALL be excluded from the per-time-cost metric.

`reachable_payouts(pos)` counts how many of the nine payout spaces (time positions 5, 11, 17, 23, 29, 35, 41, 47, 53) are strictly greater than `pos`.

#### Scenario: Reachable payouts at key positions

- **WHEN** `reachable_payouts(pos)` is evaluated for pos = 0, 18, 36, 53
- **THEN** the values are 9, 6, 3, 0 respectively

#### Scenario: Patch gain curves are non-increasing

- **WHEN** `patch_gain(patch, pos)` is evaluated for any patch over all positions 0–53
- **THEN** the sequence is non-increasing (each value is less than or equal to the previous)

### Requirement: Advance break-even position is computed for every patch

For each patch, the analysis script SHALL compute `advance_breakeven_pos`: the latest time-track position at which buying the patch is at least as beneficial as advancing (`gain_per_time(patch, pos) ≥ 1.0`). The threshold of 1.0 corresponds to the advance move's nominal value of 1 button per time unit. For any patch whose `gain_per_time` never rises above 1.0 at any position, `advance_breakeven_pos` is `NA`. This metric directly tells an agent or human player past which point a given patch is no longer worth buying.

#### Scenario: Break-even position matches gain model

- **WHEN** `advance_breakeven_pos` is computed for any patch
- **THEN** `gain_per_time(patch, advance_breakeven_pos) ≥ 1.0` and `gain_per_time(patch, advance_breakeven_pos + 1) < 1.0` (or it is the maximum position if `gain_per_time` never drops below 1.0)

#### Scenario: Patches with zero income and low placement gain have NA break-even

- **WHEN** a patch has `button_income == 0` and `placement_gain < time_cost` (i.e., `gain_per_time < 1.0` at every position)
- **THEN** its `advance_breakeven_pos` is `NA`

### Requirement: Summary table is produced and committed

The analysis script SHALL write a summary table to `analysis/output/tile_summary.csv` containing, for each patch: `id`, `name`, `button_cost`, `time_cost`, `button_income`, `cells`, `placement_gain`, `bbox_rows`, `bbox_cols`, `density`, `perimeter`, `gain_per_time_pos0`, `gain_per_time_pos18`, `gain_per_time_pos36`, and `advance_breakeven_pos`.

#### Scenario: Summary CSV is produced

- **WHEN** the analysis script is executed
- **THEN** `analysis/output/tile_summary.csv` is created or overwritten with exactly 33 data rows and the required column headers

### Requirement: Advance break-even table is produced and committed

The analysis script SHALL write `analysis/output/advance_breakeven.csv` containing, for each patch: `id`, `name`, `button_cost`, `time_cost`, `button_income`, `placement_gain`, `gain_per_time_pos0`, and `advance_breakeven_pos`, sorted by `advance_breakeven_pos` descending (patches useful for longest remain at the top; `NA` entries last). This table is a compact decision aid for both human players and agents.

#### Scenario: Break-even table is sorted correctly

- **WHEN** `advance_breakeven.csv` is read
- **THEN** rows appear in descending order of `advance_breakeven_pos`, with `NA` rows at the bottom

### Requirement: Plots are produced and committed

The analysis script SHALL produce and save the following plots to `analysis/output/`:

- **`gain_per_time.png`** — bar chart of `gain_per_time_pos0` (patch gain at pos 0 / time cost) for all 33 patches, sorted descending; patches above the advance threshold (≥ 1.0) are coloured differently from those below it.
- **`gain_curves.png`** — line plot of `gain_per_time(patch, pos)` over time-track positions 0–53, one curve per patch, with a horizontal reference line at `gain_per_time = 1.0` marking the advance threshold. Only patches with `button_income > 0` are included (their curves decline; zero-income patches are horizontal and plotted separately if desired).
- **`gain_heatmap.png`** — filled-tile heatmap with time-track position (0–53) on the x-axis and patches (sorted by `gain_per_time_pos0` descending) on the y-axis; fill colour encodes `gain_per_time(patch, pos)`, with a diverging palette centred at 1.0 (the advance threshold). This provides a complete strategic overview for agent development.
- **`shape_density.png`** — scatter plot of `density` vs. `cells`, with points labelled by patch `name`; helps identify compact high-coverage patches suitable for tight quilt-board placements.

#### Scenario: All four plots are produced

- **WHEN** the analysis script is executed
- **THEN** files `gain_per_time.png`, `gain_curves.png`, `gain_heatmap.png`, and `shape_density.png` exist under `analysis/output/`

#### Scenario: Gain heatmap encodes the full position range

- **WHEN** `gain_heatmap.png` is inspected
- **THEN** the x-axis covers positions 0–53 and all 33 patches appear on the y-axis

### Requirement: Analysis script is reproducible

The analysis script at `analysis/tile_analysis.R` SHALL be idempotent: running it multiple times on an unchanged `data/patches.yaml` SHALL produce identical output files. The script SHALL start with `pkgload::load_all()` to load project utilities and SHALL document any additional R package dependencies (beyond those declared in `DESCRIPTION`) in a comment block at the top.

#### Scenario: Additional required packages are documented

- **WHEN** the top of `analysis/tile_analysis.R` is read
- **THEN** a comment block lists any R packages required beyond those in `DESCRIPTION` (at minimum: `ggplot2`, `dplyr`)

### Requirement: Fairness analysis ingests batch summary logs via DuckDB

The fairness analysis script at `analysis/fairness_analysis.R` SHALL call `pkgload::load_all()` at startup and ingest a batch `game_summary` NDJSON file into DuckDB (via `duckplyr`), exposing a per-game table with columns `setup_id`, `seed`, `score_p0`, `score_p1`, `winner`, `plies`, and a derived `margin` = `score_p1 − score_p0`. Ingestion SHALL install and load the DuckDB `json` extension, convert the NDJSON to a typed Parquet cache once (rebuilt only when the cache is missing or older than the NDJSON), and read subsequent runs from that cache. Frames SHALL use `duckplyr` prudence `"stingy"` so the per-game rows are never materialised into R; all aggregation SHALL be pushed into DuckDB and only the small result tables collected. The script SHALL document any R package dependencies beyond `DESCRIPTION` (at minimum `duckplyr`) in a top-of-file comment block.

#### Scenario: Summary log is loaded

- **WHEN** the script is run against a batch summary file of `M` games
- **THEN** it produces a per-game table with exactly `M` rows and the columns `setup_id`, `seed`, `score_p0`, `score_p1`, `winner`, `plies`, and `margin`

### Requirement: Fairness analysis reports first-player advantage with a confidence interval

The script SHALL compute the player-1 (P1) win rate over all games together with a binomial confidence interval, and the mean score margin with its confidence interval. These quantify structural first-player advantage: under a fair game both intervals cover 50 % and 0 respectively.

#### Scenario: Win-rate CI is produced

- **WHEN** the fairness analysis runs over a batch
- **THEN** it reports the P1 win rate as a proportion in [0, 1] with lower and upper binomial confidence bounds, and the mean `margin` with a confidence interval

#### Scenario: Balanced batch is not flagged as biased

- **WHEN** the analysis runs on a batch whose P1 win rate is 0.5 exactly
- **THEN** the reported win-rate confidence interval contains 0.5

### Requirement: Fairness analysis decomposes outcome variance into between-setup and within-setup components

The script SHALL group games by `setup_id` and decompose the variance of `margin` into a between-setup component (variance of per-setup mean margins) and a within-setup component (mean of per-setup margin variances), reporting each as a share of the total. A dominant between-setup share indicates the outcome is largely determined by the dealt setup; a dominant within-setup share indicates play (agent RNG) carries the result.

#### Scenario: Variance shares are reported and sum to the total

- **WHEN** the analysis runs on a batch spanning at least two setups with multiple games each
- **THEN** it reports a between-setup and a within-setup variance component whose sum equals the total `margin` variance (within numerical tolerance)

#### Scenario: Single-setup batch reports zero between-setup variance

- **WHEN** the analysis runs on a batch containing exactly one `setup_id`
- **THEN** the between-setup variance component is 0 and the within-setup component equals the total variance

### Requirement: Fairness analysis produces committed per-setup tables and plots

The script SHALL be idempotent and SHALL write, under `analysis/output/`, a per-setup summary table (`fairness_by_setup.csv`: `setup_id`, `n_games`, `p1_win_rate`, win-rate CI bounds, `mean_margin`) and plots covering the P1 win-rate-per-setup spread and the score-margin distribution. Running the script twice on the same batch input SHALL produce identical outputs.

#### Scenario: Per-setup table is written

- **WHEN** the fairness analysis runs
- **THEN** `analysis/output/fairness_by_setup.csv` is created or overwritten with one row per distinct `setup_id` and the required columns

#### Scenario: Plots are written

- **WHEN** the fairness analysis runs
- **THEN** the win-rate-per-setup and score-margin-distribution plot files exist under `analysis/output/`
