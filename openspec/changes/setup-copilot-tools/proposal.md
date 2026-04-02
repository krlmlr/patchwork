## Why

GitHub Copilot coding agents run in an ephemeral cloud container. Without an explicit devcontainer configuration, the agent has no access to the project's build tools — Meson, Ninja, a C++23 compiler, or R — and cannot build or test the project. This prevents Copilot from verifying its own changes, defeating the "tests from day one" principle.

A companion change will add GitHub Actions CI (running the same build and tests on every push and PR). Both environments need the same toolchain. Encoding that toolchain once in a devcontainer configuration eliminates drift and is the single source of truth for "what is needed to build this project".

## What Changes

- Add **`.devcontainer/devcontainer.json`** declaring the project's build environment: Ubuntu base, Meson, Ninja, GCC (C++23), and R with the `yaml` package
- **Remove duplication** with the forthcoming GitHub Actions spec: the CI workflow will reference the same container image or reproduce its install steps from one place (the devcontainer image or a shared install script), so the toolchain is not defined twice

## Capabilities

### New Capabilities

- `devcontainer`: `.devcontainer/devcontainer.json` that gives any cloud agent (Copilot, Codespaces, devcontainer CLI) a ready-to-build environment for this project

### Modified Capabilities

- (none in this change; GitHub Actions integration is handled by the companion spec and updated here only to reduce duplication)

## Impact

- GitHub Copilot coding agents will be able to run `meson setup build && meson test -C build` immediately after checkout
- R-based codegen (`Rscript codegen/generate_patches.R`) will work inside the container
- No changes to `src/`, `tests/`, `data/`, or `codegen/` — this is purely environment scaffolding
- `BUILD.md` prerequisites section will be updated to reflect that the devcontainer satisfies all prerequisites automatically
