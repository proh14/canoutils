{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
    pre-commit-hooks = {
      url = "github:cachix/git-hooks.nix";
      inputs = {
        nixpkgs.follows = "nixpkgs";
        flake-utils.follows = "flake-utils";
      };
    };
  };

  outputs = {
    self,
    nixpkgs,
    flake-utils,
    pre-commit-hooks,
  }:
    flake-utils.lib.eachSystem [
      "aarch64-darwin"
      "aarch64-linux"
      "x86_64-darwin"
      "x86_64-linux"
    ] (system: let
      pkgs = nixpkgs.legacyPackages.${system};
    in rec {
      formatter = pkgs.alejandra;

      checks = let
        pre-commit-check = pre-commit-hooks.lib.${system}.run {
          src = ./.;
          hooks = {
            alejandra.enable = true;
            clang-format.enable = true;
          };
        };
      in
        if (builtins.elem system flake-utils.lib.defaultSystems)
        then {inherit pre-commit-check;}
        else {};

      devShells.default = pkgs.mkShell {
        inherit (checks.pre-commit-check) shellHook;

        hardeningDisable = ["fortify"];
        inputsFrom = pkgs.lib.attrsets.attrValues packages;
        packages = with pkgs; [
          bear
          python3Packages.compiledb
          gcovr
          clang-tools
        ];
      };

      packages = let
        build-base = {
          src = ./.;
          nativeBuildInputs = with pkgs; [
            gcc13
            gnumake
            # ↓ tput provider for colored makefile
            ncurses
          ];
        };

        build-single-bin = name:
          pkgs.stdenvNoCC.mkDerivation (build-base
            // rec {
              inherit name;

              buildPhase = ''
                make -C src/${name}
              '';

              installPhase = ''
                mkdir -p $out/bin

                install -D src/${name}/${name} $out/bin/${name} --mode 0755
              '';
            });
      in
        rec {
          canoutils = default;
          default = pkgs.stdenvNoCC.mkDerivation (build-base
            // {
              name = "canoutils";

              installPhase = ''
                mkdir -p $out/bin

                find bin -type f | xargs -i install -D {} $out/{} --mode 0755
              '';
            });
        }
        // (with builtins;
          listToAttrs (map
            (name: {
              inherit name;
              value = build-single-bin name;
            })
            (attrNames (pkgs.lib.filterAttrs
              (n: v: v == "directory")
              (readDir ./src)))));
    });
}
