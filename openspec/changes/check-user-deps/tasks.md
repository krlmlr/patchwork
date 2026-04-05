## 1. Dependency checker script

- [ ] 1.1 Create `scripts/check-deps.sh` with a `#!/usr/bin/env bash` shebang and a header comment mirroring `install-tools.sh`
- [ ] 1.2 Implement a `check_tool` helper that uses `command -v` to detect presence and prints an install hint on failure; accumulates failures rather than exiting immediately
- [ ] 1.3 Add a check block for each tool installed by `install-tools.sh`: `git`, `clang-format`, `Rscript`, `meson`, `ninja`, `node`, `npm`, `openspec`, `markdownlint-cli2`
- [ ] 1.4 Print a summary: all-good message on success, or a "run `scripts/install-tools.sh`" pointer on failure, then exit with the accumulated error code
- [ ] 1.5 Make the script executable (`chmod +x scripts/check-deps.sh`)

## 2. CI integration

- [ ] 2.1 Add a `Check installed tools` step to `.github/workflows/ci.yml` immediately after the `Install tools` step, running `bash scripts/check-deps.sh`
- [ ] 2.2 Verify the step has no `continue-on-error` flag so a missing tool is a hard failure

## 3. Documentation

- [ ] 3.1 Add a `Check prerequisites` section to `BUILD.md` that describes `check-deps.sh`, when to use it, and how it differs from `install-tools.sh`
