## 1. Project Structure

- [ ] 1.1 Create `analysis/` directory at the repository root
- [ ] 1.2 Create `analysis/output/` directory (add `.gitkeep` so it is tracked)
- [ ] 1.3 Add a brief `analysis/README.md` documenting the purpose of the directory, required R packages, and how to re-run the script
- [ ] 1.4 Update `openspec/specs/README.md` to link `analysis/spec.md` under the Analysis domain

## 2. Shape Feature Extraction

- [ ] 2.1 Call `pkgload::load_all()` to load the project R package; use the existing `parse_cells()` function from `R/patches.R` to convert ASCII-art shape strings to cell-coordinate matrices
- [ ] 2.2 Write R function `shape_features(cells_mat)` that computes `bbox_rows`, `bbox_cols`, `bbox_area`, `density`, and `perimeter` from the cell-coordinate matrix returned by `parse_cells()`
- [ ] 2.3 Verify `perimeter` calculation: for a 2-cell horizontal patch (`XX`), expected perimeter is 6
- [ ] 2.4 Load `data/patches.yaml` and apply shape extraction to all 33 patches; confirm `cells` column matches `count_x()` result for every entry

## 3. Patch Gain Models

- [ ] 3.1 Compute `placement_gain` for all patches (`2 × cells − button_cost`; may be negative)
- [ ] 3.2 Implement `reachable_payouts(pos)` function using payout spaces `c(5, 11, 17, 23, 29, 35, 41, 47, 53)` and verify values at pos = 0, 18, 36, 53 equal 9, 6, 3, 0
- [ ] 3.3 Compute `patch_gain(patch, pos)` for all 33 patches × 54 positions: `placement_gain + button_income × reachable_payouts(pos)`
- [ ] 3.4 Compute `gain_per_time(patch, pos) = patch_gain / time_cost` and verify the sequence is non-increasing for each patch over positions 0–53
- [ ] 3.5 Compute `advance_breakeven_pos` for every patch: the latest time-track position at which `gain_per_time ≥ 1.0`; set `NA` for patches where `gain_per_time < 1.0` at all positions

## 4. Summary Tables

- [ ] 4.1 Assemble the summary data frame with columns: `id`, `name`, `button_cost`, `time_cost`, `button_income`, `cells`, `placement_gain`, `bbox_rows`, `bbox_cols`, `density`, `perimeter`, `gain_per_time_pos0`, `gain_per_time_pos18`, `gain_per_time_pos36`, `advance_breakeven_pos`
- [ ] 4.2 Write the summary data frame to `analysis/output/tile_summary.csv` (33 data rows, header row)
- [ ] 4.3 Assemble the break-even data frame with columns: `id`, `name`, `button_cost`, `time_cost`, `button_income`, `placement_gain`, `gain_per_time_pos0`, `advance_breakeven_pos`; sort by `advance_breakeven_pos` descending, `NA` last
- [ ] 4.4 Write to `analysis/output/advance_breakeven.csv`
- [ ] 4.5 Spot-check three patches: verify `button_cost`, `time_cost`, `button_income` match `data/patches.yaml` and `advance_breakeven_pos` is consistent with the gain model

## 5. Plots

- [ ] 5.1 Create `gain_per_time.png`: bar chart of `gain_per_time_pos0` for all 33 patches, sorted descending; colour bars above 1.0 differently (strong buy vs. subthreshold); save to `analysis/output/`
- [ ] 5.2 Create `gain_curves.png`: line plot of `gain_per_time(patch, pos)` over positions 0–53, one line per patch with `button_income > 0`, with a horizontal reference line at `y = 1.0`; save to `analysis/output/`
- [ ] 5.3 Create `gain_heatmap.png`: filled-tile heatmap — x-axis: time-track position 0–53, y-axis: patches sorted by `gain_per_time_pos0` descending; fill: `gain_per_time(patch, pos)` using a diverging colour palette centred at 1.0; save to `analysis/output/`
- [ ] 5.4 Create `shape_density.png`: scatter plot of `density` vs. `cells` with patch `name` labels; save to `analysis/output/`
- [ ] 5.5 Confirm all four PNG files are non-empty (file size > 0)

## 6. Script Consolidation

- [ ] 6.1 Consolidate all steps into `analysis/tile_analysis.R`; start with `pkgload::load_all()` and add a comment block listing packages beyond `DESCRIPTION` (`ggplot2`, `dplyr`)
- [ ] 6.2 Ensure the script is idempotent: run it twice and confirm output files are identical
- [ ] 6.3 Commit `analysis/tile_analysis.R`, `analysis/output/tile_summary.csv`, `analysis/output/advance_breakeven.csv`, and the four PNG plots

## 7. Verification

- [ ] 7.1 Run `analysis/tile_analysis.R` from a clean working directory and confirm no errors
- [ ] 7.2 Verify `tile_summary.csv` has exactly 33 rows (plus header)
- [ ] 7.3 Verify `advance_breakeven.csv` is sorted correctly and contains no impossible `advance_breakeven_pos` values (all values must be in 0–53 or `NA`)
- [ ] 7.4 Confirm the existing `codegen/` scripts and C++ build are unaffected (run `meson compile` or equivalent)
