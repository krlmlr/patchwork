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
