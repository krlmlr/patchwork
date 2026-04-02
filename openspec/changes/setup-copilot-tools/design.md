## Context

The project builds with Meson + Ninja and requires a C++23 compiler and R for codegen. These tools must be present before any build or test command can run. GitHub Copilot coding agents operate in an ephemeral container that is configured by `.devcontainer/devcontainer.json` at repository root (or in `.devcontainer/`). Without this file the agent starts with a bare environment.

A companion change (`github-actions-ci`, to be implemented first) will add a GitHub Actions workflow that runs `meson setup build && meson test -C build` on every push and pull request. Both environments need the identical toolchain; this change is responsible for encoding it once and having the CI workflow consume it rather than restate it.

## Goals / Non-Goals

**Goals:**

- Provide a devcontainer that makes the project immediately buildable and testable in any container-based environment (Copilot cloud agent, GitHub Codespaces, local devcontainer CLI)
- Eliminate toolchain duplication between the devcontainer and GitHub Actions CI
- Keep `BUILD.md` accurate — the devcontainer should be presented as the zero-effort path, with manual installation as the alternative

**Non-Goals:**

- IDE extensions or settings (VS Code, Neovim, etc.) — devcontainer features unrelated to the build
- Docker image publishing — the devcontainer uses a public base image directly
- Windows or macOS container support — Linux only

## Decisions

### Base image: `mcr.microsoft.com/devcontainers/cpp`

Microsoft's official C++ devcontainer image ships with `gcc`, `g++`, `clang`, `cmake`, `make`, and common build utilities pre-installed on an Ubuntu LTS base. Meson and Ninja are added via `pip3` (Meson's recommended install path for the latest version). This avoids a custom Dockerfile while still giving the agent a production-quality toolchain.

**Alternative considered:** Plain `ubuntu:24.04` with `apt` for everything — requires a custom Dockerfile and loses the pre-configured VS Code / Codespaces conveniences without saving meaningful complexity.

### R installation: `r-base` + `r-cran-yaml` via apt

Ubuntu's `r-base` package gives a working R installation. The `yaml` package is available as `r-cran-yaml` in the Ubuntu repos, avoiding the need for an internet-connected `install.packages()` call at container build time.

**Alternative considered:** `rocker/r-ver` base image — more R-focused but loses the C++ toolchain pre-configuration; composing two heavyweight images adds complexity and pull time.

### Meson and Ninja: installed via pip3

`apt`'s Meson version lags the project's `≥ 1.0` requirement on older Ubuntu LTS releases. `pip3 install meson ninja` always gives the latest stable versions. The `mcr.microsoft.com/devcontainers/cpp` image includes Python 3 and pip3.

**Alternative considered:** `apt install meson ninja-build` — simpler but risks an old Meson version that fails on `cpp_std=c++23` features.

### Sharing the toolchain with GitHub Actions: postCreateCommand install script

A shell script `scripts/install-tools.sh` captures the `apt` and `pip3` install commands. The devcontainer's `postCreateCommand` runs it; the GitHub Actions workflow (from the companion spec) also runs it on a plain Ubuntu runner instead of pulling the full devcontainer image. This gives both environments an identical, auditable toolchain without requiring GitHub Actions to pull a large container image on every run.

**Alternative considered:** GitHub Actions reuses the devcontainer image via `docker pull` — works but is slower and ties CI image pull to devcontainer image availability/versioning.

**Alternative considered:** Inline the install commands in both files — simple to start but creates drift risk; the script is the single source of truth.

### No `devcontainer.json` features beyond the base image

Copilot coding agents do not benefit from VS Code extensions. Adding features like `ghcr.io/devcontainers/features/git` would bloat the image without aiding build/test.

## Risks / Trade-offs

- **pip3-installed Meson vs system Meson path conflicts** → Mitigation: the Microsoft C++ image puts pip3-installed binaries on PATH; add a `postCreateCommand` smoke-test (`meson --version`) to catch regressions.
- **`r-cran-yaml` availability across Ubuntu versions** → Mitigation: pin the base image to a specific Ubuntu LTS version tag.
- **Script drift** → Mitigation: the CI spec must reference `scripts/install-tools.sh` directly, not copy its contents.
