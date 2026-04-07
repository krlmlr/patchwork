## 1. Package Skeleton

- [x] 1.1 Create `DESCRIPTION` at the project root with fields: `Package: patchworkengine`, `Version: 0.0.1`, `Title`, `Description`, `License: MIT`, and `Imports: yaml`
- [x] 1.2 Create `NAMESPACE` at the project root with `exportPattern("^[^\\.]")`
- [x] 1.3 Create the `R/` directory at the project root

## 2. Extract Patch Helpers

- [x] 2.1 Create `R/patches.R` containing all helper functions extracted from `codegen/generate_patches.R`: `count_x`, `rotate90`, `reflect_h`, `normalise_cells`, `cell_key`, `cells_to_rows`, `parse_cells`, `canonical_shape`, `parse_shape`
- [x] 2.2 Add a top-level `generate_patches(output_path)` function in `R/patches.R` that encapsulates the full catalog validation, spec assertions, and C++ header emission logic currently in `codegen/generate_patches.R`

## 3. Extract Setup Helpers

- [x] 3.1 Create `R/setups.R` containing all helper functions extracted from `codegen/generate_setups.R`
- [x] 3.2 Add a top-level `generate_setups(output_path, n_setups)` function in `R/setups.R` that encapsulates the full permutation generation and C++ header emission logic currently in `codegen/generate_setups.R`

## 4. Refactor Entry-Point Scripts

- [x] 4.1 Rewrite `codegen/generate_patches.R` to: call `pkgload::load_all(quiet = TRUE)`, then call `generate_patches("cpp/generated/patches.hpp")`
- [x] 4.2 Rewrite `codegen/generate_setups.R` to: call `pkgload::load_all(quiet = TRUE)`, then call `generate_setups("cpp/generated/game_setups.hpp", n_setups = 100L)`

## 5. Verification

- [x] 5.1 Run `Rscript codegen/generate_patches.R` from the project root and confirm `cpp/generated/patches.hpp` is byte-for-byte identical to the previously committed version
- [x] 5.2 Run `Rscript codegen/generate_setups.R` from the project root and confirm `cpp/generated/game_setups.hpp` is byte-for-byte identical to the previously committed version
- [x] 5.3 Confirm `pkgload::load_all()` succeeds without errors from the project root in an interactive R session
