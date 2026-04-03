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
The set of tools required to build and test the project (Meson, Ninja, GCC/C++23, R, yaml package, clang-format, OpenSpec CLI, markdownlint-cli2) SHALL be listed in exactly one place (`scripts/install-tools.sh`). The devcontainer configuration and all GitHub Actions workflows SHALL invoke this script rather than restating the install commands.

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

### Requirement: Formatting tools are configured and installed
The repository SHALL contain `.clang-format` for C++ formatting and `.markdownlint.yml` for Markdown linting. Both tools SHALL be installed by `scripts/install-tools.sh`.

#### Scenario: C++ files are formatted consistently
- **GIVEN** the devcontainer environment is active
- **WHEN** a developer runs `clang-format -i` on any C++ file
- **THEN** the file is formatted according to the project style defined in `.clang-format`

#### Scenario: Markdown files pass the linter
- **GIVEN** the devcontainer environment is active
- **WHEN** a developer runs `markdownlint-cli2 "**/*.md"`
- **THEN** any violations are reported according to the rules in `.markdownlint.yml`
