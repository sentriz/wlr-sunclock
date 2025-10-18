project(
  'wlr-sunclock',
  'c',
  meson_version : '>=0.46.0',
  default_options : ['warning_level=3', 'werror=true'],
version : 'v1.1.0'
)

cc = meson.get_compiler('c')
gtk = dependency('gtk4', version: '>=4.0.0')
gtk_layer_shell = dependency('gtk4-layer-shell-0')
librsvg = dependency('librsvg-2.0')

add_project_arguments(cc.get_supported_arguments([
    '-Wno-deprecated-declarations',
    '-Wno-overlength-strings',
]), language: 'c')

executable(
    meson.project_name(),
    files(
        'src/main.c',
        'src/astro.c',
        'src/map.c',
    ),
    dependencies: [
        cc.find_library('m'),
        gtk,
        gtk_layer_shell,
        librsvg,
    ],
    install: true,
    link_args: ['-lm'],
)
