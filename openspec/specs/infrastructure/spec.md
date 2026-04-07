# Infrastructure Specification

## Purpose
Defines the build system, developer environment, task automation, and R toolchain setup that enable every contributor and AI agent to build, test, and run the project reproducibly.

## Requirements

### Requirement: Project builds with Meson
The project SHALL use Meson as its build system. Running `meson setup build && ninja -C build` SHALL produce a working test binary without errors or warnings.

#### Scenario: Clean setup succeeds
- **WHEN** a developer runs `meson setup build` in an empty build directory
- **THEN** Meson configures successfully, resolves all wraps, and reports no errors

#### Scenario: Build produces test binary
- **WHEN** a developer runs `ninja -C build` after setup
- **THEN** the build completes and a test binary is present in the build directory

### Requirement: Catch2 available via wrap
The project SHALL declare Catch2 v3 as a Meson wrap dependency. It SHALL NOT require manual installation by the developer.

#### Scenario: Wrap resolves automatically
- **WHEN** a developer runs `meson setup build` for the first time
- **THEN** Meson downloads and builds Catch2 as a subproject without manual steps

### Requirement: Baseline test passes
At least one test SHALL exist from the initial commit and SHALL pass.

#### Scenario: Test suite runs clean
- **WHEN** a developer runs `meson test -C build`
- **THEN** all tests pass and the exit code is 0

### Requirement: Standard directory layout
The project SHALL use the following top-level layout:
- `cpp/` — C++ source files
- `cpp/generated/` — committed generated C++ headers
- `tests/` — Catch2 test files
- `data/` — canonical data files (patch catalog)
- `codegen/` — R codegen scripts
- `logs/` — runtime game logs (gitignored)

#### Scenario: Directory structure is present
- **WHEN** a developer clones the repository
- **THEN** `cpp/`, `tests/`, `data/`, `codegen/` directories exist with at least one file each

### Requirement: Devcontainer provides a complete build environment
The repository SHALL contain a `.devcontainer/devcontainer.json` that, when used to create a container, results in an environment where `meson setup build && meson test -C build` succeeds without any additional manual installation steps.

#### Scenario: Copilot agent can build the project
- **GIVEN** a GitHub Copilot coding agent has opened the repository using the devcontainer configuration
- **WHEN** the agent runs `meson setup build && ninja -C build`
- **THEN** the build completes successfully and all tests pass

#### Scenario: R codegen works in the container
- **GIVEN** the devcontainer environment is active
- **WHEN** a user or agent runs `Rscript codegen/generate_patches.R`
- **THEN** the script completes without errors and `cpp/generated/patches.hpp` is produced

### Requirement: Toolchain is defined once
The toolchain required to build and test the project SHALL be defined in exactly one place (`scripts/install-tools.sh`). All tools (Meson, Ninja, GCC/C++23, R, yaml package, clang-format, OpenSpec CLI, markdownlint-cli2) SHALL be listed there, and the devcontainer configuration and all GitHub Actions workflows SHALL invoke this script rather than restating the install commands.

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
The repository SHALL expose `mise run format` (clang-format on all `cpp/` C++ files) and `mise run lint` (markdownlint-cli2 on all Markdown files) as named tasks in `.mise.toml`, so that formatting and linting are discoverable and invokable without memorizing the raw commands. Both tools SHALL be installed by `scripts/install-tools.sh` and their configuration committed to the repository (`.clang-format`, `.markdownlint.yml`).

#### Scenario: Developer formats C++ files via mise
- **GIVEN** the devcontainer environment is active and `mise` is available
- **WHEN** a developer runs `mise run format`
- **THEN** all C++ files in `cpp/` are formatted in place according to `.clang-format`

#### Scenario: Developer lints Markdown files via mise
- **GIVEN** the devcontainer environment is active and `mise` is available
- **WHEN** a developer runs `mise run lint`
- **THEN** markdownlint-cli2 reports any violations according to `.markdownlint.yml`

### Requirement: Project documentation references mise as primary invocation
The `README.md` and `BUILD.md` files SHALL reference `mise run <task>` as the canonical way to invoke every documented project action. The underlying raw commands MAY appear as secondary details.

