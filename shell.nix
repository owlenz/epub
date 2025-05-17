{pkgs ? import <nixpkgs> {}}:

pkgs.mkShell {
  packages = with pkgs; [gtk4 libadwaita pkg-config libzip];
}
