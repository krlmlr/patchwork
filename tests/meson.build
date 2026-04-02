catch2_dep = dependency('catch2-with-main', fallback : ['catch2', 'catch2_with_main_dep'])

test_main = executable(
  'test_main',
  'test_main.cpp',
  dependencies : [patchwork_dep, catch2_dep],
)
test('baseline', test_main)
