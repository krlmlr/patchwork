## Context

`scripts/install-tools.sh` is the project's authoritative installer: it runs as root, calls `apt-get`, and installs packages system-wide. It is well-suited for CI runners and fresh devcontainers, but it is too invasive for a developer who just wants to know whether their existing environment is ready to build.

Today there is no quick way to answer "am I missing anything?" without running the full installer. There is also no CI gate ensuring the installer and tool-list documentation stay consistent over time.

## Goals / Non-Goals

**Goals:**

- Provide `scripts/check-deps.sh`: a pure-read, no-root script that checks each required tool for presence (and optionally minimum version) and prints a clear, actionable "how to install" pointer for each missing tool.
- Keep the checker in sync with the installer — both files are maintained side-by-side so adding a tool to one prompts adding it to the other.
- Run the checker in CI after the installer step to confirm the installer actually places every tool on `PATH`.
- Document `check-deps.sh` in `BUILD.md` so developers know it exists.

**Non-Goals:**

- Performing any installation or modification of the system.
- Version-range enforcement beyond a basic minimum check (exact pinning belongs in the installer).
- Support for platforms other than Ubuntu 24.04 (same scope as `install-tools.sh`).
- Replacing or refactoring `install-tools.sh`.

## Decisions

### Decision: Single-file checker, parallel to the installer

The checker lives in `scripts/check-deps.sh` — the same directory as `install-tools.sh`. This keeps both scripts visually adjacent and makes "did I add it to both?" easy to audit.

*Alternative considered:* inline version checks inside `install-tools.sh`. Rejected because that mixes concerns — install and verify have different security requirements (root vs. non-root) and different audiences.

### Decision: Exit non-zero and print all failures before exiting

The script accumulates all missing-tool errors, prints them together, then exits 1. This gives developers the full picture in a single run instead of requiring repeated attempts.

*Alternative:* fail-fast on first missing tool. Rejected because seeing all gaps at once is more useful.

### Decision: `command -v` for presence; optional `--version` grep for minimum version

`command -v <tool>` is POSIX-compliant and works in any `sh`-compatible shell. For tools where a minimum version matters (e.g., Catch2's header version, Meson ≥ 1.0), a lightweight `--version` grep is added. Version parsing is kept simple (no semver library) to avoid introducing dependencies in the checker itself.

*Alternative:* use `which`. Rejected — `command -v` is more portable and does not require `which` to be installed.

### Decision: Install pointers are inline in the script

Each tool block includes a short human-readable hint (`apt-get install …` / `pip install …` / `npm install -g …`) rather than linking to external docs. This keeps the output self-contained for developers working offline or in restricted environments.

### Decision: Add a CI step after the installer, not a separate job

A new `Check installed tools` step in the existing `build-and-test` job runs `bash scripts/check-deps.sh` immediately after the `Install tools` step. This validates the installer without introducing job-level parallelism overhead, and the step's failure surfaces in the same log as the install.

## Risks / Trade-offs

- [Risk] Installer and checker diverge over time → Mitigation: code-review checklist (both files touched when tool list changes); CI failure if installer produces a broken environment.
- [Risk] Version detection is brittle for some tools (e.g. Catch2 header version) → Mitigation: version checks are optional per-tool and can be omitted for tools where `--version` output is non-standard; presence check alone is still useful.
- [Risk] Script runs on macOS in local dev and `apt-get` hints are irrelevant → Mitigation: hints are labelled "Ubuntu 24.04" and the script itself is platform-agnostic (it never calls apt-get, only prints text).
