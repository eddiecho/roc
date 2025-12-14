{pkgs, ...}: {
  projectRootFile = "flake.nix";
  programs.alejandra.enable = true;
  programs.jsonfmt.enable = true;
  programs.shfmt.enable = true;
  programs.cmake-format.enable = true;
  programs.clang-format.enable = true;
}
