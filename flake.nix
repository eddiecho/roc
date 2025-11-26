{
  inputs = {
    utils.url = "github:numtide/flake-utils";
  };
  outputs = { self, nixpkgs, utils }: utils.lib.eachDefaultSystem (system:
    let
      pkgs = import nixpkgs {
        inherit system;
      };
    in
    {
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
