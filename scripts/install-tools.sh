#!/usr/bin/env bash
# install-tools.sh — installs all prerequisites for building, testing,
# formatting, and working with the patchwork project.
# Suitable for Ubuntu 24.04 (Noble) and compatible derivatives.
# Run as a regular user with sudo privileges; the script invokes sudo as needed.
# (rig invokes sudo internally for system-level R installation steps.)

set -euo pipefail

# ---------------------------------------------------------------------------
# Package manager packages
# ---------------------------------------------------------------------------
sudo apt-get update -y
sudo apt-get install -y --no-install-recommends \
    git \
    ca-certificates \
    catch2 \
    clang-format

# ---------------------------------------------------------------------------
# R — rig manages its own sudo for system-level steps
# ---------------------------------------------------------------------------
curl -Ls https://github.com/r-lib/rig/releases/download/latest/rig-linux-$(arch)-latest.tar.gz | sudo tar xz -C /usr/local
rig add release
rig system setup-user-lib
rig system add-pak

# ---------------------------------------------------------------------------
# Configure R to use PPM pre-built binaries for Ubuntu 24.04 (noble)
# Both options are required: repos tells R where to download from; HTTPUserAgent
# tells PPM this is a Linux R client so it serves binaries instead of sources.
# Rprofile.site is owned by root, so sudo tee -a is required.
# ---------------------------------------------------------------------------
R_PROFILE_SITE=$(Rscript --no-save --no-restore -e 'cat(file.path(R.home("etc"), "Rprofile.site"))')
sudo tee -a "$R_PROFILE_SITE" > /dev/null <<'REOF'
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
# DBI binary-install smoke test — verify PPM serves a pre-built binary.
# Placed immediately after R/pak setup to fail fast if PPM is misconfigured.
# pak outputs a line like "DBI 1.2.2 (x86_64-pc-linux-gnu-ubuntu-24.04)" for
# binary installs; source installs would show "R CMD INSTALL" instead.
# DBI is removed first so a fresh download always happens (making the binary
# marker visible regardless of prior state).
# ---------------------------------------------------------------------------
dbi_output=$(Rscript --no-save --no-restore -e '
if (requireNamespace("DBI", quietly = TRUE)) remove.packages("DBI")
pak::pkg_install("DBI", ask = FALSE)
' 2>&1)
printf '%s\n' "$dbi_output"
printf '%s\n' "$dbi_output" | grep -qE "DBI.*(linux-gnu-ubuntu|binary)" || \
  { printf 'ERROR: DBI was not installed as a PPM binary\n' >&2; exit 1; }

# ---------------------------------------------------------------------------
# Python-based build tools (pipx gives latest Meson/Ninja in isolated envs)
# ---------------------------------------------------------------------------
sudo apt-get install -y --no-install-recommends pipx
sudo env PIPX_HOME=/opt/pipx PIPX_BIN_DIR=/usr/local/bin pipx install meson
sudo env PIPX_HOME=/opt/pipx PIPX_BIN_DIR=/usr/local/bin pipx install ninja

# ---------------------------------------------------------------------------
# Node.js-based tools: OpenSpec CLI and Markdown linter
# ---------------------------------------------------------------------------
sudo npm install -g @fission-ai/openspec@latest markdownlint-cli2

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
