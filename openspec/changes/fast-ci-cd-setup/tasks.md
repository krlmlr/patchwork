## 1. Configure PPM in install-tools.sh

- [ ] 1.1 Add a step in `scripts/install-tools.sh` (after `rig add release` installs R) that writes a site-wide `Rprofile.site` with the options necessary, perhaps: `repos` pointing to `https://packagemanager.posit.co/cran/__linux__/noble/latest` and `HTTPUserAgent` set to `sprintf("R/%s R (%s)", getRversion(), paste(getRversion(), R.version$platform, R.version$arch, R.version$os))` — both are required for PPM to serve Linux binaries
- [ ] 1.2 Add a binary-install smoke test in `install-tools.sh`: run `Rscript -e 'pak::pkg_install("DBI", ask = FALSE)'` and confirm one output line matches `DBI.*linux-gnu-ubuntu`, proving PPM served a pre-built binary

## 2. Validation

- [ ] 2.1 Manual: Trigger a CI run and confirm the "Install R package dependencies" step completes faster than before
