## ADDED Requirements

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
- **THEN** it contains definitions for at minimum: patch, patch circle, time track, quilt board, button cost, time cost, button income, button balance, leather patch, circle marker, advance move, buy move, income phase, bonus tile, active player, starting player, turn order, patch gain, scoring

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
