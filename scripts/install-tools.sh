#!/usr/bin/env bash
# install-tools.sh — installs all prerequisites for building, testing,
# formatting, and working with the patchwork project.
# Suitable for Ubuntu 24.04 (Noble) and compatible derivatives.
# Run as root (or with sudo) or in a container that already has sudo.

set -euo pipefail

# ---------------------------------------------------------------------------
# Package manager packages
# ---------------------------------------------------------------------------
apt-get update -y
apt-get install -y --no-install-recommends \
    git \
    ca-certificates \
    catch2 \
    clang-format \
    r-base \
    r-cran-yaml \
    nodejs \
    npm

# ---------------------------------------------------------------------------
# R package manager: pak (from Posit Package Manager binary repo)
# ---------------------------------------------------------------------------
Rscript -e 'options(repos = c(CRAN = sprintf("https://p3m.dev/cran/latest/bin/linux/noble-%s/%s", R.version["arch"], substr(getRversion(), 1, 3)))); install.packages("pak"); library(pak)'

# ---------------------------------------------------------------------------
# Python-based build tools (pipx gives latest Meson/Ninja in isolated envs)
# ---------------------------------------------------------------------------
apt-get install -y --no-install-recommends pipx
PIPX_HOME=/opt/pipx PIPX_BIN_DIR=/usr/local/bin pipx install meson
PIPX_HOME=/opt/pipx PIPX_BIN_DIR=/usr/local/bin pipx install ninja

# ---------------------------------------------------------------------------
# Node.js-based tools: OpenSpec CLI and Markdown linter
# ---------------------------------------------------------------------------
npm install -g @fission-ai/openspec@latest markdownlint-cli2

# ---------------------------------------------------------------------------
# Smoke tests — verify each tool is accessible and print its version
# ---------------------------------------------------------------------------
echo ""
echo "=== Tool versions ==="

set -x

meson --version
ninja --version
Rscript --version
clang-format --version
node --version
npm --version
openspec --version
# markdownlint-cli2 doesn't have a --version flag but prints its version in the first line of --help output
markdownlint-cli2 --help | head -n 1 || true
echo "=== All tools installed successfully ==="
