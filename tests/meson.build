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
