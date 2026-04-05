## 1. Orientation Engine

- [ ] 1.1 Create `src/moves.hpp` in namespace `patchwork`, include `src/generated/patches.hpp`
  and `src/game_setup.hpp`; add the forward declarations for `orient_cells`,
  `unique_orientations`, `fits_at`, `visible_patches`, `legal_moves`, `apply`, and
  `current_player`
- [ ] 1.2 Add `constexpr std::array<int, 9> kIncomeSpaces = {5, 11, 17, 23, 29, 35, 41, 47, 53}`
- [ ] 1.3 Implement `orient_cells(const PatchData& patch, int orientation_idx) → std::vector<CellOffset>`:
  apply horizontal flip when `orientation_idx & 1`, then rotate 90° clockwise `(orientation_idx >> 1)` times
  (transform `(r, c) → (c, -r)` per rotation); normalise so min row = 0 and min col = 0; return result
- [ ] 1.4 Implement `unique_orientations(const PatchData& patch) → std::vector<int>`:
  iterate orientation indices 0–7, collect the normalised sorted cell list for each, deduplicate by
  inserting into a `std::set`; return the indices of distinct orientations

## 2. Placement Validator

- [ ] 2.1 Implement `fits_at(const PlayerState& board, const std::vector<CellOffset>& cells, int row, int col) → bool`:
  for each cell `(dr, dc)`, check `(row+dr, col+dc)` is in `[0,9)×[0,9)` and `board.cell(row+dr, col+dc)` is false;
  return true only if all cells pass

## 3. Move Types

- [ ] 3.1 Add `struct PlacePatch` with fields: `uint8_t circle_offset` (0–2),
  `uint8_t orientation` (0–7), `uint8_t row`, `uint8_t col`
- [ ] 3.2 Add `struct AdvanceAndReceive` (empty struct, used as tag)
- [ ] 3.3 Add `using Move = std::variant<PlacePatch, AdvanceAndReceive>` and include `<variant>`

## 4. Visible Patches and Move Generator

- [ ] 4.1 Implement `visible_patches(const GameSetup& setup, const GameState& state) → std::array<int, 3>`:
  return `setup.circle[(m+1)%33]`, `setup.circle[(m+2)%33]`, `setup.circle[(m+3)%33]`
  where `m = state.circle_marker()`
- [ ] 4.2 Implement `current_player(const GameState& state) → int`:
  return 0 if `state.player(0).position() <= state.player(1).position()`, else 1
- [ ] 4.3 Implement `legal_moves(const GameSetup& setup, const GameState& state, int player_idx) → std::vector<Move>`:
  - Add `AdvanceAndReceive{}` if the player's position < 53
  - For each `circle_offset` in {0, 1, 2}: look up the patch via `visible_patches`, skip if
    `player.buttons() < patch.buttons`; otherwise iterate `unique_orientations`; for each
    orientation iterate all 81 anchors (row 0–8, col 0–8), call `fits_at`, and if true push a
    `PlacePatch{circle_offset, orientation, row, col}`

## 5. Move Application

- [ ] 5.1 Add a helper `income_spaces_crossed(int from, int to) → int` that counts elements of
  `kIncomeSpaces` strictly greater than `from` and less than or equal to `to`
- [ ] 5.2 Implement `apply(const GameSetup& setup, GameState& state, int player_idx, const Move& move)`:
  - For `PlacePatch`: retrieve the patch (via `visible_patches` + `kPatches`); deduct button cost;
    advance time (cap at 53); award `player.income() × income_spaces_crossed(old_pos, new_pos)` buttons;
    add patch income to player income; stamp board cells at the given orientation+anchor; mark patch
    unavailable; advance `circle_marker` by `circle_offset + 1` modulo 33
  - For `AdvanceAndReceive`: compute target = `min(other_player.position() + 1, 53)`;
    award `(target - player.position())` buttons; award income for crossed income spaces;
    set new position
- [ ] 5.3 Add `src/moves.hpp` to the umbrella header `src/patchwork.hpp`

## 6. Unit Tests

- [ ] 6.1 Create `tests/test_moves.cpp`; add a Catch2 section for the orientation engine:
  - Identity orientation (index 0) returns the normalised original cells
  - Applying orientation index 2 (90° rotation) to a known patch matches the expected result
  - All 8 indices applied to an asymmetric patch yield 8 distinct cell lists
  - `unique_orientations` on a square (2×2) patch returns fewer than 8 entries
- [ ] 6.2 Add a Catch2 section for `fits_at`:
  - Returns `true` on an empty board with a valid anchor
  - Returns `false` when a cell overlaps an occupied board cell
  - Returns `false` when an anchor places a cell out of bounds
- [ ] 6.3 Add a Catch2 section for `visible_patches`:
  - Returns correct IDs for `circle_marker` = 0
  - Returns wrapping IDs for `circle_marker` = 31
- [ ] 6.4 Add a Catch2 section for `legal_moves`:
  - Contains exactly one `AdvanceAndReceive` when player position < 53
  - Contains no `PlacePatch` when player buttons = 0
  - Contains `PlacePatch` moves when the player can afford at least one visible patch and the
    board is not full
- [ ] 6.5 Add a Catch2 section for `apply` with `PlacePatch`:
  - Buttons decrease by patch button cost
  - Position increases by patch time cost
  - Income increases by patch income
  - Button income awarded when crossing a known income space
  - Board cells are stamped correctly
  - Patch marked unavailable after apply
  - Circle marker advances by `circle_offset + 1`
- [ ] 6.6 Add a Catch2 section for `apply` with `AdvanceAndReceive`:
  - Player position moves to `other_pos + 1`
  - Buttons increase by spaces moved
  - Button income awarded when crossing income spaces
- [ ] 6.7 Add a Catch2 test verifying `kIncomeSpaces` has 9 entries equal to
  `{5, 11, 17, 23, 29, 35, 41, 47, 53}`
- [ ] 6.8 Register `tests/test_moves.cpp` in `tests/meson.build`
- [ ] 6.9 Run `meson test -C build` and verify all tests pass

## 7. Spec

- [ ] 7.1 Create `openspec/specs/piece-placement/spec.md` from the delta spec produced in this
  change (already present at `openspec/changes/piece-placement/specs/piece-placement/spec.md`)
