{pkgs}: {
  deps = [
    pkgs.pcre2
    pkgs.gtk3
    pkgs.webkitgtk
    pkgs.curl
    pkgs.fontconfig
    pkgs.xorg.libXcomposite
    pkgs.xorg.libXcursor
    pkgs.xorg.libXinerama
    pkgs.mesa
    pkgs.xorg.libXrandr
    pkgs.xorg.libXext
    pkgs.xorg.libX11
    pkgs.freetype
    pkgs.alsa-lib
    pkgs.pkg-config
    pkgs.unzip
    pkgs.wget
    pkgs.cmake
  ];
}
