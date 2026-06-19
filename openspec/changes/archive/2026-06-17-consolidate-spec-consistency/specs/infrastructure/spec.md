## MODIFIED Requirements

### Requirement: build directory is on PATH in mise-managed shells
The `.mise.toml` SHALL configure `[env] _.path` to include `{{config_root}}/build/cpp` so that compiled binaries produced under `build/cpp/` are accessible by name without a full path.

#### Scenario: compiled binary is reachable by name
- **GIVEN** a developer has run `mise run build` and is in a mise-managed shell
- **WHEN** the developer types the binary name (without a path prefix)
- **THEN** the shell resolves the binary from the `build/cpp/` directory

### Requirement: Catalog is kept up to date
Every change that adds, removes, or renames a spec, or that changes a domain's described coverage in the catalog, SHALL include a task step to update `openspec/specs/README.md`. This SHALL match the maintenance rule stated at the top of `openspec/specs/README.md` itself, so the two never diverge.

#### Scenario: Change tasks include a catalog update step
- **WHEN** a change's `tasks.md` introduces, removes, or renames a spec, or changes what a domain covers
- **THEN** the tasks list SHALL include an explicit step to update `openspec/specs/README.md` accordingly
