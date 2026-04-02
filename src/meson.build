patchwork_inc = include_directories('.')

patchwork_lib = static_library(
  'patchwork',
  files(),
  include_directories : patchwork_inc,
)

patchwork_dep = declare_dependency(
  include_directories : patchwork_inc,
  link_with : patchwork_lib,
)
