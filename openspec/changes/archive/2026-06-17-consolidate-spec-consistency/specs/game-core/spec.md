## ADDED Requirements

### Requirement: `make_setup` sources circles from canonical `kGameSetups`

`make_setup(id)` SHALL return a `GameSetup` whose circle is `kGameSetups[id mod kNumGameSetups]` from `cpp/generated/game_setups.hpp`. It SHALL NOT construct the circle by any independent method (for example, rotating `kPatches`), so that the play driver and the TUI produce identical circles for the same `setup_id`. `kGameSetups` is the single source of truth for setup circles.

#### Scenario: Same setup id yields identical circle in driver and TUI

- **WHEN** `make_setup(id)` is called and `kGameSetups[id mod kNumGameSetups]` is read directly
- **THEN** both produce the same 33-character circle string
- **AND** the last character is `'2'` (the two-square tile), consistent with every canonical setup

#### Scenario: Setup id wraps around the canonical table

- **WHEN** `make_setup(id)` is called with `id` greater than or equal to `kNumGameSetups`
- **THEN** the returned circle equals `kGameSetups[id mod kNumGameSetups]`
