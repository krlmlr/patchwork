#!/usr/bin/env Rscript
# Generate src/generated/patches.hpp from data/patches.yaml
#
# Usage: Rscript codegen/generate_patches.R
#   Run from the project root directory.

pkgload::load_all(quiet = TRUE)
generate_patches("src/generated/patches.hpp")
