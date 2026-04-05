## Context

Game concepts such as "patch," "time track," "button income," and "quilt board" are used throughout source code, OpenSpec specs, and AI agent prompts — but without a single authoritative reference. Contributors (human and AI) must infer term meanings from context. As the project grows (more specs, agents, analysis scripts), inconsistent terminology creates confusion and drift.

## Goals / Non-Goals

**Goals:**
- A canonical, human-readable Markdown glossary (`docs/glossary.md`) covering current and forward-looking game and engine terminology
- Integration into OpenSpec context so AI agents automatically consult the glossary
- Links from `README.md` and relevant agent prompts

**Non-Goals:**
- Machine-readable structured data (no YAML, no codegen from glossary)
- Automated validation of term usage in code
- Translating the glossary to other languages
- Documenting implementation-internal details (e.g., bit packing layout) — only game/engine concepts visible at the API level

## Decisions

### Decision: Markdown only, no parallel YAML source

**Choice:** `docs/glossary.md` is the single file. No `data/glossary.yaml`.

**Rationale:** The glossary is authored by humans and read by humans and AI alike. Markdown is already the project's documentation format and is directly readable in every tool (browser, editor, AI context window) without parsing. A YAML-plus-Markdown dual-source setup adds synchronisation overhead and tooling complexity with no immediate benefit — there is no current consumer of structured glossary data. If machine-readable access is needed in the future, the Markdown file can be parsed or replaced then.

### Decision: Group terms by category with a table of contents

**Choice:** Sections — **Game Rules**, **Engine**, **Data & Tooling** — each with a definition list. A short table of contents at the top.

**Rationale:** Grouping by category makes the glossary navigable and mirrors how contributors think about the codebase. Flat alphabetical lists are harder to scan when looking for related terms.

### Decision: Include forward-looking terms from the roadmap

**Choice:** The initial glossary covers not only terms currently in the codebase but also terms that will be needed for upcoming roadmap phases (agents, search algorithms, RL).

**Rationale:** Defining terms early avoids ambiguity when those phases are implemented. A glossary that only reflects current code would need frequent updates; a glossary that reflects the full problem domain is more stable and more useful for AI agents planning future changes.

### Decision: Glossary terms are independent of rulebook terms; rulebook terms are noted as references

**Choice:** The glossary uses canonical names suited to this codebase. Where the physical rulebook uses a different name for the same concept, the rulebook term is noted in parentheses as `[rulebook: "…"]` and treated as an alias, not the canonical name. Our canonical names MUST NOT conflict in meaning with rulebook names: if a term appears in the rulebook with a different meaning than we assign it, it is renamed here to avoid ambiguity.

**Rationale:** The rulebook targets physical-game players and uses informal language. Our glossary targets contributors and AI agents working on a digital engine. Some rulebook terms are ambiguous or overloaded (e.g., "special patch" refers to both the leather patch and the bonus tile in some printings). Using independent canonical names avoids inheriting that ambiguity while still giving readers a bridge to the rulebook through the noted references.

## Proposed Initial Glossary

The following is the proposed initial content for `docs/glossary.md`, grouped by category. This list is the starting point; it will be expanded as new concepts are introduced.

### Game Rules

**patch** (also: tile)
A fabric piece that players purchase and place on their quilt board. Each patch has a button cost, a time cost, a button income value, and a shape.

**patch circle**
The ring of 33 patches arranged around the time board at the start of the game. Players may always choose from the three patches immediately clockwise of the circle marker.

**time track** (also: time board)
The linear track numbered 0–53 on which players advance their time tokens. The player furthest behind always takes the next turn.

**time token**
A player's marker on the time track. Its position represents how far into the game that player has progressed.

**quilt board**
The 9×9 grid each player fills with patches. Empty spaces cost 2 points at end of game.

**button cost** (also: patch cost)
The number of buttons a player must pay to purchase a patch. Listed on the patch label.

**time cost** (also: time advance)
The number of spaces a player's time token advances when purchasing a patch. Listed on the patch label.

**button income** (also: income)
The number of buttons a patch contributes each time the player passes a button-income space on the time track. Also refers to the total income earned across all placed patches.

**button balance** (also: buttons)
The current number of buttons a player holds. Players start with 5 buttons.

**income phase** (also: payout; [rulebook: "Button Income"])
The event triggered when a player's time token passes or lands on a button-symbol space on the time track. The player receives buttons equal to the total button income of all patches on their quilt board. Also called a payout.

**leather patch** (also: 1×1 patch; [rulebook: "special patch"])
A 1×1 patch awarded to a player when their time token passes one of the five marked spaces on the time track. Placed immediately on the quilt board. Used to fill single-cell gaps. Not to be confused with the bonus tile (also occasionally called "special tile" in some rulebook printings).

**circle marker** ([rulebook: "neutral token"])
An index (0–32) into the patch circle that indicates the position from which the three available patches are counted. After a patch is taken, the circle marker advances to the position just after the taken patch. In the engine, stored as a 6-bit integer in the shared game state.

**buy window** (also: window)
The set of up to three patches currently available for purchase: the patches at positions marker, marker+1, and marker+2 (mod 33) in the patch circle. At least one patch is always available unless the patch circle is empty. The buy window shifts after each purchase because the marker advances past the taken patch.

**advance move** (also: action A; [rulebook: "Advance and Receive Buttons"])
The move where the active player moves their time token to the space directly in front of the opponent's token and receives one button per space advanced.

