## ADDED Requirements

### Requirement: Spec catalog index exists
The file `openspec/specs/README.md` SHALL exist and SHALL list every spec currently in `openspec/specs/`, grouped by domain.

#### Scenario: Every spec folder is represented
- **WHEN** a developer opens `openspec/specs/README.md`
- **THEN** every subdirectory of `openspec/specs/` (excluding `README.md` itself) SHALL have an entry in the catalog under its assigned domain

### Requirement: Domain taxonomy is defined
The catalog SHALL define exactly seven domains with a one-sentence description of each: **Infrastructure**, **Data**, **Game Core**, **Game Logic**, **Engine**, **Agents**, **Analysis**.

#### Scenario: Each domain has a description
- **WHEN** a developer reads the catalog
- **THEN** each domain heading SHALL be accompanied by a description explaining what kinds of specs belong there

### Requirement: Decision rules assign each spec to exactly one domain
The catalog SHALL provide numbered, unambiguous decision rules so that a new spec can be assigned to exactly one domain without discussion. The rules SHALL cover all seven domains and SHALL specify a tiebreaker for ambiguous cases.

#### Scenario: Unambiguous assignment for a new spec
- **WHEN** a developer needs to assign a new spec to a domain
- **THEN** following the decision rules in order SHALL yield exactly one domain with no remaining ambiguity

#### Scenario: Cross-cutting specs have a defined tiebreaker
- **WHEN** a spec appears to fit two domains
- **THEN** the decision rules SHALL specify which domain wins

### Requirement: In-progress specs are listed
The catalog SHALL also list all specs that exist in active changes (not yet archived) alongside their proposed domain, so the full intended capability surface is visible.

#### Scenario: Simplified-rules specs are visible in catalog
- **WHEN** a developer reads the catalog before `simplified-rules` is archived
- **THEN** the catalog SHALL show game-logger, move-application, move-generation, play-driver, random-agent, simplified-game-state, and terminal-and-scoring with their domains

### Requirement: Naming convention is documented
The catalog SHALL document the kebab-case naming convention for spec folders and provide guidance on deriving a spec name from a capability description.

#### Scenario: New spec name can be determined without ambiguity
- **WHEN** a developer is creating a new spec for a capability
- **THEN** the catalog SHALL provide enough guidance to choose a spec folder name and domain without needing to ask someone else

### Requirement: Catalog is kept up to date
Every change that adds or removes a spec SHALL include a task step to update `openspec/specs/README.md`.

#### Scenario: Change tasks include a catalog update step
- **WHEN** a change's `tasks.md` introduces a new spec
- **THEN** the tasks list SHALL include an explicit step: "Update `openspec/specs/README.md` to add the new spec under its domain"
