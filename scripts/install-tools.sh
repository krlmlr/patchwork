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
    clang-format

# ---------------------------------------------------------------------------
# R
# ---------------------------------------------------------------------------
curl -Ls https://github.com/r-lib/rig/releases/download/latest/rig-linux-$(arch)-latest.tar.gz | tar xz -C /usr/local
rig add release
# Set up the per-user R library and install pak into the system library so
# that pak is available when running as root (e.g. during sudo invocations).
rig system setup-user-lib
rig system add-pak

# ---------------------------------------------------------------------------
# Configure R to use PPM pre-built binaries for Ubuntu 24.04 (noble)
# Both options are required: repos tells R where to download from; HTTPUserAgent
# tells PPM this is a Linux R client so it serves binaries instead of sources.
# ---------------------------------------------------------------------------
R_PROFILE_SITE=$(Rscript --no-save --no-restore -e 'cat(file.path(R.home("etc"), "Rprofile.site"))')
cat >> "$R_PROFILE_SITE" <<'REOF'
options(
  repos = c(PPM = "https://packagemanager.posit.co/cran/__linux__/noble/latest"),
  HTTPUserAgent = sprintf(
    "R/%s R (%s)",
    getRversion(),
    paste(getRversion(), R.version$platform, R.version$arch, R.version$os)
  )
)
REOF

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

# DBI binary-install smoke test — verify PPM serves a pre-built binary.
# pak outputs a line like "DBI 1.2.2 (x86_64-pc-linux-gnu-ubuntu-24.04)" for
# binary installs; source installs would show "R CMD INSTALL" instead.
dbi_output=$(Rscript --no-save --no-restore -e 'pak::pkg_install("DBI", ask = FALSE)' 2>&1)
printf '%s\n' "$dbi_output"
printf '%s\n' "$dbi_output" | grep -qE "DBI.*(linux-gnu-ubuntu|binary)" || \
  { printf 'ERROR: DBI was not installed as a PPM binary\n' >&2; exit 1; }

echo "=== All tools installed successfully ==="
