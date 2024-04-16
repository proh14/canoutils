{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = {
    self,
    nixpkgs,
    flake-utils,
  }:
    flake-utils.lib.eachSystem ["x86_64-linux"]
    (system: let
      pkgs = nixpkgs.legacyPackages.${system};
    in rec {
      formatter = pkgs.alejandra;

      devShells.default = pkgs.mkShell {
        inputsFrom = pkgs.lib.attrsets.attrValues packages;
        packages = with pkgs; [
          bear
          python3Packages.compiledb
          gcovr
        ];
      };

      packages = rec {
        canoutils = default;
        default = pkgs.stdenvNoCC.mkDerivation {
          name = "canoutils";

          src = ./.;
          nativeBuildInputs = with pkgs; [
            gcc13
            gnumake
            # ↓ tput provider for colored makefile
            ncurses
          ];

          installPhase = ''
            mkdir -p $out/bin

            find bin -type f | xargs -i install -D {} $out/{} --mode 0755
          '';
        };
      };
    });
}
