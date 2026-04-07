## 1. Configure PPM in install-tools.sh

- [x] 1.0 Rework the script to run as a regular user: add `sudo` inline to apt-get, rig binary install, and pipx/npm global installs; invoke rig commands without sudo (rig handles its own escalation); update all callers (ci.yml, copilot-setup-steps.yml, devcontainer.json) to use `bash scripts/install-tools.sh` instead of `sudo bash`
- [x] 1.1 After `rig add release`, call `rig system setup-user-lib` and `rig system add-pak` (without sudo) so pak is installed into the current user's R library
- [x] 1.2 Add a step in `scripts/install-tools.sh` (after `rig add release` installs R) that writes a site-wide `Rprofile.site` with the options necessary, perhaps: `repos` pointing to `https://packagemanager.posit.co/cran/__linux__/noble/latest` and `HTTPUserAgent` set to `sprintf("R/%s R (%s)", getRversion(), paste(getRversion(), R.version$platform, R.version$arch, R.version$os))` — both are required for PPM to serve Linux binaries
- [x] 1.3 Add a binary-install smoke test immediately after PPM config (not at end): run `Rscript -e 'pak::pkg_install("DBI", ask = FALSE)'` and confirm one output line matches `DBI.*linux-gnu-ubuntu`, proving PPM served a pre-built binary

## 2. Validation

- [ ] 2.1 Manual: Trigger a CI run and confirm the "Install R package dependencies" step completes faster than before
