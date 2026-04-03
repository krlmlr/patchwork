## Context

The project builds with Meson + Ninja and requires a C++23 compiler and R for codegen. These tools must be present before any build or test command can run. GitHub Copilot coding agents operate in an ephemeral container that is configured by `.devcontainer/devcontainer.json` at repository root (or in `.devcontainer/`). Without this file the agent starts with a bare environment.

This change also delivers GitHub Actions CI (`.github/workflows/ci.yml`) and a Copilot agent setup-steps workflow (`.github/workflows/copilot-setup-steps.yml`), along with a project README and formatter configuration. All environments share one toolchain definition in `scripts/install-tools.sh`.

## Goals / Non-Goals

**Goals:**

- Provide a devcontainer that makes the project immediately buildable and testable in any container-based environment (Copilot cloud agent, GitHub Codespaces, local devcontainer CLI)
- Deliver GitHub Actions CI and Copilot setup-steps workflow, sharing one toolchain definition
- Keep `BUILD.md` and `README.md` accurate — the devcontainer is the zero-effort path, with manual installation as the alternative
- Configure formatters (clang-format, markdownlint-cli2) consistently for all contributors

**Non-Goals:**

- IDE extensions or settings (VS Code, Neovim, etc.) — devcontainer features unrelated to the build
- Docker image publishing — the devcontainer uses a public base image directly
- Windows or macOS container support — Linux only

## Decisions

### Base image: `mcr.microsoft.com/devcontainers/cpp`

Microsoft's official C++ devcontainer image ships with `gcc`, `g++`, `clang`, `cmake`, `make`, and common build utilities pre-installed on an Ubuntu LTS base. Meson and Ninja are added via `pipx` (latest versions in isolated environments). This avoids a custom Dockerfile while still giving the agent a production-quality toolchain.

**Alternative considered:** Plain `ubuntu:24.04` with `apt` for everything — requires a custom Dockerfile and loses the pre-configured VS Code / Codespaces conveniences without saving meaningful complexity.

### R installation: `r-base` + `r-cran-yaml` via apt

Ubuntu's `r-base` package gives a working R installation. The `yaml` package is available as `r-cran-yaml` in the Ubuntu repos, avoiding the need for an internet-connected `install.packages()` call at container build time.

**Alternative considered:** `rocker/r-ver` base image — more R-focused but loses the C++ toolchain pre-configuration; composing two heavyweight images adds complexity and pull time.

### Meson and Ninja: installed via pipx

`apt`'s Meson version lags the project's `≥ 1.0` requirement on older Ubuntu LTS releases. Installing via `pipx` (with `PIPX_BIN_DIR=/usr/local/bin`) always gives the latest stable versions in an isolated environment and avoids contaminating the system Python.

**Alternative considered:** `pip3 install meson ninja` — works but installs into the global site-packages, which can conflict with system Python packages.

**Alternative considered:** `apt install meson ninja-build` — simpler but risks an old Meson version that fails on `cpp_std=c++23` features.

### Sharing the toolchain with GitHub Actions: postCreateCommand install script

A shell script `scripts/install-tools.sh` captures all `apt`, `pipx`, and `npm` install commands. The devcontainer's `postCreateCommand` runs it; both GitHub Actions workflows (`.github/workflows/ci.yml` and `.github/workflows/copilot-setup-steps.yml`) also run it on plain Ubuntu runners. This gives all environments an identical, auditable toolchain without requiring them to pull a large container image.

**Alternative considered:** GitHub Actions reuses the devcontainer image via `docker pull` — works but is slower and ties CI image pull to devcontainer image availability/versioning.

**Alternative considered:** Inline the install commands in both files — simple to start but creates drift risk; the script is the single source of truth.

### Additional tooling bundled in the install script

While the primary goal is the C++ build environment, Copilot agents also need project-level tooling to be productive from the start: `clang-format` (C++ formatting), `markdownlint-cli2` (Markdown linting), and the `openspec` CLI (change management). Including these in `scripts/install-tools.sh` means the devcontainer and CI always have a fully-equipped environment.

Node.js and npm are installed from Ubuntu's apt repos to satisfy the `npm install -g` step for OpenSpec and markdownlint-cli2.

### Formatting configuration co-located with installer

`.clang-format` (Google style, 4-space indent, 100-column limit) and `.markdownlint.yml` (standard rules, 100-char lines, code blocks and tables exempt) are committed to the repository root. Having formatter config in the repo means any contributor using the installed tools gets consistent formatting with no additional setup.

### CI and Copilot setup-steps in the same PR

Rather than waiting for a separate `github-actions-ci` companion change, both `.github/workflows/ci.yml` and `.github/workflows/copilot-setup-steps.yml` were delivered together with the devcontainer. This avoids a window where the devcontainer exists but CI does not, keeping the two environments in sync from the first commit.

## Risks / Trade-offs

- **pipx-installed Meson vs system PATH** → Mitigation: `PIPX_BIN_DIR=/usr/local/bin` places the binaries on PATH system-wide; the smoke-test at the end of the install script (`meson --version`) catches regressions immediately.
- **`r-cran-yaml` availability across Ubuntu versions** → Mitigation: pin the base image to a specific Ubuntu LTS version tag.
- **Script drift** → Mitigation: both CI workflows reference `scripts/install-tools.sh` directly, not copies of its contents.
- **npm-installed tools version churn** → Mitigation: `@fission-ai/openspec@latest` and `markdownlint-cli2` are pinned to "latest" intentionally for now; they can be pinned to specific versions once stability requirements are known.
