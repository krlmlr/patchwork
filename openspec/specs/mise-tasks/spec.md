# mise-tasks Specification

## Purpose
Defines the convention that every useful developer or agent project action is accessible via a `mise run <task>` entry in `.mise.toml`.

## Requirements

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
