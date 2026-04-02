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
The set of tools required to build and test the project (Meson, Ninja, GCC/C++23, R, yaml package) SHALL be listed in exactly one place (`scripts/install-tools.sh`). The devcontainer configuration and the GitHub Actions CI workflow SHALL both invoke this script rather than restating the install commands.

#### Scenario: Adding a new tool requires one change
- **GIVEN** a new build dependency is added to the project
- **WHEN** a developer updates `scripts/install-tools.sh`
- **THEN** both the devcontainer and the GitHub Actions workflow gain the new dependency without further changes
