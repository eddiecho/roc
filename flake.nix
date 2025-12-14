{
  inputs = {
    utils.url = "github:numtide/flake-utils";

    treefmt-nix.url = "github:numtide/treefmt-nix";
  };
  outputs = {
    self,
    nixpkgs,
    utils,
    treefmt-nix,
  }:
    utils.lib.eachDefaultSystem (
      system: let
        pkgs = import nixpkgs {
          inherit system;
        };
      in {
        formatter =
          (treefmt-nix.lib.evalModule pkgs ./treefmt.nix).config.build.wrapper;

        devShell = pkgs.mkShellNoCC {
          buildInputs = with pkgs; [
            clang-tools # this has to come before clang everytime...
            clang_21
            cmake
            gnumake
            ninja
          ];
        };
      }
    );
}
