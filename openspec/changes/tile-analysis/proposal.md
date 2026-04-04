## Why

The patch catalog is the foundation of every game decision, but no quantitative analysis of tile value exists yet. Before building heuristic or search agents, we need data-driven models of gain-per-time and shape efficiency to inform their design.

## What Changes

- New R analysis scripts that read `data/patches.yaml` and compute value metrics for each of the 33 patches
- Shape feature extraction (cell count, circumference/perimeter, area-to-perimeter ratio, bounding-box density)
- Gain-per-time model: income divided by time cost, with variants that account for early-game vs. late-game time-track position
- Summary plots and tables (tile rankings, metric distributions, value curves over time)
- All analysis outputs committed as static artifacts for reference during agent development

## Capabilities

### New Capabilities

- `tile-value-analysis`: R-based analysis of patch gain-per-time, shape features, and time-position-dependent value curves, producing committed plots and tables

### Modified Capabilities

*(none — no existing spec-level behavior changes)*

## Impact

- New R scripts under `analysis/` (or `codegen/` if collocated with existing R work)
- New output artifacts (plots as PNG/PDF, summary tables as CSV or markdown) committed to the repository
- No C++ code changes
- No changes to `data/patches.yaml` or generated headers
- Directly informs the design of the upcoming Heuristic & Search Agents phase
