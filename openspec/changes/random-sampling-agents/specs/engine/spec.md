## MODIFIED Requirements

### Requirement: Play driver runs a complete game between two agents and writes a log

The play driver executable SHALL accept `--seed <n>`, `--setup <id>`, and an optional `--agent <strategy>` command-line argument. When `--agent` is given, both players use the named strategy; when omitted, both players use `random` (uniform). Valid strategy names are `random`, `cheap`, `income`, `income-per-time`. An optional `--output <file>` argument SHALL redirect the log to a file instead of stdout.

#### Scenario: Game terminates and log is written

- **WHEN** the play driver is invoked with valid `--seed` and `--setup` arguments
- **THEN** it exits with code 0 and the output contains exactly one `game_start` line, one or more `move` lines, and exactly one `game_end` line

#### Scenario: Same seed and setup produce identical log

- **WHEN** the play driver is invoked twice with the same `--seed`, `--setup`, and `--agent` values
- **THEN** the two output logs are byte-for-byte identical

#### Scenario: Output redirected to file with --output

- **WHEN** `--output game.ndjson` is given
- **THEN** the log is written to `game.ndjson` and nothing is written to stdout

#### Scenario: --agent selects biased strategy for both players

- **WHEN** the play driver is invoked with `--agent cheap`
- **THEN** both players use the cheap-biased random agent

#### Scenario: Invalid agent name is rejected

- **WHEN** the play driver is invoked with `--agent unknown`
- **THEN** it exits with a non-zero code and prints an error to stderr

## ADDED Requirements

### Requirement: Play driver records agent strategy in game-start log event

The `game_start` NDJSON line SHALL include an `agent` field (string) recording the strategy name used for both players, or `agent_p0` and `agent_p1` fields if the strategies differ in a future extension.

#### Scenario: Agent field present in game-start

- **WHEN** the play driver runs with `--agent income`
- **THEN** the `game_start` line contains `"agent": "income"`

#### Scenario: Default agent field is random

- **WHEN** the play driver runs without `--agent`
- **THEN** the `game_start` line contains `"agent": "random"`
