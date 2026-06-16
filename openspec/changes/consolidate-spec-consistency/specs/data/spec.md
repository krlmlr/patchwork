## MODIFIED Requirements

### Requirement: YAML catalog is single source of truth
The file `data/patches.yaml` SHALL be the canonical definition of all Patchwork patches. All patch data that appears in C++ SHALL be produced by the codegen pipeline from `data/patches.yaml` and live only in the committed generated header under `cpp/generated/`. No patch data SHALL be hand-written or maintained directly in hand-edited C++ source, and the generated header SHALL NOT be edited by hand.

#### Scenario: Catalog contains all patches
- **WHEN** the catalog file is loaded
- **THEN** it contains exactly 33 patch entries

#### Scenario: Each patch has required fields
- **WHEN** any patch entry is read from the catalog
- **THEN** it has: integer `id` (1–33), single-character string `name` (ASCII letter or digit), integer `buttons` (placement cost in buttons), integer `time` (placement cost in time), integer `income` (buttons earned per income phase), and a multiline `shape` string
- **AND** the R codegen script SHALL assert that each field is present and has the correct type before generating the header

#### Scenario: All patch names are unique single characters
- **WHEN** the `name` field of all 33 patch entries is read
- **THEN** every value is a single ASCII letter or digit (`[A-Za-z0-9]`), and no two entries share the same value
- **AND** the R codegen script SHALL assert uniqueness and the single-character constraint before generating the header

#### Scenario: Patch shapes are in canonical form
- **WHEN** any `shape` value is read from the catalog
- **THEN** it equals the canonical form for that tile: among all 8 orientations (4 rotations × 2 reflections), the one whose normalised `(row, col)` cell coordinates, sorted and serialised as `"row,col"` pairs joined by `;`, are lexicographically smallest
- **AND** the R codegen script SHALL assert this property for every entry before generating the header

#### Scenario: Catalog entries are sorted by size and cost
- **WHEN** all 33 entries are read in ID order
- **THEN** they appear in non-decreasing order of cell count, and within each cell-count group in non-decreasing order of button cost, and within each button-cost group in non-increasing order of income
- **AND** the R codegen script SHALL assert this ordering before generating the header

### Requirement: Generated header exposes patch array
`cpp/generated/patches.hpp` SHALL define a `constexpr` array of patch data accessible at compile time.

#### Scenario: All patches accessible at compile time
- **WHEN** the generated header is included
- **THEN** a `constexpr` collection of exactly 33 `PatchData` entries is available, each with `id`, `name`, `buttons`, `time`, `income`, `num_cells`, and `cells` — where `cells` is a fixed-size `std::array<CellOffset, 8>` of `(row, col)` offsets from the top-left of the bounding box, of which the first `num_cells` entries are valid and any remaining entries are `{0, 0}`

#### Scenario: Patch data matches catalog
- **WHEN** any patch is accessed from the generated array
- **THEN** its `buttons`, `time`, and `income` values match the corresponding entry in `data/patches.yaml`

#### Scenario: Patch name matches catalog
- **WHEN** any patch is accessed from the generated array
- **THEN** its `name` field contains the single character specified in the corresponding entry in `data/patches.yaml`
