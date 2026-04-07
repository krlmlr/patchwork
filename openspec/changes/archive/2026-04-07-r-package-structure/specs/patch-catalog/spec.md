## ADDED Requirements

### Requirement: R codegen script is a thin entry point
The file `codegen/generate_patches.R` SHALL contain only: a call to `pkgload::load_all(quiet = TRUE)`, any path or argument setup, and a single call to the top-level `generate_patches()` function defined in `R/patches.R`. All logic (spec assertions, shape helpers, code emission) SHALL reside in `R/patches.R`.

#### Scenario: Script delegates to package function
- **WHEN** `Rscript codegen/generate_patches.R` is run from the project root
- **THEN** `pkgload::load_all()` is called first and then `generate_patches()` is invoked
- **AND** the generated `cpp/generated/patches.hpp` is byte-for-byte identical to what the original script produced
