## 1. Create YAML Glossary

- [ ] 1.1 Create `docs/` directory if it does not exist
- [ ] 1.2 Create `data/glossary.yaml` with schema fields: `term`, `definition`, `aliases` (optional), `see_also` (optional), `category`
- [ ] 1.3 Add game-rules category entries: patch, patch circle, time track, quilt board, button income, button balance, leather patch, 7×7 bonus, advance move, buy move, income phase, starting player marker, neutral piece
- [ ] 1.4 Add engine category entries: game state, player state, move, legal move, terminal state, random agent, game log
- [ ] 1.5 Add data category entries: patch catalog, game setup, YAML catalog, codegen

## 2. Create Human-Readable Markdown Glossary

- [ ] 2.1 Create `docs/glossary.md` with a table of contents and terms grouped by category
- [ ] 2.2 Ensure every term in `data/glossary.yaml` appears in `docs/glossary.md` with the same definition
- [ ] 2.3 Add a note at the top of `docs/glossary.md` that `data/glossary.yaml` is the canonical source and both files should be kept in sync

## 3. Update README

- [ ] 3.1 Add `data/glossary.yaml` row to the project structure table in `README.md`
- [ ] 3.2 Add `docs/glossary.md` row to the project structure table in `README.md`
- [ ] 3.3 Add a prose reference to the glossary in the README overview section (e.g., "See [docs/glossary.md](docs/glossary.md) for definitions of all game and engine terms.")

## 4. Update OpenSpec Context

- [ ] 4.1 Add a reference to `data/glossary.yaml` and `docs/glossary.md` in the `context` block of `openspec/config.yaml`, instructing AI agents to consult the glossary for domain terminology

## 5. Update Agent Prompts

- [ ] 5.1 Add a reference to the glossary in `.github/prompts/opsx-apply.prompt.md` so implementing agents use canonical terminology
- [ ] 5.2 Review other prompt files (opsx-propose, opsx-ff, opsx-verify, opsx-explore) and add a glossary reference where appropriate
