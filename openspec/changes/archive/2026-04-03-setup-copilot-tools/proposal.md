## Why

GitHub Copilot coding agents run in an ephemeral cloud container. Without an explicit devcontainer configuration, the agent has no access to the project's build tools — Meson, Ninja, a C++23 compiler, or R — and cannot build or test the project. This prevents Copilot from verifying its own changes, defeating the "tests from day one" principle.

A companion change will add GitHub Actions CI (running the same build and tests on every push and PR). Both environments need the same toolchain. Encoding that toolchain once in a devcontainer configuration eliminates drift and is the single source of truth for "what is needed to build this project".

## What Changes

- Add **`scripts/install-tools.sh`** — single source of truth for all prerequisites: apt packages (C++23 compiler via the base image, R, r-cran-yaml, clang-format, nodejs, npm), Meson and Ninja via pipx, and OpenSpec CLI plus markdownlint-cli2 via npm; ends with version smoke-tests
- Add **`.devcontainer/devcontainer.json`** — Microsoft C++ devcontainer base image (Ubuntu 24.04 LTS), `postCreateCommand` runs `scripts/install-tools.sh`
- Add **`.github/workflows/ci.yml`** — build + Catch2 tests + R codegen smoke-test on every push and PR; toolchain installed via `scripts/install-tools.sh`
- Add **`.github/workflows/copilot-setup-steps.yml`** — provisions the full toolchain for Copilot agent sessions
- Add **`README.md`** — quick-start with `mise run setup` / `mise run test`, project-structure table, `mise run format` / `mise run lint` for formatting, OpenSpec usage
- Add **`.clang-format`** — Google style, 4-space indent, 100-column limit
- Add **`.markdownlint.yml`** — standard rules, 100-char line length, code blocks and tables exempt
- Update **`BUILD.md`** — add "Zero-effort setup" section at the top; reference `scripts/install-tools.sh` as the direct Ubuntu install path; `mise run <task>` as primary invocation for every documented action (raw commands in `<details>` blocks)
- Add `[tasks.format]` and `[tasks.lint]` to **`.mise.toml`** — expose clang-format and markdownlint-cli2 as named `mise run` tasks

## Capabilities

### New Capabilities

- `devcontainer`: `.devcontainer/devcontainer.json` that gives any cloud agent (Copilot, Codespaces, devcontainer CLI) a ready-to-build environment for this project
- `ci`: `.github/workflows/ci.yml` running build, tests, and R codegen smoke-test on every push and PR
- `copilot-setup-steps`: `.github/workflows/copilot-setup-steps.yml` provisioning the full toolchain for Copilot agent sessions
- `readme`: `README.md` with quick-start (`mise run` commands), project structure, and tooling documentation
- `formatting`: `.clang-format` and `.markdownlint.yml` configuration files; `mise run format` and `mise run lint` tasks; both formatters installed by `scripts/install-tools.sh`

### Modified Capabilities

- `build-docs`: `BUILD.md` updated to present the devcontainer as the zero-effort path and `mise run <task>` as the primary invocation for every documented action
- `mise-tasks`: `.mise.toml` extended with `format` and `lint` tasks for the new formatting tools

## Impact

- GitHub Copilot coding agents will be able to run `mise run setup && mise run test` immediately after checkout
- R-based codegen (`mise run codegen`) will work inside the container
- CI will catch regressions on every push and pull request
- `mise run format` and `mise run lint` are available in the devcontainer for consistent formatting
- Every project action is now invokable via `mise run <task>`, satisfying the `mise-tasks` spec invariant
- No changes to `src/`, `tests/`, `data/`, or `codegen/` — this is purely environment scaffolding and documentation
