## 1. Install Script (shared toolchain definition)

- [x] 1.1 Create `scripts/install-tools.sh` that installs all project prerequisites:
  - `apt-get update && apt-get install -y r-base r-cran-yaml`
  - `pip3 install --upgrade meson ninja`
- [x] 1.2 Make the script executable (`chmod +x`)
- [x] 1.3 Add a smoke-test block at the end of the script that prints the version of each installed tool (`meson --version`, `ninja --version`, `Rscript --version`) so failures surface immediately

## 2. Devcontainer Configuration

- [x] 2.1 Create `.devcontainer/devcontainer.json` using `mcr.microsoft.com/devcontainers/cpp` as the base image (pin to an Ubuntu 24.04 LTS tag)
- [x] 2.2 Set `postCreateCommand` to run `scripts/install-tools.sh`
- [x] 2.3 Set `remoteUser` to `vscode` (the default non-root user in the Microsoft C++ image)
- [x] 2.4 Verify the configuration is valid JSON and that the image name and tag exist on the Microsoft Container Registry

## 3. Update GitHub Actions Workflow (duplication removal)

> **Prerequisite:** The `github-actions-ci` companion change must be implemented and merged before this task is executed.

- [ ] 3.1 In the GitHub Actions workflow introduced by the companion spec, replace any inline toolchain install steps with a call to `scripts/install-tools.sh`
- [ ] 3.2 Confirm the CI job still passes after the refactor (same build and test steps, same outcomes)

## 4. Update BUILD.md

- [x] 4.1 Add a "Zero-effort setup" section at the top of `BUILD.md` explaining that opening the repository in GitHub Codespaces, a devcontainer-capable editor, or a GitHub Copilot cloud session provides all prerequisites automatically via `.devcontainer/devcontainer.json`
- [x] 4.2 Retain the existing manual installation instructions as the fallback path, but note that `scripts/install-tools.sh` can also be run directly on a plain Ubuntu system
