#!/usr/bin/env Rscript
# Generate cpp/generated/game_setups.hpp from data/patches.yaml
#
# Usage: Rscript codegen/generate_setups.R
#   Run from the project root directory.

pkgload::load_all(quiet = TRUE)
generate_setups("cpp/generated/game_setups.hpp", n_setups = 100L)
