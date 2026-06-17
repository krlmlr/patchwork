## ADDED Requirements

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
