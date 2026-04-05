## Context

The patch catalog (`data/patches.yaml`) contains 33 patches, each with `buttons` (placement cost), `time` (time cost), `income` (buttons per income phase), and `shape` (ASCII art). The `codegen/` directory holds existing R scripts that generate C++ headers from this data.

No quantitative analysis of tile value currently exists. The upcoming Heuristic & Search Agents and Reinforcement Learning phases will need value estimates to build evaluation functions. This change produces those estimates as committed R scripts and static artifacts.

## Goals / Non-Goals

**Goals:**
- Compute shape features for every patch (cell count, perimeter, bounding-box dimensions, density)
- Model gain-per-time for each patch across the full range of time-track positions (0–53)
- Produce ranked tables and plots committed to the repository
- Keep analysis reproducible from `data/patches.yaml` alone

**Non-Goals:**
- No C++ code changes
- No changes to `data/patches.yaml` or any generated headers
- No game simulation; analysis is static (catalog-only)
- No integration into the build system (Meson does not need to know about analysis scripts)

## Decisions

### Analysis scripts location: `analysis/` over `codegen/`

`codegen/` is reserved for scripts that produce committed C++ headers. Analysis scripts produce plots and tables, not source code. A separate `analysis/` directory makes the distinction clear and avoids cluttering the codegen pipeline.

*Alternative considered:* Putting scripts in `codegen/` alongside existing R scripts. Rejected because it blurs the codegen/analysis boundary.

### Output artifacts location: `analysis/output/`

Plots (PNG) and tables (CSV/markdown) are committed to `analysis/output/` so they are always accessible without re-running R.

*Alternative considered:* Not committing outputs and generating on demand. Rejected because it requires R to be installed and run before the artifacts are usable as a reference.

### Value model: income-per-time with time-remaining adjustment

The base metric is `income / time`. A time-position-dependent variant multiplies by the fraction of income phases still reachable from a given time-track position: `value(pos) = income * reachable_phases(pos) / time`. This captures the intuition that late-game income patches are worth less.

Income phases occur at time-track positions 9, 18, 27, 36, 45 (the five button markers). `reachable_phases(pos)` counts how many of these thresholds lie at positions > pos.

*Alternative considered:* Discounted future income (NPV-style). Deferred as over-engineered for the current phase; can be revisited when RL baselines are available.

### Shape features: perimeter-based circumference

Perimeter of the occupied region (number of exposed edges of occupied cells) captures how "border-heavy" a shape is, which relates to placement difficulty. Density = cells / (bounding-box rows × bounding-box cols).

## Risks / Trade-offs

- [Risk] Output files become stale if `patches.yaml` is edited → Mitigation: README note that analysis must be re-run after catalog changes; script is idempotent.
- [Risk] R environment differences produce non-reproducible outputs → Mitigation: Pin R package versions in a `renv.lock` if the project adopts renv; for now, document required packages (yaml, ggplot2, dplyr, knitr/kableExtra or gt).
- [Trade-off] Static income-phase model ignores button-cost payback period → Acceptable at this phase; refinement belongs in the Heuristic Agents phase once game logs are available.
