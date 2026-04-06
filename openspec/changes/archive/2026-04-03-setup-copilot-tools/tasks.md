## 1. Install Script (shared toolchain definition)

- [x] 1.1 Create `scripts/install-tools.sh` that installs all project prerequisites:
  - `apt-get install -y r-base r-cran-yaml git ca-certificates clang-format nodejs npm`
  - Meson and Ninja via `pipx` (isolated environment, always-latest versions)
  - OpenSpec CLI and `markdownlint-cli2` via `npm install -g`
- [x] 1.2 Make the script executable (`chmod +x`)
- [x] 1.3 Add a smoke-test block at the end of the script that prints the version of each installed tool (`meson --version`, `ninja --version`, `Rscript --version`, `clang-format --version`, `openspec --version`, `markdownlint-cli2 --version`) so failures surface immediately

## 2. Devcontainer Configuration

- [x] 2.1 Create `.devcontainer/devcontainer.json` using `mcr.microsoft.com/devcontainers/cpp:1-ubuntu-24.04` as the base image
- [x] 2.2 Set `postCreateCommand` to `sudo bash scripts/install-tools.sh`
- [x] 2.3 Set `remoteUser` to `vscode` (the default non-root user in the Microsoft C++ image)
- [x] 2.4 Verified the configuration is valid JSON and the image exists on the Microsoft Container Registry

## 3. GitHub Actions Workflows

- [x] 3.1 Create `.github/workflows/ci.yml` with build, test, and R codegen smoke-test steps; toolchain installed via `sudo bash scripts/install-tools.sh`
- [x] 3.2 Create `.github/workflows/copilot-setup-steps.yml` so the Copilot agent provisions the full toolchain before attempting any build
- [x] 3.3 Confirmed both workflows use `scripts/install-tools.sh` as the single toolchain source

## 4. Update BUILD.md

- [x] 4.1 Add "Zero-effort setup" section at the top of `BUILD.md` explaining that opening the repository in GitHub Codespaces, a devcontainer-capable editor, or a GitHub Copilot cloud session provides all prerequisites automatically
- [x] 4.2 Retain existing manual installation instructions as the fallback path, noting `scripts/install-tools.sh` can also be run directly on a plain Ubuntu system

## 5. Additional deliverables

- [x] 5.1 Create `README.md` with quick-start (devcontainer and manual paths), project-structure table, formatting commands, and OpenSpec usage
- [x] 5.2 Create `.clang-format` (Google style, 4-space indent, 100-column limit)
- [x] 5.3 Create `.markdownlint.yml` (standard rules, 100-char line length, code blocks and tables exempt)

## 6. Align with mise spec

- [x] 6.1 Add `[tasks.format]` to `.mise.toml` running `clang-format` on all C++ files in `cpp/`
- [x] 6.2 Add `[tasks.lint]` to `.mise.toml` running `markdownlint-cli2` on all Markdown files
- [x] 6.3 Update `README.md` to reference `mise run setup`, `mise run test`, `mise run format`, `mise run lint` as the primary invocations
- [x] 6.4 Update `BUILD.md` to reference `mise run <task>` as the primary invocation for every documented action (raw commands moved to `<details>` blocks)

## 7. Fix CI

- [x] 7.1 Add `catch2` to the `apt-get install` list in `scripts/install-tools.sh` so the Meson build can resolve the `catch2-with-main` dependency
