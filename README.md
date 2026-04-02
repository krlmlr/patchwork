# Patchwork Engine

A C++ game engine for [Uwe Rosenberg's Patchwork](https://boardgamegeek.com/boardgame/163412/patchwork), used as a case study in modern C++ idioms, game-tree search, and reinforcement learning.

## Prerequisites

| Tool | Version | Purpose |
|------|---------|---------|
| [Meson](https://mesonbuild.com/) | ≥ 1.3 | Build system |
| [Ninja](https://ninja-build.org/) | ≥ 1.11 | Build backend |
| C++23 compiler | GCC ≥ 14 or Clang ≥ 17 | Engine and tests |
| [R](https://www.r-project.org/) | ≥ 4.3 | Codegen (dev-time only) |
| [clang-format](https://clang.llvm.org/docs/ClangFormat.html) | ≥ 18 | C++ formatting |

R packages (install once with `install.packages(c("yaml"))`):

- `yaml`

## Build

```sh
meson setup build
meson compile -C build
```

## Test

```sh
meson test -C build
```

Verbose output:

```sh
meson test -C build --verbose
```

## Codegen

The patch catalog (`data/patches.yaml`) is the single source of truth for all 33 patches.
Run the R script to regenerate `src/generated/patches.hpp` after editing the catalog:

```sh
Rscript codegen/generate_patches.R
```

Commit the resulting header alongside any catalog changes.

## Code Formatting

C++ sources are formatted with `clang-format` using the project `.clang-format` config:

```sh
# Check
clang-format --dry-run --Werror src/**/*.hpp src/**/*.cpp tests/*.cpp

# Apply
clang-format -i src/**/*.hpp src/**/*.cpp tests/*.cpp
```

Markdown files are linted with [`markdownlint-cli2`](https://github.com/DavidAnson/markdownlint-cli2):

```sh
npx markdownlint-cli2 "**/*.md"
```

## Directory Structure

```
data/              Patch catalog (YAML)
codegen/           R codegen scripts
src/               C++ engine headers
  generated/       Auto-generated headers (do not edit)
tests/             Catch2 unit tests
logs/              Runtime logs (gitignored)
openspec/          Project spec and task tracking
```
