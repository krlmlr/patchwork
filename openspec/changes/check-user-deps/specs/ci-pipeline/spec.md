## ADDED Requirements

### Requirement: CI runs dep checker after install
The CI workflow SHALL include a step that executes `bash scripts/check-deps.sh` immediately after the `Install tools` step in the `build-and-test` job.

#### Scenario: Installer installs all tools
- **WHEN** the `Install tools` step completes successfully
- **THEN** the `Check installed tools` step SHALL pass (exit 0) because all tools are on `PATH`

#### Scenario: Installer misses a tool
- **WHEN** a tool is present in `check-deps.sh` but not installed by `install-tools.sh`
- **THEN** the `Check installed tools` step SHALL fail with a non-zero exit code and list the missing tool(s) in the CI log

### Requirement: CI step fails the build on missing tools
The `Check installed tools` CI step SHALL NOT use `continue-on-error: true`. A missing tool after installation SHALL be a hard build failure.

#### Scenario: Missing tool after install
- **WHEN** `check-deps.sh` exits with code 1 in CI
- **THEN** the overall CI job SHALL be marked as failed
