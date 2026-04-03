## ADDED Requirements

### Requirement: Devcontainer provides a complete build environment
The repository SHALL contain a `.devcontainer/devcontainer.json` that, when used to create a container, results in an environment where `meson setup build && meson test -C build` succeeds without any additional manual installation steps.

#### Scenario: Copilot agent can build the project
- **GIVEN** a GitHub Copilot coding agent has opened the repository using the devcontainer configuration
- **WHEN** the agent runs `meson setup build && ninja -C build`
- **THEN** the build completes successfully and all tests pass

#### Scenario: R codegen works in the container
- **GIVEN** the devcontainer environment is active
- **WHEN** a user or agent runs `Rscript codegen/generate_patches.R`
- **THEN** the script completes without errors and `src/generated/patches.hpp` is produced

### Requirement: Toolchain is defined once
The set of tools required to build and test the project (Meson, Ninja, GCC/C++23, R, yaml package,
clang-format, OpenSpec CLI, markdownlint-cli2) SHALL be listed in exactly one place
(`scripts/install-tools.sh`). The devcontainer configuration and all GitHub Actions workflows SHALL
invoke this script rather than restating the install commands.

#### Scenario: Adding a new tool requires one change
- **GIVEN** a new build dependency is added to the project
- **WHEN** a developer updates `scripts/install-tools.sh`
- **THEN** both the devcontainer and all GitHub Actions workflows gain the new dependency without further changes

### Requirement: Copilot agent setup steps workflow exists
The repository SHALL contain `.github/workflows/copilot-setup-steps.yml` so that GitHub Copilot coding agents can provision the full project toolchain before starting a task.

#### Scenario: Copilot agent provisions the toolchain
- **GIVEN** a GitHub Copilot coding agent session starts for this repository
- **WHEN** the copilot-setup-steps workflow runs
- **THEN** all tools from `scripts/install-tools.sh` are available in the agent's environment

### Requirement: Repository contains a README with quick-start instructions
The repository SHALL contain a `README.md` at the root that explains zero-effort setup (devcontainer / Codespaces), links to `BUILD.md` for manual setup, documents the project structure, and describes how to run the formatters.

#### Scenario: New contributor can start building without reading BUILD.md
- **GIVEN** a developer opens the repository for the first time
- **WHEN** they follow the quick-start in `README.md`
- **THEN** they can build and test the project without consulting any other document

### Requirement: Formatting and linting available as mise tasks
The repository SHALL expose `mise run format` (clang-format on all `src/` C++ files) and `mise run lint` (markdownlint-cli2 on all Markdown files) as named tasks in `.mise.toml`, so that formatting and linting are discoverable and invokable without memorizing the raw commands. Both tools SHALL be installed by `scripts/install-tools.sh` and their configuration committed to the repository (`.clang-format`, `.markdownlint.yml`).

#### Scenario: Developer formats C++ files via mise
- **GIVEN** the devcontainer environment is active and `mise` is available
- **WHEN** a developer runs `mise run format`
- **THEN** all C++ files in `src/` are formatted in place according to `.clang-format`

#### Scenario: Developer lints Markdown files via mise
- **GIVEN** the devcontainer environment is active and `mise` is available
- **WHEN** a developer runs `mise run lint`
- **THEN** markdownlint-cli2 reports any violations according to `.markdownlint.yml`

### Requirement: Project documentation references mise as primary invocation
The `README.md` and `BUILD.md` files SHALL reference `mise run <task>` as the canonical way to invoke every documented project action, in compliance with the globally merged `mise-tasks` spec. The underlying raw commands MAY appear as secondary details.

#### Scenario: README quick-start uses mise
- **GIVEN** a developer opens `README.md`
- **WHEN** they follow the quick-start section
- **THEN** the commands shown are `mise run setup` and `mise run test`

#### Scenario: BUILD.md sections use mise as primary
- **GIVEN** a developer opens `BUILD.md` for the Setup, Build, Test, or Code Generation section
- **WHEN** they look for the command to run
- **THEN** the primary command shown is `mise run <task>` and the raw command is in a secondary detail block
