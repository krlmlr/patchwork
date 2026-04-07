# Building patchwork

## Zero-effort setup

Open this repository in **GitHub Codespaces**, a **devcontainer-capable
editor** (VS Code with the Dev Containers extension), or a **GitHub Copilot
cloud session**. The `.devcontainer/devcontainer.json` configuration
automatically installs all prerequisites listed below, so you can start
building immediately without any manual steps.

Alternatively, run the shared installer on a plain Ubuntu 24.04 system:

```sh
sudo bash scripts/install-tools.sh
```

## Manual prerequisites

- C++23-capable compiler (Clang 16+ or GCC 13+)
- [Meson](https://mesonbuild.com/) ≥ 1.0
- [Ninja](https://ninja-build.org/)
- [Catch2](https://github.com/catchorg/Catch2) ≥ 3.x (or let Meson fetch it as a subproject)

Install on macOS with Homebrew:

```sh
brew install meson ninja catch2
```

## Setup

Configure the build directory (only needed once):

```sh
mise run setup
```

<details><summary>Without mise</summary>

```sh
meson setup build
```

To reconfigure an existing build directory: `meson setup build --reconfigure`

</details>

## Build

```sh
mise run build
```

<details><summary>Without mise</summary>

```sh
ninja -C build
```

</details>

## Run Tests

```sh
mise run test
```

<details><summary>Without mise</summary>

```sh
meson test -C build
```

For verbose output: `meson test -C build -v`

</details>

## Code Generation

Patch data in `cpp/generated/patches.hpp` is auto-generated from `data/patches.yaml`.
To regenerate it:

```sh
mise run codegen
```

<details><summary>Without mise</summary>

```sh
Rscript codegen/generate_patches.R
```

Requires R with `yaml` and `pkgload` packages (installed automatically by `pak::pak()`).

</details>
