## ADDED Requirements

### Requirement: Glossary YAML is the single source of truth for game terminology

The file `data/glossary.yaml` SHALL be the canonical definition of all game and engine terminology. It SHALL be human- and machine-readable. No new term SHALL be introduced in specs, code comments, or agent prompts without a corresponding entry in `data/glossary.yaml`.

#### Scenario: Glossary file exists and is valid YAML

- **WHEN** `data/glossary.yaml` is loaded by any YAML parser
- **THEN** it parses without error

#### Scenario: Each entry has required fields

- **WHEN** any entry in `data/glossary.yaml` is read
- **THEN** it has: a non-empty string `term`, a non-empty string `definition`, and a non-empty string `category` that is one of `game-rules`, `engine`, or `data`

#### Scenario: Optional fields are well-typed when present

- **WHEN** an entry in `data/glossary.yaml` includes optional fields
- **THEN** `aliases` (if present) is a list of strings, and `see_also` (if present) is a list of strings that each match the `term` of another entry in the file

#### Scenario: All terms are unique

- **WHEN** all `term` values in `data/glossary.yaml` are read
- **THEN** no two entries share the same `term` value (case-insensitive)

#### Scenario: Glossary contains initial game-rules terms

- **WHEN** `data/glossary.yaml` is loaded
- **THEN** it contains entries for at minimum: patch, patch circle, time track, quilt board, button income, button balance, leather patch, 7×7 bonus, advance move, buy move, income phase, starting player marker, neutral piece

#### Scenario: Glossary contains initial engine terms

- **WHEN** `data/glossary.yaml` is loaded
- **THEN** it contains entries for at minimum: game state, player state, move, legal move, terminal state, random agent, game log

### Requirement: Human-readable glossary document exists

The file `docs/glossary.md` SHALL provide a human-readable rendering of all glossary terms, organised by category. It SHALL be kept in sync with `data/glossary.yaml`.

#### Scenario: Markdown glossary is navigable

- **WHEN** `docs/glossary.md` is opened in a Markdown renderer
- **THEN** it displays all terms with their definitions, grouped by category, with a table of contents or heading structure allowing navigation

#### Scenario: Markdown glossary matches YAML source

- **WHEN** `docs/glossary.md` and `data/glossary.yaml` are both read
- **THEN** every term present in the YAML appears in the Markdown with the same definition

### Requirement: README links to the glossary

The `README.md` project structure table SHALL include an entry for `data/glossary.yaml` and `docs/glossary.md`, and SHALL include a prose reference to the glossary in the project overview.

#### Scenario: README contains glossary entry in structure table

- **WHEN** `README.md` is read
- **THEN** it contains a table row or reference for `data/glossary.yaml` describing its purpose

#### Scenario: README contains glossary entry for docs

- **WHEN** `README.md` is read
- **THEN** it contains a reference to `docs/glossary.md` as the human-readable rendering

### Requirement: OpenSpec context references the glossary

The `openspec/config.yaml` context block SHALL include a reference to `data/glossary.yaml` and `docs/glossary.md` so that AI agents read the glossary before proposing or implementing changes.

#### Scenario: OpenSpec config mentions the glossary

- **WHEN** `openspec/config.yaml` is read
- **THEN** it contains a reference to `data/glossary.yaml` in its `context` field, instructing AI to consult the glossary for domain terminology

### Requirement: Agent prompts reference the glossary

At least the OpenSpec apply prompt (`opsx-apply.prompt.md`) SHALL reference `data/glossary.yaml` or `docs/glossary.md` so that implementing agents use canonical terminology.

#### Scenario: Apply prompt mentions the glossary

- **WHEN** `.github/prompts/opsx-apply.prompt.md` is read
- **THEN** it contains a reference to the glossary file(s)
