## MODIFIED Requirements

### Requirement: Each patch has required fields

The file `data/patches.yaml` SHALL be the canonical definition of all Patchwork patches. Each entry SHALL have: integer `id` (1–33), single-character string `name` (ASCII letter or digit), integer `buttons` (placement cost in buttons as defined in `data/glossary.yaml`), integer `time` (placement cost in time as defined in `data/glossary.yaml`), integer `income` (buttons earned per income phase as defined in `data/glossary.yaml`), and a multiline `shape` string. No patch data SHALL be hardcoded anywhere in the C++ source directly.

#### Scenario: Catalog contains all patches

- **WHEN** the catalog file is loaded
- **THEN** it contains exactly 33 patch entries

#### Scenario: Each patch has required fields

- **WHEN** any patch entry is read from the catalog
- **THEN** it has: integer `id` (1–33), single-character string `name` (ASCII letter or digit), integer `buttons` (placement cost in buttons), integer `time` (placement cost in time), integer `income` (buttons earned per income phase), and a multiline `shape` string
- **AND** the R codegen script SHALL assert that each field is present and has the correct type before generating the header

#### Scenario: Field names match glossary terms

- **WHEN** the field names `buttons`, `time`, and `income` in `data/patches.yaml` are cross-referenced with `data/glossary.yaml`
- **THEN** each field name corresponds to a glossary term or alias in the `game-rules` or `data` category
