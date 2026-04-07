# Data Specification

## Purpose
Defines the canonical data files that are the single source of truth for game content, the code-generation pipelines that produce committed C++ headers from those files, and the shared game terminology glossary.

## Requirements

### Requirement: YAML catalog is single source of truth
The file `data/patches.yaml` SHALL be the canonical definition of all Patchwork patches. No patch data SHALL be hardcoded anywhere in the C++ source directly.

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
- **THEN** it equals the canonical form for that tile: the grid string (rows joined by newline) produced by the orientation — among all 8 (4 rotations × 2 reflections) — whose normalised `(row, col)` cell coordinates are lexicographically smallest when sorted and compared as a sequence of `"row,col"` pairs joined by `;`; equivalently, the orientation that produces the widest (most columns) bounding box when there is a tie, and where `(0,1) < (1,0)` so horizontal shapes are preferred over vertical ones
- **AND** the R codegen script SHALL assert this property for every entry before generating the header

#### Scenario: Catalog entries are sorted by size and cost
- **WHEN** all 33 entries are read in ID order
- **THEN** they appear in non-decreasing order of cell count, and within each cell-count group in non-decreasing order of button cost, and within each button-cost group in non-increasing order of income
- **AND** the R codegen script SHALL assert this ordering before generating the header

### Requirement: ASCII art shape encoding
Each patch shape SHALL be encoded as a multiline string of `.` (empty) and `X` (occupied) characters on a rectangular grid. The grid SHALL be the minimal bounding box of the patch.

#### Scenario: Shape parses to correct cell count
- **WHEN** a patch shape string is parsed
- **THEN** the number of `X` characters equals the number of tiles in that patch

#### Scenario: Shape grid is rectangular
- **WHEN** a patch shape string is parsed
- **THEN** all rows have equal length

### Requirement: R codegen produces committed C++ header
The script `codegen/generate_patches.R` SHALL read `data/patches.yaml` and write `cpp/generated/patches.hpp`. The generated file SHALL be committed to version control.

#### Scenario: Codegen runs without errors
- **WHEN** `codegen/generate_patches.R` is executed with R
- **THEN** it exits without error and `cpp/generated/patches.hpp` is created or updated

#### Scenario: Generated header is valid C++
- **WHEN** `cpp/generated/patches.hpp` is included in a C++ translation unit
- **THEN** it compiles without errors

### Requirement: Generated header exposes patch array
`cpp/generated/patches.hpp` SHALL define a `constexpr` array of patch data accessible at compile time.

#### Scenario: All patches accessible at compile time
- **WHEN** the generated header is included
- **THEN** a `constexpr` collection of exactly 33 `PatchData` entries is available, each with `id`, `name`, `buttons`, `time`, `income`, and `cells` (list of `(row, col)` offsets from the top-left of the bounding box)

#### Scenario: Patch data matches catalog
- **WHEN** any patch is accessed from the generated array
- **THEN** its `buttons`, `time`, and `income` values match the corresponding entry in `data/patches.yaml`

#### Scenario: Patch name matches catalog
- **WHEN** any patch is accessed from the generated array
- **THEN** its `name` field contains the single character specified in the corresponding entry in `data/patches.yaml`

### Requirement: Glossary Markdown file is the single source of truth for game terminology

The file `docs/glossary.md` SHALL be the canonical definition of all game and engine terminology. No new term SHALL be introduced in specs, code comments, or agent prompts without a corresponding entry in `docs/glossary.md`.

#### Scenario: Glossary file exists and is valid Markdown

- **WHEN** `docs/glossary.md` is opened in a Markdown renderer
- **THEN** it renders without error, displaying all terms with their definitions

#### Scenario: Glossary is organised by category with a table of contents

- **WHEN** `docs/glossary.md` is read
- **THEN** it contains a table of contents and terms grouped into at least the categories: Game Rules, Engine, Data & Tooling

#### Scenario: Glossary contains initial game-rules terms

- **WHEN** `docs/glossary.md` is read
- **THEN** it contains definitions for at minimum: patch, patch circle, time track, quilt board, button cost, time cost, button income, button balance, leather, marker, buy window, advance move, buy move, income phase, bonus tile, active player, starting player, turn order, patch gain, scoring

#### Scenario: Rulebook term aliases are noted

- **WHEN** any term in `docs/glossary.md` has a different name in the physical rulebook
- **THEN** the rulebook term is noted as `[rulebook: "…"]` alongside the canonical name, and is not used as the canonical term

#### Scenario: Glossary contains initial engine terms

- **WHEN** `docs/glossary.md` is read
- **THEN** it contains definitions for at minimum: game state, player state, simplified game state, move, legal move, terminal state, active player, game setup, game log, random agent

#### Scenario: Glossary contains forward-looking engine terms

- **WHEN** `docs/glossary.md` is read
- **THEN** it contains definitions for at minimum: heuristic agent, evaluation function, minimax, alpha-beta pruning, iterative deepening, MCTS, UCB1, rollout policy, policy network, value network, self-play, Elo rating

#### Scenario: Glossary contains data and tooling terms

- **WHEN** `docs/glossary.md` is read
- **THEN** it contains definitions for at minimum: patch catalog, game setups, codegen, NDJSON

### Requirement: README links to the glossary

The `README.md` project structure table SHALL include an entry for `docs/glossary.md` and SHALL include a prose reference to the glossary.

#### Scenario: README contains glossary entry in structure table

- **WHEN** `README.md` is read
- **THEN** it contains a row referencing `docs/glossary.md` with a description of its purpose

#### Scenario: README contains glossary prose reference

- **WHEN** `README.md` is read
- **THEN** it contains a link or reference to `docs/glossary.md` in the project overview or documentation section

### Requirement: OpenSpec context references the glossary

The `openspec/config.yaml` context block SHALL include a reference to `docs/glossary.md` so that AI agents read the glossary before proposing or implementing changes.

#### Scenario: OpenSpec config mentions the glossary

- **WHEN** `openspec/config.yaml` is read
- **THEN** it contains a reference to `docs/glossary.md` in its `context` field, instructing AI to consult the glossary for domain terminology

### Requirement: Agent prompts reference the glossary

At least the OpenSpec apply prompt (`opsx-apply.prompt.md`) SHALL reference `docs/glossary.md` so that implementing agents use canonical terminology.

#### Scenario: Apply prompt mentions the glossary

- **WHEN** `.github/prompts/opsx-apply.prompt.md` is read
- **THEN** it contains a reference to `docs/glossary.md`
