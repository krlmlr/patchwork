## ADDED Requirements

### Requirement: Fairness analysis ingests batch summary logs via DuckDB

The fairness analysis script at `analysis/fairness_analysis.R` SHALL call `pkgload::load_all()` at startup, read a batch `game_summary` NDJSON file into DuckDB, and expose a per-game table with columns `setup_id`, `seed`, `score_p0`, `score_p1`, `winner`, `plies`, and a derived `margin` = `score_p1 − score_p0`. The script SHALL document any R package dependencies beyond `DESCRIPTION` (at minimum `duckdb`) in a top-of-file comment block.

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
