## Context

The project builds with Meson + Ninja. `BUILD.md` documents the three commands a developer needs: `meson setup build`, `ninja -C build`, and `meson test -C build`. mise is a polyglot dev tool manager that reads `.mise.toml` to expose named tasks and manage per-project environment variables. Adding a `.mise.toml` makes the workflow discoverable without duplicating the underlying commands.

## Goals / Non-Goals

**Goals:**

- Expose the three build workflow steps as named `mise run` tasks
- Put the `build/` output directory on `$PATH` in mise-managed shells so compiled binaries are accessible without a full path
- Keep the task definitions in sync with `BUILD.md` (same underlying commands)

**Non-Goals:**

- Replacing or removing `BUILD.md` — manual instructions remain the primary reference
- Adding mise-managed tool version pinning (e.g., pinning Meson or Ninja versions via `[tools]`) — not needed at this stage
- Supporting environments without mise — the file is silently ignored by non-mise toolchains

## Decisions

### Three tasks matching BUILD.md exactly

Tasks map one-to-one to the commands in `BUILD.md`: `meson setup build`, `ninja -C build`, and `meson test -C build`. No abstraction layer is added; the tasks are thin aliases so there is no drift risk between the docs and the tool config.

**Alternative considered:** A single `mise run all` composite task — rejected because developers often run only one step (e.g., re-running tests without rebuilding).

### `[env] _.path` for build directory

mise's `_.path` mechanism appends entries to `$PATH` only inside mise-managed shells. Using it for `build/` means compiled binaries are reachable by name without requiring `./build/` prefixes or manual `export PATH` steps.

**Alternative considered:** A wrapper task that sets `PATH` inline — more verbose and only applies during the task, not across the entire shell session.

### No `[tools]` section

Pinning Meson and Ninja versions via mise's tool management (`[tools]`) would require developers to install them through mise instead of their system package manager. The project's build prerequisites are documented in `BUILD.md`; mise is an optional convenience layer, not the authoritative tool installer.
