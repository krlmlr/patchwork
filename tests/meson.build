catch2_dep = dependency('catch2-with-main', fallback : ['catch2', 'catch2_with_main_dep'])

test_main = executable(
  'test_main',
  'test_main.cpp',
  dependencies : [patchwork_dep, catch2_dep],
)
test('baseline', test_main)

test_player_state = executable(
  'test_player_state',
  'test_player_state.cpp',
  dependencies : [patchwork_dep, catch2_dep],
)
test('player_state', test_player_state)

test_game_state = executable(
  'test_game_state',
  'test_game_state.cpp',
  dependencies : [patchwork_dep, catch2_dep],
)
test('game_state', test_game_state)

test_simplified_game_state = executable(
  'test_simplified_game_state',
  'test_simplified_game_state.cpp',
  dependencies : [patchwork_dep, catch2_dep],
)
test('simplified_game_state', test_simplified_game_state)

test_game_setup = executable(
  'test_game_setup',
  'test_game_setup.cpp',
  dependencies : [patchwork_dep, catch2_dep],
)
test('game_setup', test_game_setup)

test_move = executable(
  'test_move',
  'test_move.cpp',
  dependencies : [patchwork_dep, catch2_dep],
)
test('move', test_move)

test_move_generation = executable(
  'test_move_generation',
  'test_move_generation.cpp',
  dependencies : [patchwork_dep, catch2_dep],
)
test('move_generation', test_move_generation)

test_move_application = executable(
  'test_move_application',
  'test_move_application.cpp',
  dependencies : [patchwork_dep, catch2_dep],
)
test('move_application', test_move_application)

test_terminal_and_scoring = executable(
  'test_terminal_and_scoring',
  'test_terminal_and_scoring.cpp',
  dependencies : [patchwork_dep, catch2_dep],
)
test('terminal_and_scoring', test_terminal_and_scoring)

test_game_logger = executable(
  'test_game_logger',
  'test_game_logger.cpp',
  dependencies : [patchwork_dep, catch2_dep],
)
test('game_logger', test_game_logger)

test_random_agent = executable(
  'test_random_agent',
  'test_random_agent.cpp',
  dependencies : [patchwork_dep, catch2_dep],
)
test('random_agent', test_random_agent)

test_seed_util = executable(
  'test_seed_util',
  'test_seed_util.cpp',
  dependencies : [patchwork_dep, catch2_dep],
)
test('seed_util', test_seed_util)

test_setup_range = executable(
  'test_setup_range',
  'test_setup_range.cpp',
  dependencies : [patchwork_dep, catch2_dep],
)
test('setup_range', test_setup_range)

test_game_loop = executable(
  'test_game_loop',
  'test_game_loop.cpp',
  dependencies : [patchwork_dep, catch2_dep],
)
test('game_loop', test_game_loop)

# End-to-end batch_driver checks driven through the built executables. Written
# in R to match the project's tooling (R is already a CI dependency).
rscript_prog = find_program('Rscript', required : false)
if rscript_prog.found()
  test(
    'batch_cli',
    rscript_prog,
    args : [files('batch_cli_test.R'), batch_driver_exe, play_driver_exe],
  )
endif

test_tui_history = executable(
  'test_tui_history',
  'tui_history_test.cpp',
  '../cpp/tui/history.cpp',
  dependencies : [patchwork_dep, catch2_dep],
)
test('tui_history', test_tui_history)

test_tui_display = executable(
  'test_tui_display',
  'tui_display_test.cpp',
  '../cpp/tui/display.cpp',
  dependencies : [patchwork_dep, catch2_dep],
)
test('tui_display', test_tui_display)

test_tui_snapshot = executable(
  'test_tui_snapshot',
  'tui_snapshot_test.cpp',
  '../cpp/tui/display.cpp',
  '../cpp/tui/history.cpp',
  dependencies : [patchwork_dep, catch2_dep],
)
# Pass the absolute path to the snapshot directory so the test finds the
# golden files regardless of the working directory meson uses.
test('tui_snapshot', test_tui_snapshot,
     env : ['SNAPSHOT_DIR=' + meson.project_source_root() / 'tests' / 'snapshots'])