#### Scenario: README quick-start uses mise
- **GIVEN** a developer opens `README.md`
- **WHEN** they follow the quick-start section
- **THEN** the commands shown are `mise run setup` and `mise run test`

#### Scenario: BUILD.md sections use mise as primary
- **GIVEN** a developer opens `BUILD.md` for the Setup, Build, Test, or Code Generation section
- **WHEN** they look for the command to run
- **THEN** the primary command shown is `mise run <task>` and the raw command is in a secondary detail block

### Requirement: Every useful project action is runnable through mise
The repository SHALL maintain the invariant that every useful developer or agent action has a corresponding `mise run <task>` entry in `.mise.toml`. Whenever a new useful action is introduced to the project, a mise task for it SHALL be added in the same change.

#### Scenario: new capability ships with a mise task
- **GIVEN** a change adds a new project action (build step, script, analysis, etc.)
- **WHEN** the change is merged
- **THEN** `.mise.toml` contains a task that invokes that action by name

### Requirement: mise tasks cover all current useful project actions
The `.mise.toml` SHALL define tasks for every action documented in `BUILD.md`:
- `setup` — configure the Meson build directory
- `build` — compile the project
- `test` — run the test suite
- `codegen` — regenerate committed C++ headers from data files

#### Scenario: setup task configures the build directory
- **WHEN** a developer runs `mise run setup` in the repository root
- **THEN** Meson configures the `build/` directory (`meson setup build` is invoked)

#### Scenario: build task compiles the project
- **WHEN** a developer runs `mise run build` after setup
- **THEN** Ninja builds the project (`ninja -C build` is invoked)

#### Scenario: test task runs the test suite
- **WHEN** a developer runs `mise run test` after build
- **THEN** Meson runs all tests (`meson test -C build` is invoked)

#### Scenario: codegen task regenerates headers
- **WHEN** a developer runs `mise run codegen`
- **THEN** the R codegen script runs (`Rscript codegen/generate_patches.R`) and `cpp/generated/patches.hpp` is produced

### Requirement: build directory is on PATH in mise-managed shells
The `.mise.toml` SHALL configure `[env] _.path` to include `{{config_root}}/build` so that compiled binaries in the `build/` directory are accessible by name without a full path.

#### Scenario: compiled binary is reachable by name
- **GIVEN** a developer has run `mise run build` and is in a mise-managed shell
- **WHEN** the developer types the binary name (without a path prefix)
- **THEN** the shell resolves the binary from the `build/` directory

### Requirement: mise tasks are the primary entry point in project documentation
Project documentation SHALL reference `mise run <task>` as the canonical invocation for every documented project action. The underlying raw commands MAY be shown as secondary details, but mise tasks SHALL be presented first.

> **Note:** This requirement applies once project documentation specs are written. It is a cross-cutting constraint to be enforced when documentation specs are created or updated.

#### Scenario: documentation references mise for build steps
- **GIVEN** project documentation describes how to build or test the project
- **WHEN** a developer reads that documentation
- **THEN** the documented invocation is `mise run <task>`, not the raw Meson/Ninja/Rscript command

#### Scenario: mise task list matches documented actions
- **GIVEN** project documentation lists the available project actions
- **WHEN** a developer runs `mise tasks` in the repository root
- **THEN** every documented action appears as a mise task with a matching description

### Requirement: Script runs as regular user, sudo used only where needed
`install-tools.sh` SHALL be invoked as a regular user (without `sudo bash`). Commands requiring root privileges (apt-get, rig binary extraction to /usr/local, pipx/npm global installs) SHALL use `sudo` inline. `rig` commands SHALL be invoked without `sudo` as rig manages its own privilege escalation internally.

#### Scenario: Script succeeds when run as a regular user
- **WHEN** `bash scripts/install-tools.sh` is executed as a non-root user with sudo privileges on Ubuntu 24.04
- **THEN** the script completes successfully without requiring `sudo bash` invocation, and system-level steps (apt-get, rig binary extraction) succeed via inline `sudo`

### Requirement: rig user library and pak are set up for the current user
After installing R via `rig add release`, `install-tools.sh` SHALL call `rig system setup-user-lib` and `rig system add-pak` (without sudo) so that pak is installed into the current user's R library and is accessible to subsequent `Rscript` calls running as the same user.

