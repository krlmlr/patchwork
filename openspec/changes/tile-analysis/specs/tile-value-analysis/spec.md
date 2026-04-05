## ADDED Requirements

### Requirement: Shape features are extracted for every patch
The analysis script SHALL read `data/patches.yaml` and compute the following shape features for each of the 33 patches: cell count (number of `X` cells), bounding-box rows, bounding-box columns, bounding-box area (rows × columns), density (cell count / bounding-box area), and perimeter (number of exposed cell edges — edges adjacent to `.` cells or outside the bounding box).

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

### Requirement: Gain-per-time model is computed for every patch
The analysis script SHALL compute, for each patch, a base gain-per-time metric defined as `income / time`. Patches with `time == 0` SHALL be excluded from this metric (no such patches exist in the current catalog, but the script SHALL handle this gracefully).

#### Scenario: Gain-per-time computed for all income-earning patches
- **WHEN** the analysis script is executed
- **THEN** every patch with `income > 0` has a computed `gain_per_time` value equal to `income / time`

#### Scenario: Zero-income patches have gain-per-time of zero
- **WHEN** the analysis script is executed
- **THEN** every patch with `income == 0` has `gain_per_time` equal to 0

### Requirement: Time-position-dependent value is computed for every patch and position
The analysis script SHALL compute `total_gain(patch, pos)` for each patch and each time-track position `pos` in 0–53. `total_gain` is defined as `income * reachable_income_phases(pos)`, where `reachable_income_phases(pos)` counts how many of the five button-income thresholds (time positions 9, 18, 27, 36, 45) are strictly greater than `pos`. A normalised value `value_per_time(patch, pos) = total_gain(patch, pos) / time` SHALL also be computed.

#### Scenario: Reachable income phases decrease monotonically
- **WHEN** `reachable_income_phases(pos)` is evaluated for pos = 0, 9, 18, 27, 36, 45, 53
- **THEN** the values are 5, 4, 3, 2, 1, 0, 0 respectively

#### Scenario: Total gain is zero at or after last income threshold
- **WHEN** `total_gain` is computed for any patch with `income > 0` at `pos >= 45`
- **THEN** `total_gain` equals 0

#### Scenario: Value curve is non-increasing over time positions
- **WHEN** `value_per_time(patch, pos)` is evaluated for any patch over all positions 0–53
- **THEN** the sequence is non-increasing (each value is less than or equal to the previous)

### Requirement: Summary table is produced and committed
The analysis script SHALL write a summary table to `analysis/output/tile_summary.csv` containing, for each patch: `id`, `name`, `buttons`, `time`, `income`, `cells`, `bbox_rows`, `bbox_cols`, `density`, `perimeter`, `gain_per_time`, and `total_gain_at_pos0` (total gain when starting from position 0).

#### Scenario: Summary CSV is produced
- **WHEN** the analysis script is executed
- **THEN** `analysis/output/tile_summary.csv` is created or overwritten with exactly 33 data rows and the required column headers

#### Scenario: Summary CSV values are consistent with source data
- **WHEN** any row of `tile_summary.csv` is read
- **THEN** the `buttons`, `time`, and `income` values match the corresponding entry in `data/patches.yaml`

### Requirement: Plots are produced and committed
The analysis script SHALL produce and save the following plots to `analysis/output/`:
- `gain_per_time.png`: bar chart of `gain_per_time` for all 33 patches, sorted descending
- `value_curves.png`: line plot of `value_per_time(patch, pos)` over time-track positions 0–53, with one line per patch that has `income > 0`
- `shape_density.png`: scatter plot of `density` vs. `cells`, with points labelled by patch `name`

#### Scenario: All three plots are produced
- **WHEN** the analysis script is executed
- **THEN** files `gain_per_time.png`, `value_curves.png`, and `shape_density.png` exist under `analysis/output/`

#### Scenario: Plots are non-empty images
- **WHEN** any of the three plot files is opened
- **THEN** it contains a visible chart (file size > 0 bytes)

### Requirement: Analysis script is reproducible
The analysis script at `analysis/tile_analysis.R` SHALL be idempotent: running it multiple times on an unchanged `data/patches.yaml` SHALL produce identical output files. The script SHALL document its R package dependencies in a comment block at the top.

#### Scenario: Re-running produces identical outputs
- **WHEN** `analysis/tile_analysis.R` is executed twice in succession without modifying `data/patches.yaml`
- **THEN** the output CSV and PNG files are byte-for-byte identical between runs (or functionally equivalent for PNG, ignoring metadata timestamps)

#### Scenario: Required packages are documented
- **WHEN** the top of `analysis/tile_analysis.R` is read
- **THEN** a comment block lists all required R packages (at minimum: `yaml`, `ggplot2`, `dplyr`)