**buy move** (also: action B; [rulebook: "Take and Place a Patch"])
The move where the active player purchases one patch from the buy window, pays its button cost, places it on their quilt board, and advances their time token by its time cost.

**bonus tile** ([rulebook: "special tile"])
A tile worth 7 points awarded to the first player to completely fill a 7×7 area (49 cells) on their quilt board. In the engine, its ownership is tracked by the `BonusStatus` enum. Sometimes called the "7×7 bonus."

**active player**
The player whose turn it is to move. Determined by turn order. At the start of the game the active player is the starting player; thereafter, the active player changes according to time-track positions after each move. See also: starting player, turn order.

**starting player**
The player who acts as the active player on the very first turn of the game. In the physical game, the player who last used a needle; in the engine, the player whose token is placed on top when both start on space 0 (i.e., player 0 in the engine's convention).

**turn order**
The active player is always the player whose time token is furthest behind on the time track. If both tokens are on the same space, the player whose token is on top (established by who last moved onto that space) acts first. Because a buy move may advance the active player past the opponent, the same player can take multiple consecutive turns.

**patch gain**
The net effect on a player's final score from acquiring a specific patch, assuming it can be placed without displacing other patches. Composed of two parts:

- **placement gain** = 2 × (squares covered by the patch) − button cost. Placing the patch removes that many empty spaces (each worth −2 at scoring) and costs the button cost in buttons (each worth +1 at scoring), giving a net of 2 × coverage − cost.
- **projected income** = tile income × (number of payouts remaining). The buttons the patch will yield over the rest of the game.

Total patch gain = placement gain + projected income. Patch gain is an engine-level concept used for evaluation; it is not defined in the physical rulebook.

**scoring**
Final score = buttons held + 7 (if holding the bonus tile) − 2 × (number of empty quilt board spaces). The player with the highest score wins; tiebreaker is the player who reached space 53 first.

During the game (before both players reach space 53), a projected score can be estimated by adding projected income from remaining payouts to the current buttons and board state. This is useful for evaluation functions.

### Engine

**game state**
A complete snapshot of the game at a point in time, containing both players' states and the shared state (patch availability, circle marker position, bonus status).

**player state**
The per-player portion of the game state: quilt board occupancy, time-track position, button balance, and button income.

**simplified game state**
A variant of game state that tracks free spaces as a single integer instead of the full quilt board grid. Used for the current simplified ruleset.

**move**
A single player action taken by the active player: either an advance move or a buy move.

**legal move**
A move that is valid in the current game state. A buy move is legal if the player has enough buttons and the patch is available; an advance move is always legal unless the game is terminal.

**terminal state**
A game state in which both players have reached or passed position 53 on the time track. No further moves are possible; the game is over.

**game setup**
The initial configuration of a game: the starting arrangement of the patch circle (a permutation of the 33 patch IDs) and the random seed used to generate it.

**game log**
An NDJSON file recording all moves, state transitions, and outcomes of a game. Used for analysis and reproducibility.

**random agent**
An agent that selects uniformly at random among all legal moves. Serves as a baseline for benchmarking.

**heuristic agent**
An agent that scores positions with a hand-crafted evaluation function and picks the highest-scoring legal move.

**evaluation function**
A numeric scoring of a non-terminal game state used by heuristic and search agents to estimate the value of a position.

**minimax**
A game-tree search algorithm that minimises the maximum loss, assuming the opponent plays optimally.

**alpha-beta pruning**
An optimisation for minimax that prunes branches that cannot affect the final decision, reducing the number of nodes evaluated.

**iterative deepening**
A depth-limited search strategy that increases the search depth incrementally, enabling time-bounded search.

**MCTS** (Monte Carlo Tree Search)
A rollout-based search algorithm that builds a game tree by sampling random (or policy-guided) playouts and uses visit statistics to select moves.

**UCB1** (Upper Confidence Bound 1)
The bandit formula used in MCTS to balance exploration of less-visited nodes and exploitation of high-value nodes.

**rollout policy**
The strategy used to complete a game from a non-terminal node during MCTS simulation. Can be random or guided by a learned policy.

**policy network**
A learned model that outputs a probability distribution over legal moves from a given game state. Used to guide MCTS.

**value network**
A learned model that estimates the expected outcome (win/loss/score) from a given game state.

**self-play**
Training data generation where an agent plays against itself to produce game records for learning.

**Elo rating**
A relative skill ranking assigned to agents based on head-to-head game outcomes. Used to track agent improvement over training.

### Data & Tooling

**patch catalog**
The file `data/patches.yaml` — the single source of truth for all 33 patches, including their IDs, names, costs, incomes, and shapes.

**game setups**
Stored configurations of patch circle arrangements in `data/setups/`, used for reproducible experiments.

**codegen**
The R script (`codegen/generate_patches.R`) that reads `data/patches.yaml` and writes the committed C++ header `src/generated/patches.hpp`.

**NDJSON** (Newline-Delimited JSON)
The format used for game logs: one JSON object per line, enabling streaming and easy analysis with tools like DuckDB.

## Risks / Trade-offs

- **Glossary drift** → Mitigation: New concepts introduced in specs or code should be added to the glossary in the same change. The `game-glossary` spec includes this as a requirement.
- **Markdown is harder to parse programmatically than YAML** → Accepted trade-off for now. If structured access is needed, the Markdown can be parsed or replaced with YAML at that time.
- **Initial list may be incomplete** → Expected and by design. The glossary is explicitly "initial, to be expanded."

## Open Questions

- Should there be a lint rule or CI check verifying that new terms in specs appear in the glossary? (Deferred to a future change.)

