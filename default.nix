with import <nixpkgs> {};

stdenv.mkDerivation {

  pname = "sss";
  version = "v1.0.1-alpha";

  src = ./.;

  nativeBuildInputs = [
    pkg-config
    tinycc
    gcc
  ];

  buildInputs = [
    xorg.libxcb
    xorg.xcbutilcursor
    xorg.xcbutilimage
    libpng
  ];

  preInstall = ''
    sed -i "s|/usr/local|$out|" Makefile
    mkdir -p $out/bin
  '';

  makeFlags = [ "CC=gcc" "PREFIX=$(out)" ];

  system = builtins.currentSystem;

  meta = with lib; {
    homepage = "https://github.com/leath-dub/sscreenshot";
    description = "A simple screenshot program";
    license = licenses.gpl3;
    platforms = platforms.all;
    maintainers = [ maintainers.leath-dub ];
  };

}
