## Why

R package installation is slow in all three environments (GHA CI, Copilot setup steps, and devcontainer) because packages are sourced from CRAN without binary pre-builds and are never cached between runs. This bottleneck slows developer feedback loops and wastes CI minutes on every run.

## What Changes

- Configure `pak` to install R packages from Posit Package Manager (PPM) pre-built binaries for Ubuntu 24.04 instead of compiling from source
- Add caching of the R package library in the GHA CI workflow and Copilot setup workflow to avoid redundant reinstalls on unchanged dependencies
- Update the devcontainer to use PPM for initial R package installation during `postCreateCommand`

## Capabilities

### New Capabilities

- `r-ppm-install`: Configure R toolchain (pak + PPM) for fast binary package installation across all environments (GHA, Copilot, devcontainer)

### Modified Capabilities

<!-- No existing spec-level requirements are changing. -->

## Impact

- `scripts/install-tools.sh`: Add PPM repository configuration for Ubuntu 24.04 so `pak` uses binaries
- `.github/workflows/ci.yml`: Add `actions/cache` step for the R library path, keyed on `DESCRIPTION` hash
- `.github/workflows/copilot-setup-steps.yml`: Same PPM + caching treatment
- `.devcontainer/devcontainer.json`: No structural change; PPM config flows through the shared install script
- No changes to game logic, data, or generated code