#### Scenario: pak is accessible after rig setup
- **WHEN** `rig system setup-user-lib` and `rig system add-pak` have been called after `rig add release`
- **THEN** `Rscript -e 'pak::pkg_install("DBI", ask = FALSE)'` succeeds without error, confirming pak is available in the current user's library

### Requirement: PPM binary repository is configured for all R package installs
`install-tools.sh` SHALL write a site-wide `Rprofile.site` entry that sets the necessary options for the PPM binary CRAN mirror for Ubuntu 24.04 (`noble`) (repos **and** the `HTTPUserAgent` string that PPM requires to serve Linux binaries), before any R package installation occurs.

#### Scenario: Rprofile.site contains PPM repos and HTTPUserAgent
- **WHEN** `install-tools.sh` has completed the R setup section on Ubuntu 24.04
- **THEN** `/opt/R/<version>/lib/R/etc/Rprofile.site` contains an `options()` block setting `repos` to the PPM noble URL and `HTTPUserAgent` to the Linux R client string required for PPM to serve pre-built binaries

### Requirement: Binary installation is verified by a smoke test immediately after R setup
The DBI binary-install smoke test SHALL be placed immediately after `rig system add-pak` and PPM configuration (not in the final smoke-test section) to fail fast. The test SHALL install `DBI` using `pak::pkg_install("DBI", ask = FALSE)` and assert that no install from source was invoked, confirming PPM served a pre-built binary.

#### Scenario: DBI installs as binary without source compilation
- **WHEN** `install-tools.sh` runs the DBI smoke test on Ubuntu 24.04 with PPM configured
- **THEN** `pak::pkg_install("DBI", ask = FALSE)` completes without printing any reference to source install, and DBI appears in the installed packages list

#### Scenario: pak uses PPM when installing packages
- **WHEN** `Rscript -e 'pak::pak()'` is executed after `install-tools.sh`
- **THEN** packages are downloaded as pre-built binaries (no compilation from source for packages with available binaries)

### Requirement: DESCRIPTION file declares the R package
The project root SHALL contain a `DESCRIPTION` file with at minimum the fields `Package`, `Version`, `Title`, `Description`, `License`, and `Imports` (listing `yaml`). The `Package` field SHALL NOT be `patchwork` (to avoid collision with the CRAN package of the same name).

#### Scenario: pkgload::load_all() succeeds from project root
- **WHEN** `pkgload::load_all()` is called from the project root
- **THEN** all functions defined in `R/*.R` are available in the current session without error

#### Scenario: DESCRIPTION declares yaml as a dependency
- **WHEN** the `DESCRIPTION` file is read
- **THEN** `yaml` appears in the `Imports` field

### Requirement: NAMESPACE file enables function export
The project root SHALL contain a `NAMESPACE` file managed by `roxygen2` (indicated by `Config/roxygen2/version` in `DESCRIPTION`). Public functions intended for use from `codegen/` scripts SHALL carry `@export` roxygen tags; helper functions may remain unexported.

#### Scenario: Public functions are accessible after load_all
- **WHEN** `pkgload::load_all()` is called
- **THEN** functions defined in `R/` that do not start with `.` are accessible by name in the calling environment

### Requirement: R/ directory holds shared package functions
An `R/` directory SHALL exist at the project root and contain at minimum `patches.R` (patch-related helpers) and `setups.R` (setup-related helpers).

#### Scenario: R/ contains patches.R and setups.R
- **WHEN** the `R/` directory is listed
- **THEN** both `patches.R` and `setups.R` are present

#### Scenario: Patch helper functions are defined in R/patches.R
- **WHEN** `pkgload::load_all()` is called
- **THEN** the functions `parse_cells`, `rotate90`, `reflect_h`, `normalise_cells`, `cell_key`, `cells_to_rows`, `canonical_shape`, `parse_shape`, and `count_x` are available

#### Scenario: Setup helper functions are defined in R/setups.R
- **WHEN** `pkgload::load_all()` is called
- **THEN** the functions `generate_setups` and `generate_patches` (or equivalently named top-level generators) are available
