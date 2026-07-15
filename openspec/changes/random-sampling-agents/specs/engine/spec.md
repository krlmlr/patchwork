## RENAMED Requirements

- FROM: `### Requirement: Play driver runs a complete game between two random agents and writes a log`
- TO: `### Requirement: Play driver runs a complete game between two agents and writes a log`

## MODIFIED Requirements

### Requirement: Play driver runs a complete game between two agents and writes a log

The play driver executable SHALL accept `--seed1 <n>` and `--seed2 <n>` (independent RNG seeds for player 0 and player 1 respectively; each defaults to `42`), an optional `--setup <id>` argument (default `0`), optional `--agent1 <strategy>` / `--agent2 <strategy>` command-line arguments, and an optional `--advance-weight <w>` argument (a strictly positive floating-point value, default `1.0`) that controls how strongly the biased agents prefer the advance move. Every argument has a default, so a bare invocation runs a complete game. When an `--agentN` flag is given the corresponding player uses the named strategy; when omitted, that player defaults to `random` (uniform). Valid strategy names are `random`, `cheap`, `income`, `income-per-time`. `--advance-weight` has no effect when both agents use the `random` strategy. An optional `--output <file>` argument SHALL redirect the log to a file instead of stdout.

#### Scenario: Game terminates and log is written

- **WHEN** the play driver is invoked with valid `--seed1`, `--seed2`, and `--setup` arguments
- **THEN** it exits with code 0 and the output contains exactly one `game_start` line, one or more `move` lines, and exactly one `game_end` line

#### Scenario: Bare invocation uses all defaults

- **WHEN** the play driver is invoked with no arguments
- **THEN** it exits with code 0 and runs setup `0`, seeds `42`/`42`, agents `random`/`random`, and advance weight `1.0`

#### Scenario: Same seeds and setup produce identical log

- **WHEN** the play driver is invoked twice with the same `--seed1`, `--seed2`, `--setup`, `--agent1`, `--agent2`, and `--advance-weight` values
- **THEN** the two output logs are byte-for-byte identical

#### Scenario: Output redirected to file with --output

- **WHEN** `--output game.ndjson` is given
- **THEN** the log is written to `game.ndjson` and nothing is written to stdout

#### Scenario: --agent1 and --agent2 select independent strategies

- **WHEN** the play driver is invoked with `--agent1 cheap --agent2 income`
- **THEN** player 0 uses the cheap-biased agent and player 1 uses the income-biased agent

#### Scenario: Invalid agent name is rejected

- **WHEN** the play driver is invoked with `--agent1 unknown`
- **THEN** it exits with a non-zero code and prints an error to stderr

#### Scenario: Non-positive advance weight is rejected

- **WHEN** the play driver is invoked with `--advance-weight 0` or `--advance-weight -1.0`
- **THEN** it exits with a non-zero code and prints an error to stderr

### Requirement: Play driver reports an error for invalid arguments

Because `--seed1`, `--seed2`, `--setup`, and `--advance-weight` all have defaults, none of them is mandatory and omitting any of them is not an error. The play driver SHALL print a usage message to stderr and exit with a non-zero code only when a supplied value is malformed: a non-numeric `--seed1` or `--seed2`, a `--setup` that is non-numeric or does not name a valid setup, a non-numeric or non-positive `--advance-weight`, or an unrecognized `--agent1` / `--agent2` strategy name.

#### Scenario: Omitting all optional arguments is not an error

- **WHEN** the play driver is invoked without `--seed1`, `--seed2`, `--setup`, `--agent1`, `--agent2`, or `--advance-weight`
- **THEN** it exits with code 0 using the default values, and does not print a usage message

#### Scenario: Non-numeric seed is rejected

- **WHEN** the play driver is invoked with `--seed1 abc`
- **THEN** it prints a usage message to stderr and exits with a non-zero code

#### Scenario: Invalid setup is rejected

- **WHEN** the play driver is invoked with `--setup 200` (out of range) or `--setup foo` (non-numeric)
- **THEN** it prints a usage message to stderr and exits with a non-zero code

### Requirement: Game-start event is logged when a game begins

The logger SHALL write a JSON object line (NDJSON) of type `"game_start"` immediately before the first move is applied. The event SHALL include: `event` (`"game_start"`), `setup_id` (integer or string), an initial state summary with both players' starting `buttons`, `income`, and `free_spaces`, and a `circle` field — a 33-character string of single-character patch names in circle order from the `GameSetup` (the `'2'` two-square tile is always the last character, by neutral-token convention). It SHALL additionally include `"agent_p0"` and `"agent_p1"` (strings recording the strategy name for each player), `"seed_p0"` and `"seed_p1"` (integers recording each player's seed), and an `"advance_weight"` field (number recording the advance-weight value used).

The per-player `seed_p0` / `seed_p1` fields **supersede the single `seed` field** of the prior requirement: with two independent RNG streams there is no longer a single game seed, so the play driver records one seed per player and SHALL NOT emit a `seed` field. The `circle` field is carried over unchanged from the richer-logging change (#26). Together, setup + circle + both strategies + both seeds + advance weight make every game fully reproducible from the `game_start` line alone.

#### Scenario: Game-start line is well-formed JSON

- **WHEN** a game begins and the logger is attached
- **THEN** the first line written is valid JSON with `"event": "game_start"` and no `"seed"` field

#### Scenario: Game-start includes 33-character circle string

- **WHEN** `log_game_start` is called with a valid `GameSetup`
- **THEN** the emitted JSON line contains a `"circle"` field whose value is a 33-character string of patch-name characters in `GameSetup` circle order, and the last character SHALL be `'2'` (the two-square patch, by neutral-token convention)

#### Scenario: Circle string encodes patch names not IDs

- **WHEN** the circle is serialised at game start
- **THEN** each character SHALL be the single-character patch name (as stored in `kPatches[id].name`), not the numeric patch ID

#### Scenario: Agent, seed, and advance-weight fields present in game-start

- **WHEN** the play driver runs with `--agent1 income --seed1 7 --agent2 cheap --seed2 99 --advance-weight 1.5`
- **THEN** the `game_start` line contains `"agent_p0": "income"`, `"agent_p1": "cheap"`, `"seed_p0": 7`, `"seed_p1": 99`, `"advance_weight": 1.5`, and no `"seed"` field

#### Scenario: Default values recorded when flags are omitted

- **WHEN** the play driver runs without `--agent1`, `--agent2`, `--seed1`, `--seed2`, `--setup`, or `--advance-weight`
- **THEN** the `game_start` line contains `"setup_id": 0`, `"agent_p0": "random"`, `"agent_p1": "random"`, `"seed_p0": 42`, `"seed_p1": 42`, `"advance_weight": 1.0`
