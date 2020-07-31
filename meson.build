project(
    'wlr-sunclock',
    'c',
    version: '0.0.0',
    meson_version: '>=0.47.0',
)

cc = meson.get_compiler('c')
math = cc.find_library('m')
gtk = dependency('gtk+-wayland-3.0', version: '>=3.22.0')
gtk_layer_shell = dependency('gtk-layer-shell-0')

executable(
    meson.project_name(),
    files(
    	'src/main.c',
    	'src/gui.c',
    	'src/astro.c',
    ),
    include_directories: ['include'],
    dependencies: [
        math, 
        gtk, 
        gtk_layer_shell,
    ],
    install: true,
    link_args: ['-X', '-lm'],
)
