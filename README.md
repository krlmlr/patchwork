# patchwork

A C++23 implementation of the patchwork board game.

## Quick start (zero-effort setup)

Open this repository in **GitHub Codespaces**, any
**devcontainer-capable editor** (VS Code with the Dev Containers extension,
JetBrains IDEs), or a **GitHub Copilot cloud session**. The
`.devcontainer/devcontainer.json` configuration will automatically install
every prerequisite — compiler, build tools, R, OpenSpec, and formatters —
so you can build and test immediately:

```sh
mise run setup
mise run test
```

If you don't have [mise](https://mise.jdx.dev/) installed, use the underlying
commands directly (see [BUILD.md](BUILD.md)).

## Manual setup

See [BUILD.md](BUILD.md) for step-by-step instructions if you are not using a
devcontainer.

The shared tool installer can also be run directly on a plain Ubuntu 24.04
system:

```sh
sudo bash scripts/install-tools.sh
```

## Project structure

| Path | Purpose |
|------|---------|
| `src/` | C++ source and header files |
| `tests/` | Catch2 unit tests |
| `data/` | Patch data in YAML format |
| `codegen/` | R script that generates `src/generated/patches.hpp` |
| `scripts/` | Helper scripts (tool installer, etc.) |
| `.devcontainer/` | Devcontainer configuration for Codespaces / Copilot |
| `.github/workflows/` | GitHub Actions CI and Copilot agent setup |
| `openspec/` | OpenSpec change artifacts (proposals, designs, tasks) |

## Code formatting

**C++**: clang-format is configured in `.clang-format`. Run it with:

```sh
mise run format
```

**Markdown**: markdownlint-cli2 is configured in `.markdownlint.yml`. Run it
with:

```sh
mise run lint
```

## OpenSpec

This project uses [OpenSpec](https://github.com/Fission-AI/OpenSpec) for
structured change management. Change proposals, designs, and task lists live
under `openspec/changes/`. To interact with them:

```sh
openspec status          # list all changes and their progress
openspec status --change <name>   # status for a single change
```

## Contributing

1. Open an issue or start an OpenSpec change (`openspec new change <name>`)
2. Discuss the proposal
3. Implement the tasks
4. Open a pull request; CI will run the build and tests automatically
