## ADDED Requirements

### Requirement: DESCRIPTION file declares the R package
The project root SHALL contain a `DESCRIPTION` file with at minimum the fields `Package`, `Version`, `Title`, `Description`, `License`, and `Imports` (listing `yaml`). The `Package` field SHALL NOT be `patchwork` (to avoid collision with the CRAN package of the same name).

#### Scenario: pkgload::load_all() succeeds from project root
- **WHEN** `pkgload::load_all()` is called from the project root
- **THEN** all functions defined in `R/*.R` are available in the current session without error

#### Scenario: DESCRIPTION declares yaml as a dependency
- **WHEN** the `DESCRIPTION` file is read
- **THEN** `yaml` appears in the `Imports` field

### Requirement: NAMESPACE file enables function export
The project root SHALL contain a `NAMESPACE` file that exports all public R functions (those not starting with `.`).

#### Scenario: Public functions are accessible after load_all
- **WHEN** `pkgload::load_all()` is called
- **THEN** functions defined in `R/` that do not start with `.` are accessible by name in the calling environment

### Requirement: R/ directory holds shared package functions
An `R/` directory SHALL exist at the project root and contain at minimum `patches.R` (patch-related helpers) and `setups.R` (setup-related helpers).

#### Scenario: R/ contains patches.R and setups.R
- **WHEN** the `R/` directory is listed
- **THEN** both `patches.R` and `setups.R` are present

#### Scenario: Patch helper functions are defined in R/patches.R
- **WHEN** `pkgload::load_all()` is called
- **THEN** the functions `parse_cells`, `rotate90`, `reflect_h`, `normalise_cells`, `cell_key`, `cells_to_rows`, `canonical_shape`, `parse_shape`, and `count_x` are available

#### Scenario: Setup helper functions are defined in R/setups.R
- **WHEN** `pkgload::load_all()` is called
- **THEN** the functions `generate_setups` and `generate_patches` (or equivalently named top-level generators) are available
