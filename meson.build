project(
  'biner',
  'cpp',
  version : '0.1',
  default_options : ['cpp_std=c++2a']
)

project_source_files = [
  'src/biner.cpp',
]

project_include_directories = [
  include_directories('include'),
]

project_dependencies = [
]

build_args = [
  '-DVERSION=' + meson.project_version(),
]

project_target = executable(
  meson.project_name(),
  project_source_files, install : true,
  dependencies: project_dependencies,
  c_args : build_args,
  include_directories : project_include_directories,
)

test(meson.project_name(), project_target)
