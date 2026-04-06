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

### Output artifacts location: `analysis/output/`

Plots (PNG) and tables (CSV/markdown) are committed to `analysis/output/` so they are always accessible without re-running R.

*Alternative considered:* Not committing outputs and generating on demand. Rejected because it requires R to be installed and run before the artifacts are usable as a reference.

### Value model: patch-gain-per-time-cost

Patch gain is composed of two parts, as defined in the glossary:
- **placement gain** = 2 × cells − button cost. Placing a patch covers that many quilt board spaces (each worth −2 at scoring) and costs the button cost in buttons (each worth +1 at scoring).
- **projected income** = button income × reachable_payouts(pos). The buttons the patch yields over the rest of the game, where `reachable_payouts(pos)` counts the payout spaces strictly ahead of the current time-track position.

Total patch gain at position `pos` = placement gain + button income × reachable_payouts(pos). Patch gain per time cost = patch gain / time cost.

Payout spaces on the time track are at positions 5, 11, 17, 23, 29, 35, 41, 47, and 53 (nine spaces, as defined in the move-application spec). `reachable_payouts(pos)` counts how many of these positions are strictly greater than `pos`.

*Alternative considered:* Discounted future income (NPV-style). Deferred as over-engineered for the current phase; can be revisited when RL baselines are available.

### Shape features: perimeter-based circumference

Perimeter of the occupied region (number of exposed edges of occupied cells) captures how "border-heavy" a shape is, which relates to placement difficulty. Density = cells / (bounding-box rows × bounding-box cols).

## Risks / Trade-offs

- [Risk] Output files become stale if `patches.yaml` is edited → Mitigation: README note that analysis must be re-run after catalog changes; script is idempotent.
- [Risk] R environment differences produce non-reproducible outputs → Mitigation: The project `DESCRIPTION` already declares `yaml` as a dependency; `ggplot2` and `dplyr` must be installed separately. Document this in `analysis/README.md`.
- [Trade-off] Static gain model ignores opponent state and placement context → Acceptable at this phase; refinement belongs in the Heuristic Agents phase once game logs are available.
