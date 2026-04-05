## ADDED Requirements

### Requirement: Spec catalog index exists
The file `openspec/specs/README.md` SHALL exist and SHALL list every spec currently in `openspec/specs/`, grouped by domain.

#### Scenario: Every spec folder is represented
- **WHEN** a developer opens `openspec/specs/README.md`
- **THEN** every subdirectory of `openspec/specs/` (except `README.md` itself) SHALL have an entry in the index under its assigned domain

### Requirement: Domain taxonomy is defined
The catalog SHALL define a fixed set of domain names with a one-sentence description of each. Recognised domains SHALL be: **Infrastructure**, **Data**, **Game Core**, **Game Logic**, **Engine**, **Agents**, **Analysis**.

#### Scenario: Each domain has a description
- **WHEN** a developer reads the catalog
- **THEN** each domain heading SHALL be accompanied by a description explaining what kinds of specs belong there

### Requirement: Naming convention is documented
The catalog SHALL document the kebab-case naming convention for spec folders and provide guidance on which domain to place a new spec in.

#### Scenario: New spec name can be determined without ambiguity
- **WHEN** a developer is creating a new spec for a capability
- **THEN** the catalog SHALL provide enough guidance to choose a spec folder name and domain without needing to ask someone else

### Requirement: Catalog is kept up to date
Every change that adds or removes a spec SHALL include a task step to update `openspec/specs/README.md`.

#### Scenario: Change tasks include a catalog update step
- **WHEN** a change's `tasks.md` introduces a new spec
- **THEN** the tasks list SHALL include an explicit step: "Update `openspec/specs/README.md` to add the new spec under its domain"
