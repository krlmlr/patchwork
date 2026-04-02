### Requirement: mise tasks expose the Meson + Ninja workflow
The repository SHALL contain a `.mise.toml` at the root that defines three tasks — `setup`, `build`, and `test` — each running the corresponding Meson or Ninja command documented in `BUILD.md`.

#### Scenario: setup task configures the build directory
- **WHEN** a developer runs `mise run setup` in the repository root
- **THEN** Meson configures the `build/` directory (`meson setup build` is invoked)

#### Scenario: build task compiles the project
- **WHEN** a developer runs `mise run build` after setup
- **THEN** Ninja builds the project (`ninja -C build` is invoked)

#### Scenario: test task runs the test suite
- **WHEN** a developer runs `mise run test` after build
- **THEN** Meson runs all tests (`meson test -C build` is invoked)

### Requirement: build directory is on PATH in mise-managed shells
The `.mise.toml` SHALL configure `[env] _.path` to include `{{config_root}}/build` so that compiled binaries in the `build/` directory are accessible by name without a full path.

#### Scenario: compiled binary is reachable by name
- **GIVEN** a developer has run `mise run build` and is in a mise-managed shell
- **WHEN** the developer types the binary name (without a path prefix)
- **THEN** the shell resolves the binary from the `build/` directory
