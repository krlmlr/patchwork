## Why

Developers and Copilot coding agents who have [mise](https://mise.jdx.dev/) installed have no standard entry points for the project's build workflow. The raw Meson + Ninja commands are documented in `BUILD.md`, but there is no machine-readable task definition that a tool (or agent) can discover and invoke by name. Adding a `.mise.toml` encodes the workflow once, keeps it in sync with `BUILD.md`, and puts the `build/` output directory on `$PATH` automatically in mise-managed shells.

## What Changes

- Add **`.mise.toml`** at the repository root with three tasks (`setup`, `build`, `test`) that mirror the commands in `BUILD.md`
- Configure **`[env] _.path`** to include `build/` so compiled binaries are on `$PATH` without a full path when running inside a mise-managed shell

## Capabilities

### New Capabilities

- `mise-tasks`: `.mise.toml` exposing `mise run setup`, `mise run build`, and `mise run test` as named entry points for the Meson + Ninja workflow

### Modified Capabilities

- (none)

## Impact

- Developers with mise installed can run `mise run setup`, `mise run build`, and `mise run test` instead of memorising raw Meson/Ninja commands
- The `build/` directory is on `$PATH` in mise-managed shells, so compiled binaries are directly accessible
- No changes to `src/`, `tests/`, `data/`, `codegen/`, or any build configuration
