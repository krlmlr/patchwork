## Context

The `openspec/specs/` directory holds the canonical, living specification of every system capability. Specs accumulate as phases are completed — each archived change promotes its specs into `openspec/specs/`. With seven specs already present and more arriving each phase, the flat directory is increasingly hard to navigate and gives AI agents no signal about domain groupings, naming conventions, or where to place new specs.

Without explicit decision rules, a contributor proposing a new spec (human or AI) must guess which domain applies, resulting in:
- Inconsistent spec names (no signal whether a spec is about the engine, an agent, or the data layer)
- Ambiguous domain placement for cross-cutting concerns (e.g. "does a logging spec go in Engine or Infrastructure?")
- No single reference that enumerates all current capabilities in one place

The fix is a `README.md` in `openspec/specs/` that acts as the authoritative catalog and decision guide.

## Goals / Non-Goals

**Goals:**
- Define a fixed domain taxonomy aligned with roadmap phases.
- State unambiguous decision rules so any new spec can be assigned to exactly one domain without discussion.
- List every current spec (including in-progress `simplified-rules` specs) with its domain.
- Document the kebab-case naming convention.

**Non-Goals:**
- Moving or renaming existing spec folders. The `openspec archive` command writes new specs to `openspec/specs/<name>/spec.md` (flat). Delta specs in active changes reference canonical spec names via that flat path. Reorganising the filesystem would break `openspec list --specs`, `openspec archive`, and any delta spec that targets an existing canonical spec (e.g. `simplified-game-state` in `simplified-rules`). The README provides logical grouping without touching file paths.
- Automating spec discovery or generating the index programmatically.
- Enforcing conventions via tooling (convention-by-documentation is sufficient at this scale).

## Decisions

**Decision: README.md in `openspec/specs/`**
A plain Markdown file is the lightest possible solution. It is human-readable in editors and on GitHub, AI-readable as plain text, and requires no tooling. Alternatives (a YAML manifest, a generated HTML page) add complexity with no benefit at the current scale.

**Decision: Domain taxonomy based on roadmap phases**
The roadmap already partitions the project into coherent phases. Using the same language makes it easy to trace roadmap phase → change → specs. Seven domains are defined:

| Domain | What belongs here |
|--------|------------------|
| **Infrastructure** | Build system, developer tooling, devcontainer, task runners |
| **Data** | Canonical data files, code-generation pipelines, patch catalog |
| **Game Core** | Fundamental game-state types and initial setup structures |
| **Game Logic** | Rules: legal moves, move application, terminal detection, scoring |
| **Engine** | Game loop, play drivers, logging, reproducibility plumbing |
| **Agents** | Any concrete agent (random, heuristic, MCTS, RL) |
| **Analysis** | R analysis scripts, plot outputs, statistical summaries |

**Decision rules (one domain per spec)**
1. If the spec is about build, CI, developer environment, or task automation → **Infrastructure**
2. If the spec is about canonical data files or code generation from data → **Data**
3. If the spec defines a game-state *type* or the initial arrangement of the game → **Game Core**
4. If the spec defines *what is legal* or *what happens* when a move is applied, or how the game ends → **Game Logic**
5. If the spec is about running a game end-to-end (loop, driver, logging, seed/setup plumbing) → **Engine**
6. If the spec defines a concrete decision-making strategy or agent interface → **Agents**
7. If the spec is about offline analysis, plotting, or statistical summaries of game data → **Analysis**

If a spec seems to fit two domains, apply the highest-numbered rule that fits (rules are listed in specificity order, most specific last). If still ambiguous, prefer the narrower domain.

**Decision: Flat spec folders remain unchanged**
See Non-Goals above for the technical rationale.

## Risks / Trade-offs

- [Risk] README can drift out of sync as new specs are added. → Mitigation: each future change's `tasks.md` MUST include a step to update the catalog.
- [Risk] Domain boundaries are sometimes ambiguous. → Mitigation: numbered decision rules are the tiebreaker; edge cases can be re-classified cheaply since only the README changes.
