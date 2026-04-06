## Context

R packages are currently installed from CRAN source in every CI run with no caching. On Ubuntu 24.04, source compilation can take several minutes per package. The project uses `pak` as its package manager (invoked via `Rscript -e 'pak::pak()'`) but `pak` itself is not yet installed before that call — it relies on a bootstrap that CRAN must serve. PPM (Posit Package Manager) provides pre-compiled binary packages for Ubuntu 24.04, reducing installation to a simple file copy. The three environments to fix are: GitHub Actions CI (`ci.yml`), GitHub Copilot setup (`copilot-setup-steps.yml`), and the devcontainer (`devcontainer.json` → `install-tools.sh`).

## Goals / Non-Goals

**Goals:**
- Point `pak` at PPM binaries for Ubuntu 24.04 so packages install in seconds, not minutes
- Cache the R package library in GHA workflows so unchanged dependencies cost nothing on repeat runs
- Apply the same PPM configuration uniformly in `install-tools.sh` so devcontainer benefits automatically
- Keep `pak::pak()` as the install command — no workflow restructuring

**Non-Goals:**
- Switching away from `pak` or `rig` as the R toolchain
- Pinning R or package versions (that is a separate reproducibility concern)
- Adding R package caching to the devcontainer (devcontainer rebuild is the existing mechanism)

## Decisions

### Use PPM binary snapshot for Ubuntu 24.04

PPM exposes a CRAN-compatible URL with pre-built binaries: `https://packagemanager.posit.co/cran/__linux__/noble/latest`. Setting `options(repos = c(PPM = "<url>"))` in a site-wide `Rprofile.site` (written by `install-tools.sh`) makes every subsequent `pak` / `install.packages` call use it without further changes to R scripts.

**Alternative considered**: setting `RENV_CONFIG_REPOS_OVERRIDE` or editing each workflow step inline. Rejected — the site-wide Rprofile is the single place that covers all three environments because `install-tools.sh` runs in all of them.

### Cache key based on DESCRIPTION hash

The R package library path is `$(Rscript -e 'cat(.libPaths()[1])')`. Keying the cache on `hashFiles('DESCRIPTION')` means a cache hit whenever dependencies haven't changed, which is the common case in CI.

**Alternative considered**: keying on `renv.lock`. Rejected — the project doesn't use renv; `DESCRIPTION` is the canonical dependency list.

### Write PPM config in install-tools.sh, not in workflows

A single place to maintain. Workflows already call `install-tools.sh`; no inline duplication needed.

## Risks / Trade-offs

- **PPM binary lag** → Posit typically publishes binaries within hours of a CRAN release. For a game engine repo with stable dependencies this is not a concern. Mitigation: `pak` falls back to CRAN source automatically if a binary is unavailable.
- **Cache stale on DESCRIPTION rename** → If the file is renamed or moved the cache key breaks. Mitigation: document the key strategy in a comment in the workflow.
- **Cache size** → R library for yaml + testthat is small (< 50 MB). GHA cache limit is 10 GB; no risk.

## Migration Plan

1. Add PPM `Rprofile.site` write to `scripts/install-tools.sh` before the `pak` install step
2. Add `actions/cache` steps to `ci.yml` (before the install step) and `copilot-setup-steps.yml`
3. No rollback needed — removing the cache step or the Rprofile line restores prior behavior exactly
