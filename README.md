# wlr-sunclock

    pacman -S gtk-layer-shell gtk3 wayland

## install from source

    meson build
    sudo ninja -C build install

## install from aur

see [wlr-sunclock-git](https://aur.archlinux.org/packages/wlr-sunclock-git/)

## cli args

    -a, --anchors=ANCHORS      window anchors (see below)
    -l, --layer=<background|bottom|top|overlay>
                               desktop layer to show the widget on
    -m, --margins=MARGINS      window margins
    -w, --width=WIDTH          window width
    -c, --border-colour=BORDER_COLOUR
                               window border colour (unused)
    -d, --border-width=BORDER_WIDTH
                               window border width
    -?, --help                 Give this help list
        --usage                Give a short usage message

## anchors

    -a tl   -a tr   -a br   -a bl   -a ''   -a tblr
    ┌─┬───┐ ┌───┬─┐ ┌─────┐ ┌─────┐ ┌─────┐ ╔═════╗
    ├─┘   │ │   └─┤ │     │ │     │ │ ┌─┐ │ ║     ║
    │     │ │     │ │   ┌─┤ ├─┐   │ │ └─┘ │ ║     ║
    └─────┘ └─────┘ └───┴─┘ └─┴───┘ └─────┘ ╚═════╝
