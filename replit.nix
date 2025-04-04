{pkgs}: {
  deps = [
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
