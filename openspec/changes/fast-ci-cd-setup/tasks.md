## 1. Configure PPM in install-tools.sh

- [ ] 1.1 Add a step in `scripts/install-tools.sh` (after `rig add release` installs R) that writes a site-wide `Rprofile.site` with two options: `repos` pointing to `https://packagemanager.posit.co/cran/__linux__/noble/latest` and `HTTPUserAgent` set to `sprintf("R/%s R (%s)", getRversion(), paste(getRversion(), R.version$platform, R.version$arch, R.version$os))` — both are required for PPM to serve Linux binaries
- [ ] 1.2 Verify the smoke-test section of `install-tools.sh` prints the configured repo URL and HTTPUserAgent (e.g., `Rscript -e 'cat(getOption("repos"), "\n"); cat(getOption("HTTPUserAgent"), "\n")'`)
- [ ] 1.3 Add a binary-install smoke test in `install-tools.sh`: run `Rscript -e 'pak::pkg_install("DBI", ask = FALSE)'` and confirm the output contains no `gcc`/`g++`/`R CMD INSTALL` lines (pipe to a grep assertion), proving PPM served a pre-built binary

## 2. Cache R packages in CI workflow

- [ ] 2.1 In `.github/workflows/ci.yml`, add an `actions/cache` step before "Install R package dependencies" that caches the R library path (`$(Rscript -e 'cat(.libPaths()[1])')`) keyed on `${{ runner.os }}-r-${{ hashFiles('DESCRIPTION') }}`
- [ ] 2.2 Confirm the cache step uses `restore-keys` fallback (e.g., `${{ runner.os }}-r-`) so a partial cache hit still accelerates installs

## 3. Cache R packages in Copilot setup workflow

- [ ] 3.1 In `.github/workflows/copilot-setup-steps.yml`, add the identical `actions/cache` step (same key pattern and library path) before "Install R package dependencies"

## 4. Validation

- [ ] 4.1 Trigger a CI run and confirm the "Install R package dependencies" step completes in under 60 seconds on a cold cache
- [ ] 4.2 Trigger a second CI run with no `DESCRIPTION` change and confirm the cache step reports a hit and the install step is near-instant
