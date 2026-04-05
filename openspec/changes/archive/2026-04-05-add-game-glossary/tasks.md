## 1. Create Markdown Glossary

- [x] 1.1 Create `docs/` directory
- [x] 1.2 Create `docs/glossary.md` with a table of contents and terms grouped by category: Game Rules, Engine, Data & Tooling; note rulebook aliases with `[rulebook: "…"]` where the canonical name differs from the rulebook term
- [x] 1.3 Add Game Rules terms from the design: patch, patch circle, time track, time token, quilt board, button cost, time cost, button income, button balance, income phase (alias: payout), leather (also: leather patch), marker (also: circle marker), buy window, advance move, buy move, bonus tile, active player, starting player, turn order, patch gain, scoring
- [x] 1.4 Add Engine terms from the design (current): game state, player state, simplified game state, move, legal move, terminal state, game setup, game log, random agent
- [x] 1.5 Add Engine terms from the design (forward-looking): heuristic agent, evaluation function, minimax, alpha-beta pruning, iterative deepening, MCTS, UCB1, rollout policy, policy network, value network, self-play, Elo rating
- [x] 1.6 Add Data & Tooling terms from the design: patch catalog, game setups, codegen, NDJSON

## 2. Update README

- [x] 2.1 Add `docs/glossary.md` row to the project structure table in `README.md`
- [x] 2.2 Add a prose reference to the glossary (e.g., "See [docs/glossary.md](docs/glossary.md) for definitions of all game and engine terms.")

## 3. Update OpenSpec Context

- [x] 3.1 Add a reference to `docs/glossary.md` in the `context` block of `openspec/config.yaml`, instructing AI agents to consult the glossary for domain terminology

## 4. Update Agent Prompts

- [x] 4.1 Add a reference to `docs/glossary.md` in `.github/prompts/opsx-apply.prompt.md`
- [x] 4.2 Review other prompt files (opsx-propose, opsx-ff, opsx-verify, opsx-explore) and add a glossary reference where appropriate
