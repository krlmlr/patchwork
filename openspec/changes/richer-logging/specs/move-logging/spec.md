## ADDED Requirements

### Requirement: move event includes per-player income and free_spaces
The `log_move` function SHALL emit `"income"` and `"free_spaces"` fields for the active player in every `move` NDJSON line, reflecting the post-move state.

#### Scenario: Buy-patch move emits income and free_spaces
- **WHEN** `log_move` is called after a `buy_patch` move
- **THEN** the emitted JSON line SHALL contain `"income"` and `"free_spaces"` fields for the active player, matching `state.player(player).income()` and `state.player(player).free_spaces()` respectively

#### Scenario: Advance move emits income and free_spaces
- **WHEN** `log_move` is called after an `advance` move
- **THEN** the emitted JSON line SHALL contain `"income"` and `"free_spaces"` fields for the active player

### Requirement: move event includes board_value proxy
The `log_move` function SHALL emit a `"board_value"` field for the active player, defined as `buttons - 5` (buttons above the starting amount), as a proxy for economic value accumulated.

#### Scenario: Starting position board_value
- **WHEN** a player has not yet purchased any patches and holds exactly 5 buttons
- **THEN** `"board_value"` SHALL be 0

#### Scenario: Post-purchase board_value
- **WHEN** a player holds more than 5 buttons after receiving income payouts
- **THEN** `"board_value"` SHALL equal `buttons - 5`

### Requirement: move event includes patch circle snapshot
The `log_move` function SHALL emit a `"circle"` field containing the currently available patches in circle order, starting from the current `circle_marker()` position, as a string of patch-name characters. Only patches where `patch_available(id)` is true SHALL be included.

#### Scenario: Circle shrinks after buy
- **WHEN** `log_move` is called after a `buy_patch` move
- **THEN** the `"circle"` field SHALL NOT contain the purchased patch's name character

#### Scenario: Circle is empty at game end
- **WHEN** all patches have been purchased and `log_move` is called for the final move
- **THEN** the `"circle"` field SHALL be an empty string `""`

#### Scenario: Circle wraps correctly
- **WHEN** the `circle_marker()` is non-zero
- **THEN** the `"circle"` field SHALL start from the patch at the marker position and wrap around to patches before the marker, omitting unavailable patches
