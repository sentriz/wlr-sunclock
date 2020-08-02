project(
    'wlr-sunclock',
    'c',
    version: '0.0.0',
    meson_version: '>=0.47.0',
    default_options: [
        'c_std=c99',
        'warning_level=3',
        'werror=true',
    ],
)

install_data(
    files(
        'assets/maps/1200.jpg',
        'assets/maps/1600.jpg',
        'assets/maps/200.jpg',
        'assets/maps/400.jpg',
        'assets/maps/800.jpg',
    ),
    install_dir: join_paths(get_option('datadir'), 'wlr-sunclock', 'maps'),
)

cc = meson.get_compiler('c')
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
        cc.find_library('m'),
        gtk,
        gtk_layer_shell,
    ],
    install: true,
    link_args: ['-X', '-lm'],
)
