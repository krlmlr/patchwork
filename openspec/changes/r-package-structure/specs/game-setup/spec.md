## MODIFIED Requirements

### Requirement: R codegen script is a thin entry point
The file `codegen/generate_setups.R` SHALL contain only: a call to `pkgload::load_all(quiet = TRUE)`, any path or argument setup, and a single call to the top-level `generate_setups()` function defined in `R/setups.R`. All logic (spec assertions, permutation generation, code emission) SHALL reside in `R/setups.R`.

#### Scenario: Script delegates to package function
- **WHEN** `Rscript codegen/generate_setups.R` is run from the project root
- **THEN** `pkgload::load_all()` is called first and then `generate_setups()` is invoked
- **AND** the generated `cpp/generated/game_setups.hpp` is byte-for-byte identical to what the original script produced
