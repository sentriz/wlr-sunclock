project(
  'wlr-sunclock',
  'c',
meson_version : '>=0.47.0',
default_options : ['warning_level=3', 'werror=true'],
)

cc = meson.get_compiler('c')
gtk = dependency('gtk+-wayland-3.0', version: '>=3.22.0')
gtk_layer_shell = dependency('gtk-layer-shell-0')

add_project_arguments(cc.get_supported_arguments([
    '-Wno-deprecated-declarations',
    '-Wno-overlength-strings',
]), language: 'c')

executable(
    meson.project_name(),
    files(
        'src/main.c',
        'src/gui.c',
        'src/astro.c',
    ),
    include_directories: ['include'],
    dependencies: [
        cc.find_library('m'),
        gtk,
        gtk_layer_shell,
    ],
    install: true,
    link_args: ['-X', '-lm'],
)
