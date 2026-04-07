## ADDED Requirements

### Requirement: Script runs as regular user, sudo used only where needed
`install-tools.sh` SHALL be invoked as a regular user (without `sudo bash`). Commands requiring root privileges (apt-get, rig binary extraction to /usr/local, pipx/npm global installs) SHALL use `sudo` inline. `rig` commands SHALL be invoked without `sudo` as rig manages its own privilege escalation internally.

### Requirement: rig user library and pak are set up for the current user
After installing R via `rig add release`, `install-tools.sh` SHALL call `rig system setup-user-lib` and `rig system add-pak` (without sudo) so that pak is installed into the current user's R library and is accessible to subsequent `Rscript` calls running as the same user.

### Requirement: PPM binary repository is configured for all R package installs
`install-tools.sh` SHALL write a site-wide `Rprofile.site` entry that sets the necessary options for the PPM binary CRAN mirror for Ubuntu 24.04 (`noble`) (perhaps `repos` **and** the `HTTPUserAgent` string that PPM requires to serve Linux binaries), before any R package installation occurs.

### Requirement: Binary installation is verified by a smoke test immediately after R setup
The DBI binary-install smoke test SHALL be placed immediately after `rig system add-pak` and PPM configuration (not in the final smoke-test section) to fail fast. The test SHALL install `DBI` using `pak::pkg_install("DBI", ask = FALSE)` and assert that no install from source was invoked, confirming PPM served a pre-built binary.

#### Scenario: DBI installs as binary without source compilation
- **WHEN** `install-tools.sh` runs the DBI smoke test on Ubuntu 24.04 with PPM configured
- **THEN** `pak::pkg_install("DBI", ask = FALSE)` completes without printing any reference to source install, and DBI appears in the installed packages list

#### Scenario: pak uses PPM when installing packages
- **WHEN** `Rscript -e 'pak::pak()'` is executed after `install-tools.sh`
- **THEN** packages are downloaded as pre-built binaries (no compilation from source for packages with available binaries)
