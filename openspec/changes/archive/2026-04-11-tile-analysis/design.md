## Context

The patch catalog (`data/patches.yaml`) contains 33 patches, each with `buttons` (button cost), `time` (time cost), `income` (button income per payout), and `shape` (ASCII art). The project's R package (`R/`) holds shared utilities — including `parse_cells()`, `count_x()`, and shape-manipulation functions — loaded via `pkgload::load_all()`. The `codegen/` scripts are thin wrappers that call those utilities to generate `cpp/generated/patches.hpp`.

No quantitative analysis of patch value currently exists. The upcoming Heuristic & Search Agents and Reinforcement Learning phases will need value estimates to build evaluation functions. This change produces those estimates as committed R scripts and static artifacts.

## Goals / Non-Goals

**Goals:**
- Compute shape features for every patch (cell count, perimeter, bounding-box dimensions, density)
- Model patch gain per time cost across the full range of time-track positions (0–53)
- Produce ranked tables and plots committed to the repository
- Keep analysis reproducible from `data/patches.yaml` alone

**Non-Goals:**
- No C++ code changes
- No changes to `data/patches.yaml` or any generated headers
- No game simulation; analysis is static (catalog-only)
- No integration into the build system (Meson does not need to know about analysis scripts)

## Decisions

### Analysis scripts location: `analysis/` over `codegen/`

`codegen/` is reserved for scripts that produce committed C++ headers (currently thin wrappers over `R/` utilities). Analysis scripts produce plots and tables, not source code. A separate `analysis/` directory makes the distinction clear and avoids cluttering the codegen pipeline. Analysis scripts call `pkgload::load_all()` at startup to access the project R package utilities in `R/` (including `parse_cells()` and `count_x()`), so shape parsing does not need to be reimplemented.

*Alternative considered:* Putting scripts in `codegen/` alongside existing R scripts. Rejected because it blurs the codegen/analysis boundary.

### Output artifacts: two tables and four plots

The analysis script produces:

- `analysis/output/tile_summary.csv` — one row per patch with base metrics plus `gain_per_time` at three positions (0, 18, 36) and `advance_breakeven_pos`. Covers the full picture in one file.
- `analysis/output/advance_breakeven.csv` — sorted by `advance_breakeven_pos` descending; a compact decision table showing which patches remain worth buying and until when.
- `analysis/output/gain_per_time.png` — faceted bar chart of `gain_per_time` across position bands (0–4, 5–10, …, 53), fill by income (0–3), with advance threshold line at 1.0 and whole-number y-axis breaks.
- `analysis/output/gain_curves.png` — `gain_per_time(patch, pos)` line plot for income-generating patches with colored `geom_text_repel()` name labels, a 1.0 reference line, and whole-number y-axis breaks.
- `analysis/output/gain_heatmap.png` — heatmap of `gain_per_time` with patches on x-axis and positions (0–53) on y-axis, coloured with a diverging palette centred at 1.0.
- `analysis/output/shape_density.png` — density vs. cells scatter for shape-fit considerations.
- `analysis/output/total_gain.png` — faceted bar chart of total gain (not normalised by time) across position bands, fill by income, with whole-number y-axis breaks.
- `analysis/output/shape_corners.png` — faceted scatter of convex and concave corner counts vs. cell count, annotated with the Gauss-Bonnet identity (convex − concave = 4).
- `analysis/output/circumference_vs_cells.png` — scatter of perimeter (= circumference) vs. cells with point size encoding total corners.

### Advance break-even threshold: 1.0

The advance move earns 1 button per time unit (advancing by k spaces earns k buttons). A patch is strictly worth buying when `gain_per_time(patch, pos) > 1.0`. The `advance_breakeven_pos` column captures the latest position where this holds, giving a concrete cutoff for human and agent decision logic.

### Value model: patch-gain-per-time-cost

Patch gain is composed of two parts, as defined in the glossary:
- **placement gain** = 2 × cells − button cost. Placing a patch covers that many quilt board spaces (each worth −2 at scoring) and costs the button cost in buttons (each worth +1 at scoring).
- **projected income** = button income × reachable_payouts(pos). The buttons the patch yields over the rest of the game, where `reachable_payouts(pos)` counts the payout spaces strictly ahead of the current time-track position.

Total patch gain at position `pos` = placement gain + button income × reachable_payouts(pos). Patch gain per time cost = patch gain / time cost.

Payout spaces on the time track are at positions 5, 11, 17, 23, 29, 35, 41, 47, and 53 (nine spaces, as defined in the move-application spec). `reachable_payouts(pos)` counts how many of these positions are strictly greater than `pos`.

*Alternative considered:* Discounted future income (NPV-style). Deferred as over-engineered for the current phase; can be revisited when RL baselines are available.

### Shape features: perimeter, circumference, and corners

Perimeter of the occupied region (number of exposed edges of occupied cells) captures how "border-heavy" a shape is, which relates to placement difficulty. **Circumference** is identical to perimeter for grid-cell shapes — both count the same set of exposed cell edges along the boundary. **Corners** are a vertex-level property counted at grid vertices: a convex corner occurs at a vertex shared by exactly 1 occupied cell; a concave corner at a vertex shared by 3 occupied cells; at a saddle point (2 diagonally opposite occupied cells) each vertex contributes 2 convex corners. For simply-connected shapes the discrete Gauss-Bonnet theorem guarantees `convex_corners − concave_corners = 4`, validated as a script assertion. The relationship between the three variables: circumference = perimeter (edge-based boundary length), while corners characterise vertex-level boundary complexity. Density = cells / (bounding-box rows × bounding-box cols).

## Risks / Trade-offs

- [Risk] Output files become stale if `patches.yaml` is edited → Mitigation: README note that analysis must be re-run after catalog changes; script is idempotent.
- [Risk] R environment differences produce non-reproducible outputs → Mitigation: The project `DESCRIPTION` already declares `yaml` as a dependency; `ggplot2` and `dplyr` must be installed separately. Document this in `analysis/README.md`.
- [Trade-off] Static gain model ignores opponent state and placement context → Acceptable at this phase; refinement belongs in the Heuristic Agents phase once game logs are available.
