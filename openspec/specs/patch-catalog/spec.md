### Requirement: YAML catalog is single source of truth
The file `data/patches.yaml` SHALL be the canonical definition of all Patchwork patches. No patch data SHALL be hardcoded anywhere in the C++ source directly.

#### Scenario: Catalog contains all patches
- **WHEN** the catalog file is loaded
- **THEN** it contains exactly 33 patch entries

#### Scenario: Each patch has required fields
- **WHEN** any patch entry is read from the catalog
- **THEN** it has: integer `id` (1–33), integer `buttons` (placement cost in buttons), integer `time` (placement cost in time), integer `income` (buttons earned per income phase), and a multiline `shape` string

### Requirement: ASCII art shape encoding
Each patch shape SHALL be encoded as a multiline string of `.` (empty) and `X` (occupied) characters on a rectangular grid. The grid SHALL be the minimal bounding box of the patch.

#### Scenario: Shape parses to correct cell count
- **WHEN** a patch shape string is parsed
- **THEN** the number of `X` characters equals the number of tiles in that patch

#### Scenario: Shape grid is rectangular
- **WHEN** a patch shape string is parsed
- **THEN** all rows have equal length

### Requirement: R codegen produces committed C++ header
The script `codegen/generate_patches.R` SHALL read `data/patches.yaml` and write `src/generated/patches.hpp`. The generated file SHALL be committed to version control.

#### Scenario: Codegen runs without errors
- **WHEN** `codegen/generate_patches.R` is executed with R
- **THEN** it exits without error and `src/generated/patches.hpp` is created or updated

#### Scenario: Generated header is valid C++
- **WHEN** `src/generated/patches.hpp` is included in a C++ translation unit
- **THEN** it compiles without errors

### Requirement: Generated header exposes patch array
`src/generated/patches.hpp` SHALL define a `constexpr` array of patch data accessible at compile time.

#### Scenario: All patches accessible at compile time
- **WHEN** the generated header is included
- **THEN** a `constexpr` collection of exactly 33 `PatchData` entries is available, each with `id`, `buttons`, `time`, `income`, and `cells` (list of `(row, col)` offsets from the top-left of the bounding box)

#### Scenario: Patch data matches catalog
- **WHEN** any patch is accessed from the generated array
- **THEN** its `buttons`, `time`, and `income` values match the corresponding entry in `data/patches.yaml`
