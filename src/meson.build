patchwork_inc = include_directories('.')

patchwork_lib = static_library(
  'patchwork',
  'move_generation.cpp',
  'move_application.cpp',
  'game_logger.cpp',
  'random_agent.cpp',
)

patchwork_dep = declare_dependency(
  include_directories : patchwork_inc,
  link_with : patchwork_lib,
)

executable(
  'play_driver',
  'play_driver.cpp',
  dependencies : [patchwork_dep],
)
