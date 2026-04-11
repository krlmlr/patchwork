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
| `tile_summary.csv` | One row per patch with shape features, gain metrics at positions 0/18/36, and advance break-even position |
| `advance_breakeven.csv` | Compact decision table sorted by `advance_breakeven_pos` descending (`NA` last) |
| `gain_per_time.png` | Bar chart of `gain_per_time` at position 0, coloured by whether the patch beats the advance threshold (≥ 1.0) |
| `gain_curves.png` | Line plot of `gain_per_time(pos)` over positions 0–53 for income-generating patches |
| `gain_heatmap.png` | Heatmap of `gain_per_time(pos)` for all 33 patches × 54 positions |
| `shape_density.png` | Scatter plot of `density` vs. `cells` with patch labels |

## Notes

- Re-run the script after any changes to `data/patches.yaml` to keep outputs current.
- No integration with the Meson build system is required; the script is standalone.
