## Context

The project builds with Meson + Ninja and includes an R-based codegen step. `BUILD.md` documents every command a developer needs. mise is a polyglot dev tool manager that reads `.mise.toml` to expose named tasks and manage per-project environment variables. The principle governing this change is: **every useful action in this repository should be runnable through mise**. This makes the workflow discoverable by both humans and automated tools (e.g., Copilot coding agents) without duplicating or restating the underlying commands.

## Goals / Non-Goals

**Goals:**

- Establish the invariant: every useful project action has a corresponding `mise run <task>` entry
- Expose all current useful workflow steps as named tasks: `setup`, `build`, `test`, `codegen`
- Put the `build/` output directory on `$PATH` in mise-managed shells so compiled binaries are accessible without a full path
- Keep task definitions in sync with `BUILD.md` (same underlying commands)
- Require discoverability: when project documentation exists, it SHALL reference mise tasks as the primary way to invoke project actions

**Non-Goals:**

- Replacing or removing `BUILD.md` — manual instructions remain the primary reference
- Adding mise-managed tool version pinning (e.g., pinning Meson or Ninja versions via `[tools]`) — not needed at this stage
- Supporting environments without mise — the file is silently ignored by non-mise toolchains

## Decisions

### All useful actions, not just build tasks

The set of tasks is not bounded by "build workflow" but by "useful project actions". Any command a developer or agent would reasonably want to run — build, test, codegen, future lint, format, benchmark — belongs in `.mise.toml`. This prevents ad-hoc shell commands from being the only way to run uncommon but important actions. When a new capability is added to the project, its mise task is part of the same change.

**Alternative considered:** Limiting mise to the core build loop (`setup`/`build`/`test`) and treating other scripts as optional extras — rejected because it undermines discoverability and creates a two-tier system where some actions are "first class" and others require knowing the right `Rscript` or `python` invocation.

### Task granularity: one task per meaningful action

Tasks map one-to-one to distinct user-visible actions. No composite `mise run all` task is added, because developers often run only one step (e.g., re-running tests without rebuilding). Composites can be added later if a CI or scripting need emerges.

### `[env] _.path` for build directory

mise's `_.path` mechanism appends entries to `$PATH` only inside mise-managed shells. Using it for `build/` means compiled binaries are reachable by name without requiring `./build/` prefixes or manual `export PATH` steps.

**Alternative considered:** A wrapper task that sets `PATH` inline — more verbose and only applies during the task, not across the entire shell session.

### Discoverability in documentation is a hard requirement (future)

Once the project has end-user or developer documentation (separate from `BUILD.md`), that documentation SHALL reference `mise run <task>` as the canonical invocation for every documented action. The raw commands remain visible as the implementation detail, but mise is the entry point. This requirement is tracked in the spec now so it is not forgotten when documentation specs are written.

### No `[tools]` section

Pinning Meson and Ninja versions via mise's tool management (`[tools]`) would require developers to install them through mise instead of their system package manager. The project's build prerequisites are documented in `BUILD.md`; mise is an optional convenience layer, not the authoritative tool installer.
