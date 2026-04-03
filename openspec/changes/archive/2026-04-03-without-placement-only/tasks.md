## 1. Header Definition

- [x] 1.1 Create `src/simplified_game_state.hpp` with `SimplifiedPlayerState` packed into 32 bits (free_spaces 7 bits, position 6 bits, buttons 7 bits, income 5 bits)
- [x] 1.2 Add default constructor to `SimplifiedPlayerState` initialising free_spaces to 81, position to 0, buttons to 5, income to 0
- [x] 1.3 Add accessors and mutators for all four fields with range assertions
- [x] 1.4 Add `SimplifiedGameState` struct with two `SimplifiedPlayerState` members and the same shared-state fields as `GameState` (patch availability bitmask, circle marker, 7×7 bonus)
- [x] 1.5 Add default constructor to `SimplifiedGameState` (both players default, all patches available, marker 0, bonus unclaimed)

## 2. Compile-time Checks

- [x] 2.1 Add `static_assert(sizeof(SimplifiedPlayerState) <= 4)` in the header
- [x] 2.2 Add `static_assert(!std::is_same_v<SimplifiedPlayerState, PlayerState>)` in the header

## 3. Build Integration

- [x] 3.1 Register `src/simplified_game_state.hpp` as a Meson header source so it is included in the build
- [x] 3.2 Verify `meson setup build && meson compile -C build` succeeds with no new warnings

## 4. Unit Tests

- [x] 4.1 Create `tests/test_simplified_game_state.cpp` with a Catch2 test file
- [x] 4.2 Test `sizeof(SimplifiedPlayerState) <= 4` at runtime (mirrors the static_assert)
- [x] 4.3 Test default-constructed `SimplifiedPlayerState` has free_spaces=81, position=0, buttons=5, income=0
- [x] 4.4 Test round-trip for free_spaces over the full range 0–81
- [x] 4.5 Test round-trip for position over the full range 0–53
- [x] 4.6 Test round-trip for buttons over the full range 0–127
- [x] 4.7 Test round-trip for income over the full range 0–31
- [x] 4.8 Test default-constructed `SimplifiedGameState` has expected values (both players default, all patches available, marker 0)
- [x] 4.9 Test patch availability round-trip on `SimplifiedGameState` for all indices 0–32
- [x] 4.10 Test circle marker round-trip on `SimplifiedGameState` for all values 0–32
- [x] 4.11 Register the new test file in `meson.build` and verify `meson test -C build` passes
