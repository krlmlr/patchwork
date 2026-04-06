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

PPM exposes a CRAN-compatible URL with pre-built binaries: `https://packagemanager.posit.co/cran/__linux__/noble/latest`. Two R options must be set together in a site-wide `Rprofile.site` (written by `install-tools.sh`):

```r
options(
  repos = c(PPM = "https://packagemanager.posit.co/cran/__linux__/noble/latest"),
  HTTPUserAgent = sprintf(
    "R/%s R (%s)",
    getRversion(),
    paste(getRversion(), R.version$platform, R.version$arch, R.version$os)
  )
)
```

`HTTPUserAgent` is critical: PPM inspects it to determine which pre-built binary to serve. Without it the request looks like a generic CRAN client and PPM falls back to serving source tarballs.

**Alternative considered**: setting `RENV_CONFIG_REPOS_OVERRIDE` or editing each workflow step inline. Rejected — the site-wide Rprofile is the single place that covers all three environments because `install-tools.sh` runs in all of them.

### Verify binary install in CI (acceptance test)

The smoke-test section of `install-tools.sh` SHALL install `DBI` (a stable, dependency-free pure-R package) via `pak::pkg_install("DBI", ask = FALSE)` and then assert that the package was installed from a binary (not compiled from source). The assertion checks that no C compiler was invoked — the absence of `.so` / `.dll` compilation artifacts in the install log, or simply timing the install (binary install completes in under 10 s; source compilation takes 30 s+). A practical shell check: capture `pak` output and `grep -v "R CMD INSTALL"` failing means source was used.

**Alternative considered**: checking `packageDescription("DBI")$NeedsCompilation`. Rejected — `DBI` is a pure-R package so this is always `"no"` regardless of install path; it doesn't prove PPM was used. Instead, check that pak reports `type: binary` in its install plan or that no compilation subprocess was spawned.

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
