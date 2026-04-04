## 1. Project Structure

- [ ] 1.1 Create `analysis/` directory at the repository root
- [ ] 1.2 Create `analysis/output/` directory (add `.gitkeep` so it is tracked)
- [ ] 1.3 Add a brief `analysis/README.md` documenting the purpose of the directory, required R packages, and how to re-run the script

## 2. Shape Feature Extraction

- [ ] 2.1 Write R function `parse_shape(shape_str)` that converts an ASCII-art shape string into a matrix of occupied cell coordinates `(row, col)`
- [ ] 2.2 Write R function `shape_features(cells_df)` that computes `bbox_rows`, `bbox_cols`, `bbox_area`, `density`, and `perimeter` from a cell-coordinate data frame
- [ ] 2.3 Verify `perimeter` calculation: for a 2-cell horizontal patch (`XX`), expected perimeter is 6
- [ ] 2.4 Load `data/patches.yaml` and apply shape extraction to all 33 patches; confirm `cells` column matches `X` count for every entry

## 3. Value Models

- [ ] 3.1 Compute `gain_per_time` for all patches (`income / time`; 0 when `income == 0`)
- [ ] 3.2 Implement `reachable_income_phases(pos)` function using thresholds `c(9, 18, 27, 36, 45)` and verify values at pos = 0, 9, 18, 27, 36, 45, 53
- [ ] 3.3 Compute `total_gain(patch, pos)` for all 33 patches × 54 positions (0–53)
- [ ] 3.4 Compute `value_per_time(patch, pos) = total_gain / time` and verify the sequence is non-increasing for each patch over positions 0–53

## 4. Summary Table

- [ ] 4.1 Assemble the summary data frame with columns: `id`, `name`, `buttons`, `time`, `income`, `cells`, `bbox_rows`, `bbox_cols`, `density`, `perimeter`, `gain_per_time`, `total_gain_at_pos0`
- [ ] 4.2 Write the data frame to `analysis/output/tile_summary.csv` (33 data rows, header row)
- [ ] 4.3 Spot-check three patches: verify `buttons`, `time`, `income` match `data/patches.yaml`

## 5. Plots

- [ ] 5.1 Create `gain_per_time.png`: bar chart of `gain_per_time` for all 33 patches, sorted descending, saved to `analysis/output/`
- [ ] 5.2 Create `value_curves.png`: line plot of `value_per_time` over positions 0–53, one line per income-earning patch, saved to `analysis/output/`
- [ ] 5.3 Create `shape_density.png`: scatter plot of `density` vs. `cells` with patch `name` labels, saved to `analysis/output/`
- [ ] 5.4 Confirm all three PNG files are non-empty (file size > 0)

## 6. Script Packaging and Reproducibility

- [ ] 6.1 Consolidate all steps into `analysis/tile_analysis.R` with a comment block at the top listing required packages (`yaml`, `ggplot2`, `dplyr`) and a brief description
- [ ] 6.2 Ensure the script is idempotent: run it twice and confirm output files are identical
- [ ] 6.3 Commit `analysis/tile_analysis.R`, `analysis/output/tile_summary.csv`, and the three PNG plots

## 7. Verification

- [ ] 7.1 Run `analysis/tile_analysis.R` from a clean working directory and confirm no errors
- [ ] 7.2 Verify `tile_summary.csv` has exactly 33 rows (plus header)
- [ ] 7.3 Confirm the existing `codegen/` scripts and C++ build are unaffected (run `meson compile` or equivalent)
