## ADDED Requirements

### Requirement: Dependency presence check
The system SHALL provide `scripts/check-deps.sh`, a shell script that checks whether each required tool is present on `PATH` without performing any installation or requiring elevated privileges.

#### Scenario: All tools present
- **WHEN** all required tools are available on `PATH`
- **THEN** the script SHALL print a success message and exit with code 0

#### Scenario: One or more tools missing
- **WHEN** one or more required tools are not found on `PATH`
- **THEN** the script SHALL print the name of each missing tool, a one-line install hint for Ubuntu 24.04, and exit with code 1

#### Scenario: Multiple tools missing
- **WHEN** several required tools are absent
- **THEN** the script SHALL report ALL missing tools before exiting, not stop at the first failure

### Requirement: Covered tool list
`check-deps.sh` SHALL check for every tool that `scripts/install-tools.sh` installs, including: `git`, `clang-format`, `Rscript`, `meson`, `ninja`, `node`, `npm`, `openspec`, `markdownlint-cli2`.

#### Scenario: Tool list parity
- **WHEN** a new tool is added to `install-tools.sh`
- **THEN** a corresponding check SHALL exist in `check-deps.sh`

### Requirement: No root or install side-effects
`check-deps.sh` SHALL NOT require root, sudo, or any write access to the filesystem. Running it SHALL leave the system state unchanged.

#### Scenario: Non-root execution
- **WHEN** a developer runs `bash scripts/check-deps.sh` without sudo
- **THEN** the script SHALL complete (success or failure) without prompting for a password or modifying any files

### Requirement: Actionable install pointers
For each missing tool, the script SHALL print an install hint that refers to `scripts/install-tools.sh` or provides a specific package/command sufficient to install the tool.

#### Scenario: Missing tool hint
- **WHEN** a required tool is not found
- **THEN** the output SHALL include at least one of: the apt package name, the pip/pipx command, or the npm command needed to obtain it, or a reference to run `scripts/install-tools.sh`

### Requirement: Script is executable and POSIX-compatible
`check-deps.sh` SHALL use a `#!/usr/bin/env bash` shebang, set `set -euo pipefail` is NOT used (because the script must survive individual command failures to accumulate all errors), and SHALL be compatible with Bash 5.x as found on Ubuntu 24.04.

#### Scenario: Script runs without sourcing
- **WHEN** a developer executes `bash scripts/check-deps.sh`
- **THEN** the script SHALL run to completion without requiring it to be sourced
