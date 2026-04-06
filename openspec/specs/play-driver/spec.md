### Requirement: Play driver runs a complete game between two random agents and writes a log

The play driver executable SHALL accept `--seed <n>` and `--setup <id>` command-line arguments, run a full game between two random agents on the given setup starting with the given seed, and write the NDJSON game log to stdout. An optional `--output <file>` argument SHALL redirect the log to a file instead.

#### Scenario: Game terminates and log is written

- **WHEN** the play driver is invoked with valid `--seed` and `--setup` arguments
- **THEN** it exits with code 0 and the output contains exactly one `game_start` line, one or more `move` lines, and exactly one `game_end` line

#### Scenario: Same seed and setup produce identical log

- **WHEN** the play driver is invoked twice with the same `--seed` and `--setup` values
- **THEN** the two output logs are byte-for-byte identical

#### Scenario: Output redirected to file with --output

- **WHEN** `--output game.ndjson` is given
- **THEN** the log is written to `game.ndjson` and nothing is written to stdout

### Requirement: Play driver reports an error for invalid arguments

- **WHEN** `--seed` or `--setup` is missing or non-numeric
- **THEN** the play driver prints a usage message to stderr and exits with a non-zero code

#### Scenario: Missing seed argument

- **WHEN** the play driver is invoked without `--seed`
- **THEN** it exits with a non-zero code and prints an error to stderr
