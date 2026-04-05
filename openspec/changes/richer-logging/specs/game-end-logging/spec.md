## ADDED Requirements

### Requirement: game_end event includes final per-player income and free_spaces
The `log_game_end` function SHALL emit `"income"` and `"free_spaces"` fields for both players in the `game_end` NDJSON line, reflecting the final game state.

#### Scenario: game_end contains income and free_spaces for both players
- **WHEN** `log_game_end` is called at the end of a game
- **THEN** the emitted JSON line SHALL contain `"p0"` and `"p1"` objects, each with `"income"` and `"free_spaces"` fields matching `state.player(0).income()`, `state.player(0).free_spaces()`, `state.player(1).income()`, and `state.player(1).free_spaces()` respectively

#### Scenario: game_end retains existing score and winner fields
- **WHEN** `log_game_end` is called
- **THEN** the emitted JSON line SHALL still contain `"score_p0"`, `"score_p1"`, and `"winner"` fields as before
