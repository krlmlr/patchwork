## 1. Create Spec Catalog

- [ ] 1.1 Create `openspec/specs/README.md` with domain taxonomy headings: Infrastructure, Data, Game Core, Game Logic, Engine, Agents, Analysis
- [ ] 1.2 Add a one-sentence description for each domain
- [ ] 1.3 Add numbered decision rules (one per domain) for assigning any new spec to exactly one domain; include a tiebreaker rule for ambiguous cases
- [ ] 1.4 List every existing spec under its domain with a one-line description: build-system, devcontainer, mise-tasks → Infrastructure; patch-catalog → Data; game-setup, game-state, simplified-game-state → Game Core
- [ ] 1.5 List in-progress specs from `simplified-rules` with their domains: move-generation, move-application, terminal-and-scoring → Game Logic; game-logger, play-driver → Engine; random-agent → Agents; simplified-game-state (delta) → Game Core
- [ ] 1.6 Document the kebab-case naming convention and guidance for deriving a spec name from a capability description

## 2. Verify Completeness

- [ ] 2.1 Check that every subdirectory in `openspec/specs/` has an entry in the README
- [ ] 2.2 Confirm README renders correctly on GitHub (check headings, links, formatting)
