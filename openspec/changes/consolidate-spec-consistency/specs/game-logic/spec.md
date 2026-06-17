## MODIFIED Requirements

### Requirement: 7×7 bonus is claimed when a player reaches 56 occupied cells

Under the **simplified rules** implemented in this phase, tile placement is not modelled, so the 7×7 special-tile bonus is approximated by counting occupied cells. The bonus tile is claimed by the first player whose occupied cells (81 − free_spaces) reach or exceed **56**. After each `BuyPatch` move, if the bonus is unclaimed and the buyer's occupied cells ≥ 56, the bonus status SHALL be set to that player.

The **final game** (once piece placement lands) awards this bonus to the first player to completely fill a contiguous **7×7 area (49 cells)** of their quilt board. That placement-based 49-cell rule is intentionally deferred; the 56-occupied-cell count is a deliberate simplified-rules proxy, **not** the final-game rule. The code, this spec, and `docs/glossary.md` SHALL keep the two thresholds distinct and clearly labelled (56 = simplified, 49 = final).

#### Scenario: Bonus claimed on reaching 56 occupied cells

- **WHEN** a player purchases a patch that brings their occupied cells from 55 to 60 and the bonus is unclaimed
- **THEN** the bonus status is set to that player

#### Scenario: Bonus not re-awarded

- **WHEN** a player already holds the bonus and a second player reaches 56+ occupied cells
- **THEN** the bonus status remains with the first player

#### Scenario: Advance move does not affect bonus

- **WHEN** an Advance move is applied regardless of occupied cells
- **THEN** the bonus status is unchanged

#### Scenario: Final-game 49-cell area rule is deferred

- **WHEN** the simplified rules are in effect (piece placement is not modelled)
- **THEN** the 56-occupied-cell proxy is the active rule
- **AND** the final-game 49-cell 7×7-area rule is not applied until piece placement lands
