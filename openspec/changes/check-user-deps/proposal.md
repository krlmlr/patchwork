## Why

`scripts/install-tools.sh` requires root/sudo and makes system-wide changes — it is unsuitable for developers who want a quick sanity-check without running an invasive installer. There is currently no lightweight way to verify that all required tools are present and at a usable version before building.

## What Changes

- Add `scripts/check-deps.sh`: a read-only script that checks whether each required tool is present and prints a clear, actionable pointer when one is missing. No installs, no root required.
- Update `README.md` (or `BUILD.md`) to document `check-deps.sh` alongside `install-tools.sh` so developers know both scripts exist and when to use each.
- Add a `check-deps` step to `.github/workflows/ci.yml` that runs `check-deps.sh` **before** the install step, providing early failure feedback and ensuring the script stays up to date with the real tool list.
- Keep `check-deps.sh` in sync with `install-tools.sh` — both files define the canonical tool list; any future addition to the installer must also appear in the checker.

## Capabilities

### New Capabilities

- `dep-checker`: Non-invasive shell script (`scripts/check-deps.sh`) that inspects the current `PATH` for every required tool, reports which ones are missing with install pointers, and exits non-zero if any are absent.

### Modified Capabilities

- `ci-pipeline`: Add a `check-deps` job (or step) to `ci.yml` that runs the checker after the install step, confirming the installer actually placed all tools on `PATH`.

## Impact

- `scripts/check-deps.sh` — new file
- `scripts/install-tools.sh` — no functional change; serves as source of truth for the expected tool list
- `.github/workflows/ci.yml` — new step added to `build-and-test` job
- `BUILD.md` — document both scripts
