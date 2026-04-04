## 1. Extend SimplifiedGameState with next_player and first_to_finish fields

- [ ] 1.1 Add `next_player` bit flag to `SimplifiedGameState::shared_` (bit 41) and `first_to_finish` bit flag (bit 42); update layout comment and constants
- [ ] 1.2 Expose `set_next_player(int)` setter and `active_player()` getter; expose `first_to_finish()` getter
- [ ] 1.3 Verify default construction leaves `next_player` = 0 and `first_to_finish` = 0
- [ ] 1.4 Add Catch2 tests: round-trip for both bits, active-on-tie scenario, switch-on-overtake scenario, first_to_finish set-once semantics
- [ ] 1.5 Remove cap on `SimplifiedPlayerState::set_position` — allow values 0–63 (full 6-bit range); update the assert from `v <= 53` to `v <= 63`
- [ ] 1.6 Add Catch2 test: position values 53–63 round-trip correctly

## 2. Move type

- [ ] 2.1 Create `src/move.hpp` defining `BuyPatch` and `Advance` variants and a `Move` type (std::variant or discriminated struct)
- [ ] 2.2 Add equality operators for `Move` so tests can compare moves
- [ ] 2.3 Add Catch2 tests: BuyPatch carries index, Advance != BuyPatch

## 3. Legal move generation

- [ ] 3.1 Create `src/move_generation.hpp` / `.cpp` with `legal_moves(const SimplifiedGameState&) → std::vector<Move>`
- [ ] 3.2 Implement circular scan from circle marker to find up to three available patches
- [ ] 3.3 Filter patches by affordability (buttons only; no position cap — positions > 53 are valid)
- [ ] 3.4 Always include `Advance` for non-terminal states; return empty for terminal states (both players ≥ 53)
- [ ] 3.5 Add Catch2 tests: three available patches, fewer than three, unaffordable patches, terminal state, advance always present

## 4. Move application

- [ ] 4.1 Create `src/move_application.hpp` / `.cpp` with `apply_move(SimplifiedGameState, Move) → SimplifiedGameState`
- [ ] 4.2 Implement `BuyPatch` path: deduct buttons, advance position (no cap), add income, reduce free_spaces, mark patch unavailable, advance circle marker; update `next_player`; if this is the first move crossing ≥ 53, set `first_to_finish`
- [ ] 4.3 Implement `Advance` path: advance to opponent position + 1 (no cap), credit 1 button per space advanced; update `next_player`; if this is the first move crossing ≥ 53, set `first_to_finish`
- [ ] 4.4 Implement button income space payout (positions 5, 11, 17, 23, 29, 35, 41, 47, 53): for each threshold crossed by the move, add `income` to buttons
- [ ] 4.5 Implement leather patch award: for each of the five thresholds (26, 32, 38, 44, 50) crossed by the moving player's position, check whether either player's pre-move position was already ≥ threshold; if not, decrement `free_spaces` by 1 (mandatory placement); no new state flags required
- [ ] 4.6 Implement 7×7 bonus check after `BuyPatch`: if bonus unclaimed and occupied cells ≥ 56, set bonus to active player
- [ ] 4.7 Add Catch2 tests for each requirement scenario in `specs/move-application/spec.md`, including inactive player unchanged, multiple income spaces crossed, leather patch not re-awarded

## 5. Terminal detection and scoring

- [ ] 5.1 Create `src/terminal_and_scoring.hpp` with `is_terminal`, `score`, and `winner` free functions
- [ ] 5.2 Implement `is_terminal`: both players at position ≥ 53
- [ ] 5.3 Implement `score(state, player)`: buttons − 2 × free_spaces + 7 if bonus held
- [ ] 5.4 Implement `winner(state)`: compare scores; on equal scores use `first_to_finish` as tiebreaker (never returns -1)
- [ ] 5.5 Add Catch2 tests covering all scenarios in `specs/terminal-and-scoring/spec.md`

## 6. NDJSON game logger

- [ ] 6.1 Create `src/game_logger.hpp` / `.cpp` with a `GameLogger` class (or free functions) writing to an `std::ostream`
- [ ] 6.2 Implement `log_game_start(ostream, seed, setup_id, initial_state)`
- [ ] 6.3 Implement `log_move(ostream, ply, player, move, new_state)`
- [ ] 6.4 Implement `log_game_end(ostream, state)` using `score` and `winner`
- [ ] 6.5 Ensure each line is a self-contained JSON object with a trailing newline and no surrounding array
- [ ] 6.6 Add Catch2 tests: parse each line as JSON (using a minimal hand-rolled check or regex), verify field presence, verify ply sequence

## 7. Random agent

- [ ] 7.1 Create `src/random_agent.hpp` / `.cpp` with `random_move(const SimplifiedGameState&, std::mt19937&) → Move`
- [ ] 7.2 Use `std::uniform_int_distribution` over `legal_moves` indices
- [ ] 7.3 Add Catch2 tests: returned move is always legal; same seed produces same move; distribution check over many samples

## 8. Play driver executable

- [ ] 8.1 Create `src/play_driver.cpp` with `main`, parsing `--seed`, `--setup`, and optional `--output` arguments
- [ ] 8.2 Load `GameSetup` from the specified setup id (via existing setup loading code)
- [ ] 8.3 Seed `std::mt19937` with the given seed; initialise a `SimplifiedGameState` from the setup
- [ ] 8.4 Run game loop: while not terminal, call `random_move` for active player, call `apply_move`, log move
- [ ] 8.5 Log game-start before loop and game-end after loop
- [ ] 8.6 Write output to stdout by default; redirect to file if `--output` is given
- [ ] 8.7 Print usage to stderr and exit non-zero on missing or invalid arguments
- [ ] 8.8 Register play driver as a separate Meson executable target in `src/meson.build`
- [ ] 8.9 Manual smoke test: run the driver with a fixed seed and verify the log is valid NDJSON and two identical runs produce identical output

## 9. Build and test integration

- [ ] 9.1 Add new source files to `src/meson.build` (library sources) and `tests/meson.build` (test sources)
- [ ] 9.2 Run `meson setup build && meson test -C build` and confirm all tests pass
- [ ] 9.3 Update `openspec/roadmap.md` to mark "Simplified Rules" as done
