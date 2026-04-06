## Why

The R code in `codegen/` consists of standalone scripts with no shared infrastructure — functions are re-implemented or copy-pasted, and there is no consistent way to load shared utilities, run tests, or depend on packages. Adopting R package structure (loadable via `pkgload::load_all()`) gives us namespacing, auto-loading of functions, `testthat` integration, and standard dependency declaration without requiring installation or full `R CMD check` compliance.

## What Changes

- Add a `DESCRIPTION` file at the project root (or under an `R/` subfolder) declaring the package name, version, and R package dependencies (`yaml`, etc.)
- Create an `R/` directory to hold shared R functions extracted from `codegen/` scripts
- Move reusable logic (shape parsing, canonical-form computation, spec assertions, setup generation helpers) from `codegen/generate_patches.R` and `codegen/generate_setups.R` into `R/` as named functions
- Refactor `codegen/` scripts to be thin entry-point scripts that call `pkgload::load_all()` and then invoke the package functions
- Add a `NAMESPACE` file (minimal, since we are not building for CRAN)
- Optionally add `tests/testthat/` structure so R unit tests can be run with `testthat::test_local()`

## Capabilities

### New Capabilities

- `r-package-infra`: Package skeleton (`DESCRIPTION`, `NAMESPACE`, `R/`) enabling `pkgload::load_all()` to load all shared R utilities

### Modified Capabilities

- `patch-catalog`: The R codegen logic for patch catalog generation is refactored — spec assertions and shape helpers move to package functions; the script becomes a thin caller. Behavior is unchanged.
- `game-setup`: The R codegen logic for game setup generation is refactored similarly — setup helpers move to package functions; the script becomes a thin caller. Behavior is unchanged.

## Impact

- `codegen/generate_patches.R` and `codegen/generate_setups.R`: refactored to thin scripts
- New files: `DESCRIPTION`, `NAMESPACE`, `R/patches.R`, `R/setups.R` (or similar)
- No changes to generated C++ output (`cpp/generated/`)
- No changes to `data/` files
- Downstream: future R analysis scripts (Tile Analysis phase, logging) will benefit from shared utilities loaded via `pkgload::load_all()`
