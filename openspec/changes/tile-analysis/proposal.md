## Why

The patch catalog is the foundation of every game decision, but no quantitative analysis of patch value exists yet. Before building heuristic or search agents, we need data-driven models of patch gain per time cost and shape efficiency to inform their design.

## What Changes

- New R analysis scripts that read `data/patches.yaml` and compute value metrics for each of the 33 patches
- Shape feature extraction (cell count, perimeter, area-to-perimeter ratio, bounding-box density)
- Patch gain model: placement gain (2 × cells covered − button cost) plus projected income (button income × remaining payouts), normalised by time cost, with variants across time-track positions
- Summary plots and tables (patch rankings, metric distributions, gain curves over time-track positions)
- All analysis outputs committed as static artifacts for reference during agent development

## Capabilities

### New Capabilities

- `tile-value-analysis`: R-based analysis of patch gain per time cost, shape features, and time-position-dependent patch gain curves, producing committed plots and tables

### Modified Capabilities

*(none — no existing spec-level behavior changes)*

## Impact

- New R scripts under `analysis/`; scripts reuse shape utilities from the project R package (`R/patches.R`) via `pkgload::load_all()`
- New output artifacts (plots as PNG, summary table as CSV) committed to `analysis/output/`
- No C++ changes; no changes to `data/patches.yaml` or `cpp/generated/` headers
- Directly informs the design of the upcoming Heuristic & Search Agents phase
