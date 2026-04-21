## MODIFIED Requirements

### Requirement: Play driver runs a complete game between two agents and writes a log

The play driver executable SHALL accept `--seed1 <n>` and `--seed2 <n>` (independent RNG seeds for player 0 and player 1 respectively; each defaults to `42`), `--setup <id>`, optional `--agent1 <strategy>` / `--agent2 <strategy>` command-line arguments, and an optional `--advance-weight <w>` argument (a strictly positive floating-point value, default `1.0`) that controls how strongly the biased agents prefer the advance move. When an `--agentN` flag is given the corresponding player uses the named strategy; when omitted, that player defaults to `random` (uniform). Valid strategy names are `random`, `cheap`, `income`, `income-per-time`. `--advance-weight` has no effect when both agents use the `random` strategy. An optional `--output <file>` argument SHALL redirect the log to a file instead of stdout.

#### Scenario: Game terminates and log is written

- **WHEN** the play driver is invoked with valid `--seed1`, `--seed2`, and `--setup` arguments
- **THEN** it exits with code 0 and the output contains exactly one `game_start` line, one or more `move` lines, and exactly one `game_end` line

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

## ADDED Requirements

### Requirement: Play driver records per-player agent strategies, seeds, and advance weight in game-start log event

The `game_start` NDJSON line SHALL include `"agent_p0"` and `"agent_p1"` fields (strings) recording the strategy name for each player, `"seed_p0"` and `"seed_p1"` fields (integers) recording each player's seed, and an `"advance_weight"` field (number) recording the advance-weight value used. This makes every game fully reproducible from the log alone.

#### Scenario: Agent, seed, and advance-weight fields present in game-start

- **WHEN** the play driver runs with `--agent1 income --seed1 7 --agent2 cheap --seed2 99 --advance-weight 1.5`
- **THEN** the `game_start` line contains `"agent_p0": "income"`, `"agent_p1": "cheap"`, `"seed_p0": 7`, `"seed_p1": 99`, `"advance_weight": 1.5`

#### Scenario: Default values recorded when flags are omitted

- **WHEN** the play driver runs without `--agent1`, `--agent2`, `--seed1`, `--seed2`, or `--advance-weight`
- **THEN** the `game_start` line contains `"agent_p0": "random"`, `"agent_p1": "random"`, `"seed_p0": 42`, `"seed_p1": 42`, `"advance_weight": 1.0`
