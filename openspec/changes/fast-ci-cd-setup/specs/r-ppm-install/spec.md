## ADDED Requirements

### Requirement: PPM binary repository is configured for all R package installs
`install-tools.sh` SHALL write a site-wide `Rprofile.site` entry that sets the PPM binary CRAN mirror for Ubuntu 24.04 (`noble`) as the default repository before any R package installation occurs.

#### Scenario: PPM URL is present after install-tools.sh runs
- **WHEN** `install-tools.sh` completes on Ubuntu 24.04
- **THEN** `Rscript -e 'getOption("repos")'` returns a URL containing `packagemanager.posit.co` and `noble`

#### Scenario: pak uses PPM when installing packages
- **WHEN** `Rscript -e 'pak::pak()'` is executed after `install-tools.sh`
- **THEN** packages are downloaded as pre-built binaries (no compilation from source for packages with available binaries)

### Requirement: GHA CI workflow caches the R package library
The `ci.yml` workflow SHALL include an `actions/cache` step for the R package library path, keyed on the hash of `DESCRIPTION`, placed before the R package installation step.

#### Scenario: Cache hit skips package installation network requests
- **WHEN** `DESCRIPTION` has not changed since the last successful CI run
- **THEN** the `actions/cache` step reports a cache hit and the install step completes without downloading packages

#### Scenario: Cache miss triggers fresh install
- **WHEN** `DESCRIPTION` has changed or no cache exists
- **THEN** packages are installed (using PPM binaries) and the result is saved to the cache

### Requirement: Copilot setup workflow caches the R package library
The `copilot-setup-steps.yml` workflow SHALL include the same `actions/cache` configuration as `ci.yml` for the R package library.

#### Scenario: Copilot setup uses cached packages on repeated runs
- **WHEN** the Copilot setup workflow is triggered and `DESCRIPTION` is unchanged
- **THEN** the cache step reports a hit and R packages are not re-downloaded
