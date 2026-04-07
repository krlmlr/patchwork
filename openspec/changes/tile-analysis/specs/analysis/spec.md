## ADDED Requirements

### Requirement: Shape features are extracted for every patch

The analysis script SHALL call `pkgload::load_all()` at startup to load the project R package utilities, then read `data/patches.yaml` and compute the following shape features for each of the 33 patches: cell count (number of `X` cells), bounding-box rows, bounding-box columns, bounding-box area (rows × columns), density (cell count / bounding-box area), and perimeter (number of exposed cell edges — edges adjacent to `.` cells or outside the bounding box). The script SHALL reuse `parse_cells()` and `count_x()` from `R/patches.R` rather than reimplementing them.

#### Scenario: Shape features computed for all patches

- **WHEN** the analysis script is executed
- **THEN** it produces a data frame with exactly 33 rows, one per patch, containing columns `id`, `name`, `cells`, `bbox_rows`, `bbox_cols`, `bbox_area`, `density`, and `perimeter`

#### Scenario: Cell count matches catalog

- **WHEN** shape features are extracted for any patch
- **THEN** the `cells` value equals the number of `X` characters in that patch's `shape` field in `data/patches.yaml`

#### Scenario: Density is within (0, 1]

- **WHEN** shape features are extracted for any patch
- **THEN** `density` equals `cells / bbox_area` and lies in the range (0, 1]

#### Scenario: Perimeter is a positive integer

- **WHEN** shape features are extracted for any patch
- **THEN** `perimeter` is a positive integer equal to the count of edges of occupied cells that border an empty or out-of-bounds cell

### Requirement: Patch gain model is computed for every patch

The analysis script SHALL compute patch gain for each patch using the definition from the glossary. **Placement gain** = 2 × cells − button cost. **Projected income at position 0** = button income × reachable_payouts(0) = button income × 9 (all nine payout spaces are reachable at the start). **Patch gain at position 0** = placement gain + projected income at position 0. **Patch gain per time cost** = patch gain / time cost. Patches with `time == 0` SHALL be excluded from the per-time-cost metric (no such patches exist in the current catalog, but the script SHALL handle this gracefully).

#### Scenario: Placement gain is correct

- **WHEN** placement gain is computed for any patch
- **THEN** it equals 2 × cells − button cost (may be negative for expensive patches with few cells)

#### Scenario: Patch gain at position 0 is correct

- **WHEN** patch gain is computed for any patch at time-track position 0
- **THEN** it equals placement gain + button income × 9

#### Scenario: Patch gain per time cost computed for all patches

- **WHEN** the analysis script is executed
- **THEN** every patch with `time > 0` has a computed `gain_per_time` value equal to its patch gain at position 0 divided by its time cost

### Requirement: Time-position-dependent patch gain is computed for every patch and position

The analysis script SHALL compute `patch_gain(patch, pos)` for each patch and each time-track position `pos` in 0–53. `patch_gain(patch, pos)` = placement gain + button income × reachable_payouts(pos), where `reachable_payouts(pos)` counts how many of the nine payout spaces (time positions 5, 11, 17, 23, 29, 35, 41, 47, 53) are strictly greater than `pos`. A normalised value `gain_per_time(patch, pos)` = `patch_gain(patch, pos) / time cost` SHALL also be computed.

#### Scenario: Reachable payouts decrease monotonically

- **WHEN** `reachable_payouts(pos)` is evaluated for pos = 0, 5, 11, 17, 23, 29, 35, 41, 47, 53
- **THEN** the values are 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 respectively

#### Scenario: Projected income is zero at or after last payout space

- **WHEN** `patch_gain` is computed for any patch with `button income > 0` at `pos >= 53`
- **THEN** projected income equals 0 and `patch_gain` equals placement gain

#### Scenario: Patch gain curve is non-increasing over time positions

- **WHEN** `patch_gain(patch, pos)` is evaluated for any patch over all positions 0–53
- **THEN** the sequence is non-increasing (each value is less than or equal to the previous)

### Requirement: Summary table is produced and committed

The analysis script SHALL write a summary table to `analysis/output/tile_summary.csv` containing, for each patch: `id`, `name`, `button_cost`, `time_cost`, `button_income`, `cells`, `placement_gain`, `bbox_rows`, `bbox_cols`, `density`, `perimeter`, `gain_per_time` (patch gain at position 0 / time cost), and `patch_gain_at_pos0`.

#### Scenario: Summary CSV is produced

- **WHEN** the analysis script is executed
- **THEN** `analysis/output/tile_summary.csv` is created or overwritten with exactly 33 data rows and the required column headers

#### Scenario: Summary CSV values are consistent with source data

- **WHEN** any row of `tile_summary.csv` is read
- **THEN** the `button_cost`, `time_cost`, and `button_income` values match the `buttons`, `time`, and `income` fields of the corresponding entry in `data/patches.yaml`

### Requirement: Plots are produced and committed

The analysis script SHALL produce and save the following plots to `analysis/output/`:

- `gain_per_time.png`: bar chart of `gain_per_time` (patch gain at pos 0 / time cost) for all 33 patches, sorted descending
- `gain_curves.png`: line plot of `patch_gain(patch, pos)` over time-track positions 0–53, with one line per patch that has `button income > 0`
- `shape_density.png`: scatter plot of `density` vs. `cells`, with points labelled by patch `name`

#### Scenario: All three plots are produced

- **WHEN** the analysis script is executed
- **THEN** files `gain_per_time.png`, `gain_curves.png`, and `shape_density.png` exist under `analysis/output/`

#### Scenario: Plots are non-empty images

- **WHEN** any of the three plot files is opened
- **THEN** it contains a visible chart (file size > 0 bytes)

### Requirement: Analysis script is reproducible

The analysis script at `analysis/tile_analysis.R` SHALL be idempotent: running it multiple times on an unchanged `data/patches.yaml` SHALL produce identical output files. The script SHALL start with `pkgload::load_all()` to load project utilities and SHALL document any additional R package dependencies (beyond those declared in `DESCRIPTION`) in a comment block at the top.

#### Scenario: Re-running produces identical outputs

- **WHEN** `analysis/tile_analysis.R` is executed twice in succession without modifying `data/patches.yaml`
- **THEN** the output CSV and PNG files are byte-for-byte identical between runs (or functionally equivalent for PNG, ignoring metadata timestamps)

#### Scenario: Additional required packages are documented

- **WHEN** the top of `analysis/tile_analysis.R` is read
- **THEN** a comment block lists any R packages required beyond those in `DESCRIPTION` (at minimum: `ggplot2`, `dplyr`)
