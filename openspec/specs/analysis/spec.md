# Analysis Specification

## Purpose
Defines the offline analysis pipeline: R scripts that read the patch catalog and produce committed plots and tables that quantify every patch's strategic value, guiding both human decision-making and the design of machine agents.

## Requirements

### Requirement: Shape features are extracted for every patch

The analysis script SHALL call `pkgload::load_all()` at startup to load the project R package utilities, then read `data/patches.yaml` and compute the following shape features for each of the 33 patches: cell count (number of `X` cells), bounding-box rows, bounding-box columns, bounding-box area (rows × columns), density (cell count / bounding-box area), perimeter (number of exposed cell edges — edges adjacent to `.` cells or outside the bounding box), circumference (equal to perimeter for grid-cell shapes), convex corners, concave corners, and total corners. The script SHALL reuse `parse_cells()` and `count_x()` from `R/patches.R` rather than reimplementing them.

Circumference equals perimeter for grid-cell shapes: both count exposed cell edges on the boundary. Corners are a vertex-level property: at each grid vertex shared by 1 occupied cell → convex corner; by 3 occupied cells → concave corner; by 2 diagonally opposite occupied cells → 2 convex corners (saddle point). For simply-connected shapes the discrete Gauss-Bonnet theorem gives convex_corners − concave_corners = 4.

#### Scenario: Shape features computed for all patches

- **WHEN** the analysis script is executed
- **THEN** it produces a data frame with exactly 33 rows, one per patch, containing columns `id`, `name`, `cells`, `bbox_rows`, `bbox_cols`, `bbox_area`, `density`, `perimeter`, `circumference`, `convex_corners`, `concave_corners`, and `corners`

#### Scenario: Density is within (0, 1]

- **WHEN** shape features are extracted for any patch
- **THEN** `density` equals `cells / bbox_area` and lies in the range (0, 1]

#### Scenario: Circumference equals perimeter

- **WHEN** shape features are extracted for any patch
- **THEN** `circumference` equals `perimeter`

#### Scenario: Gauss-Bonnet identity holds

- **WHEN** corner counts are computed for any patch
- **THEN** `convex_corners − concave_corners` equals 4 (discrete Gauss-Bonnet for simply-connected shapes)

### Requirement: Patch gain model is computed for every patch at all positions

The analysis script SHALL compute patch gain for each patch at all time-track positions 0–53 and at representative positions for each payout band (0–4, 5–10, 11–16, 17–22, 23–28, 29–34, 35–40, 41–46, 47–52, 53). **Placement gain** = 2 × cells − button cost. **Projected income at pos** = button income × `reachable_payouts(pos)`. **Total gain at pos** = placement gain + projected income at pos. **Gain per time cost at pos** = total gain at pos / time cost. Patches with `time == 0` SHALL be excluded from the per-time-cost metric.

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

The analysis script SHALL write a summary table to `analysis/output/tile_summary.csv` containing, for each patch: `id`, `name`, `button_cost`, `time_cost`, `button_income`, `cells`, `placement_gain`, `bbox_rows`, `bbox_cols`, `density`, `perimeter`, `circumference`, `convex_corners`, `concave_corners`, `corners`, `gain_per_time_pos0`, `gain_per_time_pos18`, `gain_per_time_pos36`, and `advance_breakeven_pos`.

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

- **`gain_per_time.png`** — faceted bar chart of `gain_per_time` for all 33 patches across position bands (0–4, 5–10, …, 53), sorted descending within each facet; bars filled by `button_income` (0–3); dashed reference line at y = 1.0 (advance threshold); y-axis uses whole-number breaks.
- **`gain_curves.png`** — line plot of `gain_per_time(patch, pos)` over time-track positions 0–53, one curve per patch with `button_income > 0`, with `geom_text_repel()` labels showing colored patch name symbols at each position. Horizontal reference line at y = 1.0; y-axis uses whole-number breaks.
- **`gain_heatmap.png`** — filled-tile heatmap with patches on the x-axis (sorted by `gain_per_time_pos0` descending) and time-track position (0–53) on the y-axis; fill colour encodes `gain_per_time(patch, pos)`, with a diverging palette centred at 1.0.
- **`shape_density.png`** — scatter plot of `density` vs. `cells`, with points labelled by patch `name`.
- **`total_gain.png`** — faceted bar chart of total gain (not normalised by time) across position bands, sorted descending within each facet; bars filled by `button_income`; y-axis uses whole-number breaks.
- **`shape_corners.png`** — faceted scatter plot of convex and concave corner counts vs. cell count, faceted by corner type; annotated with the Gauss-Bonnet identity.
- **`circumference_vs_cells.png`** — scatter plot of perimeter (= circumference) vs. cell count, with point size encoding total corners.

#### Scenario: All seven plots are produced

- **WHEN** the analysis script is executed
- **THEN** files `gain_per_time.png`, `gain_curves.png`, `gain_heatmap.png`, `shape_density.png`, `total_gain.png`, `shape_corners.png`, and `circumference_vs_cells.png` exist under `analysis/output/`

#### Scenario: Gain heatmap encodes the full position range

- **WHEN** `gain_heatmap.png` is inspected
- **THEN** the y-axis covers positions 0–53 and all 33 patches appear on the x-axis

### Requirement: Analysis script is reproducible

The analysis script at `analysis/tile_analysis.R` SHALL be idempotent: running it multiple times on an unchanged `data/patches.yaml` SHALL produce identical output files. The script SHALL start with `pkgload::load_all()` to load project utilities and SHALL document any additional R package dependencies (beyond those declared in `DESCRIPTION`) in a comment block at the top.

#### Scenario: Additional required packages are documented

- **WHEN** the top of `analysis/tile_analysis.R` is read
- **THEN** a comment block lists any R packages required beyond those in `DESCRIPTION` (at minimum: `ggplot2`, `dplyr`)
