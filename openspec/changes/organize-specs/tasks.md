## 1. Restructure Specs to Domain-Level

- [x] 1.1 Create `openspec/specs/infrastructure/spec.md` (merge: build-system, devcontainer, mise-tasks, r-ppm-install)
- [x] 1.2 Create `openspec/specs/data/spec.md` (merge: patch-catalog, game-glossary)
- [x] 1.3 Create `openspec/specs/game-core/spec.md` (merge: game-state, simplified-game-state, game-setup)
- [x] 1.4 Create `openspec/specs/game-logic/spec.md` (merge: move-generation, move-application, terminal-and-scoring)
- [x] 1.5 Create `openspec/specs/engine/spec.md` (merge: play-driver, game-logger)
- [x] 1.6 Create `openspec/specs/tui/spec.md` (merge: tui-display, tui-input, tui-launch, tui-undo-redo)
- [x] 1.7 Create `openspec/specs/agents/spec.md` (merge: random-agent)
- [x] 1.8 Remove all 19 old capability-level spec folders

## 2. Update Active Change Deltas

- [x] 2.1 Update `r-package-structure/specs/game-setup/` → `r-package-structure/specs/game-core/` (ADDED requirement for setups codegen script)
- [x] 2.2 Update `r-package-structure/specs/patch-catalog/` → `r-package-structure/specs/data/` (ADDED requirement for patches codegen script)
- [x] 2.3 Update `r-package-structure/specs/r-package-infra/` → `r-package-structure/specs/infrastructure/` (ADDED requirements for DESCRIPTION, NAMESPACE, R/ directory)
- [x] 2.4 Update `organize-specs/specs/spec-catalog/` → `organize-specs/specs/infrastructure/` (ADDED requirements for spec catalog index, taxonomy, decision rules, naming convention)

## 3. Create Spec Catalog README

- [x] 3.1 Update `openspec/specs/README.md` with 8-domain table (add TUI) linking to domain spec files
- [x] 3.2 Add summary description for each domain
- [x] 3.3 Add 8 numbered decision rules including TUI rule
- [x] 3.4 Update naming convention section

## 4. Verify Completeness

- [x] 4.1 Confirm every subdirectory in `openspec/specs/` has an entry in the README
- [x] 4.2 Confirm no old capability-level spec folder names remain in `openspec/specs/`
