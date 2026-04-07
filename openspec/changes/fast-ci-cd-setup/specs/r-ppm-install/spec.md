## ADDED Requirements

### Requirement: rig user library and pak are set up system-wide
After installing R via `rig add release`, `install-tools.sh` SHALL call `rig system setup-user-lib` and `rig system add-pak` so that pak is available in the system R library (accessible when the script runs as root via `sudo`) before any `Rscript -e 'pak::...'` call is made.

### Requirement: PPM binary repository is configured for all R package installs
`install-tools.sh` SHALL write a site-wide `Rprofile.site` entry that sets the necessary options for the PPM binary CRAN mirror for Ubuntu 24.04 (`noble`) (perhaps `repos` **and** the `HTTPUserAgent` string that PPM requires to serve Linux binaries), before any R package installation occurs.

### Requirement: Binary installation is verified by a smoke test
The smoke-test section of `install-tools.sh` SHALL install `DBI` using `pak::pkg_install("DBI", ask = FALSE)` and assert that no install from source was invoked, confirming PPM served a pre-built binary.

#### Scenario: DBI installs as binary without source compilation
- **WHEN** `install-tools.sh` runs the DBI smoke test on Ubuntu 24.04 with PPM configured
- **THEN** `pak::pkg_install("DBI", ask = FALSE)` completes without printing any reference to source install, and DBI appears in the installed packages list

#### Scenario: pak uses PPM when installing packages
- **WHEN** `Rscript -e 'pak::pak()'` is executed after `install-tools.sh`
- **THEN** packages are downloaded as pre-built binaries (no compilation from source for packages with available binaries)
