with import <nixpkgs> {};

stdenv.mkDerivation {

  pname = "sss";
  version = "v1.0.1-alpha";

  src = ./.;

  propagatedBuildInputs = with pkgs; [
    xorg.libxcb
    xorg.xcbutilcursor
    xorg.xcbutilimage
    libpng
  ];

  preInstall = ''
    export CC=gcc
    sed -i "s|/usr/local|$out|" Makefile
    mkdir -p $out/bin
  '';

  system = builtins.currentSystem;

  meta = with lib; {
    homepage = "https://github.com/leath-dub/sscreenshot";
    description = "A simple screenshot program";
    license = licenses.gpl3;
    platforms = platforms.all;
    maintainers = [ maintainers.leath-dub ];
  };

}
