#!/usr/bin/env bash
set -euo pipefail

sudo apt-get update && sudo apt-get install -y r-base r-cran-yaml

sudo pip3 install --upgrade meson ninja

# Smoke-test: verify each tool is reachable and print its version
meson --version
ninja --version
Rscript --version
