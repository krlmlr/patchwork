## MODIFIED Requirements

### Requirement: YAML catalog is single source of truth
The file `data/patches.yaml` SHALL be the canonical definition of all Patchwork patches. No patch data SHALL be hardcoded anywhere in the C++ source directly.

#### Scenario: Each patch has required fields *(updated)*
- **WHEN** any patch entry is read from the catalog
- **THEN** it has: integer `id` (1–33), single-character string `name` (ASCII letter or digit), integer `buttons` (placement cost in buttons), integer `time` (placement cost in time), integer `income` (buttons earned per income phase), and a multiline `shape` string

#### Scenario: All patch names are unique single characters *(new)*
- **WHEN** the `name` field of all 33 patch entries is read
- **THEN** every value is a single ASCII letter or digit (`[A-Za-z0-9]`), and no two entries share the same value

#### Scenario: Patch shapes are in canonical form *(new)*
- **WHEN** any `shape` value is read from the catalog
- **THEN** it equals the lexicographically minimal grid string obtainable by any rotation (0°/90°/180°/270°) or reflection of that tile; equivalently, no other orientation of the same tile would produce a smaller string under lexicographic comparison of the newline-joined rows

#### Scenario: Catalog entries are sorted by size and cost *(new)*
- **WHEN** all 33 entries are read in ID order
- **THEN** they appear in non-decreasing order of cell count, and within each cell-count group in non-decreasing order of button cost, and within each button-cost group in non-increasing order of income

### Requirement: Generated header exposes patch array
`src/generated/patches.hpp` SHALL define a `constexpr` array of patch data accessible at compile time.

#### Scenario: All patches accessible at compile time *(updated)*
- **WHEN** the generated header is included
- **THEN** a `constexpr` collection of exactly 33 `PatchData` entries is available, each with `id`, `name`, `buttons`, `time`, `income`, and `cells` (list of `(row, col)` offsets from the top-left of the bounding box)

#### Scenario: Patch name matches catalog *(new)*
- **WHEN** any patch is accessed from the generated array
- **THEN** its `name` field contains the single character specified in the corresponding entry in `data/patches.yaml`
