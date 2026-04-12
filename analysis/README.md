# Analysis

This directory contains R scripts that analyse the Patchwork patch catalog and produce committed reference artifacts (plots and tables).

## Purpose

The scripts compute shape features and gain metrics for all 33 patches based solely on `data/patches.yaml`. The resulting tables and plots are committed to the repository so they are available without re-running R.

## Required R Packages

Packages declared in `DESCRIPTION` are loaded via `pkgload::load_all()`. The following additional packages must be installed separately before running the script:

- `ggplot2` — plots
- `dplyr` — data manipulation
- `ggrepel` — non-overlapping text labels in scatter plots

Install them from the R console:

```r
install.packages(c("ggplot2", "dplyr", "ggrepel"))
```

## How to Re-run

From the repository root:

```r
source("analysis/tile_analysis.R")
```

Or from a shell:

```sh
Rscript analysis/tile_analysis.R
```

The script is idempotent: re-running it with an unchanged `data/patches.yaml` produces identical output files.

## Outputs

All outputs are written to `analysis/output/`:

| File | Description |
|------|-------------|
| `tile_summary.csv` | One row per patch with shape features (incl. circumference, corners), gain metrics at positions 0/18/36, and advance break-even position |
| `advance_breakeven.csv` | Compact decision table sorted by `advance_breakeven_pos` descending (`NA` last) |
| `gain_per_time.png` | Faceted bar chart of `gain_per_time` across position bands, filled by income (0–3) |
| `gain_curves.png` | Line plot of `gain_per_time(pos)` for income patches with colored name labels |
| `gain_heatmap.png` | Heatmap of `gain_per_time(pos)` — patches on x-axis, positions on y-axis |
| `shape_density.png` | Scatter plot of `density` vs. `cells` with patch labels |
| `total_gain.png` | Faceted bar chart of total gain (not normalised by time) across position bands |
| `shape_corners.png` | Faceted scatter of convex/concave corners vs. cells (Gauss-Bonnet identity) |
| `circumference_vs_cells.png` | Perimeter (= circumference) vs. cells, point size = total corners |

## Notes

- Re-run the script after any changes to `data/patches.yaml` to keep outputs current.
- No integration with the Meson build system is required; the script is standalone.
