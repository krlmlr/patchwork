## Why

Developers and Copilot coding agents who have [mise](https://mise.jdx.dev/) installed have no standard entry points for the project's build and development workflow. The raw commands are documented in `BUILD.md`, but there is no machine-readable task definition that a tool (or agent) can discover and invoke by name. Adding a `.mise.toml` establishes the principle that **every useful action in this repository is runnable through mise**, encodes the workflow once, keeps it in sync with documentation, and puts the `build/` output directory on `$PATH` automatically in mise-managed shells.

## What Changes

- Add **`.mise.toml`** at the repository root with tasks covering all current useful project actions:
  - `setup` — configure the Meson build directory
  - `build` — compile the project
  - `test` — run the test suite
  - `codegen` — regenerate committed C++ headers from data files
- Configure **`[env] _.path`** to include `build/` so compiled binaries are on `$PATH` without a full path when running inside a mise-managed shell

## Capabilities

### New Capabilities

- `mise-tasks`: `.mise.toml` exposing all useful project actions as named `mise run <task>` entry points, with `build/` on `$PATH` in mise-managed shells

### Modified Capabilities

- (none)

## Impact

- Developers with mise installed can run any project workflow step by name instead of memorizing raw commands
- The `build/` directory is on `$PATH` in mise-managed shells, so compiled binaries are directly accessible
- Every new useful project action added in the future SHALL be accompanied by a corresponding mise task
- No changes to `src/`, `tests/`, `data/`, `codegen/`, or any build configuration
