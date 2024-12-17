{
  description = "Lunatic Vibes F flake";

  inputs = {
    nixpkgs-unstable.url = "github:nixos/nixpkgs?ref=nixos-unstable";
  };

  outputs =
    { self, nixpkgs-unstable }:
    let
      system = "x86_64-linux";
      pkgs = nixpkgs-unstable.legacyPackages.${system};
      taocpp-json = pkgs.stdenv.mkDerivation (finalAttrs: {
        pname = "taocpp-json";
        version = "1.0.0-beta.14";
        src = pkgs.fetchFromGitHub {
          owner = "taocpp";
          repo = "json";
          rev = "${finalAttrs.version}";
          hash = "sha256-/2/K7ykt/zCmap8KWVJnoiJcTHCVthj5ZEo0BYEDEo0=";
        };
        nativeBuildInputs = [ pkgs.cmake ];
        buildInputs = [ pkgs.pegtl ];
        cmakeFlags = [
          "-DTAOCPP_JSON_BUILD_EXAMPLES=OFF"
          "-DTAOCPP_JSON_BUILD_PERFORMANCE=OFF"
          "-DTAOCPP_JSON_BUILD_TESTS=OFF"
        ];
      });
      imgui = (
        pkgs.imgui.override {
          IMGUI_BUILD_SDL2_BINDING = true;
          IMGUI_BUILD_SDL2_RENDERER_BINDING = true;
        }
      );

      buildInputs =
        [
          taocpp-json
          imgui
        ]
        ++ (with pkgs; [
          pegtl

          libtiff
          lerc

          harfbuzz
          freetype
          glib
          pcre2

          SDL2
          SDL2_gfx
          SDL2_image
          SDL2_ttf
          boost
          cereal_1_3_2
          curl
          ffmpeg
          gtest

          openssl
          plog
          re2
          sqlite
          yaml-cpp

          luajit
          sol2

          # Something here is required for GPU acceleration.
          libdrm.dev
          libglvnd.dev
          mesa.dev
          mesa_glu.dev
          xorg.libX11.dev
          xorg.libXext.dev
          xorg.libXi.dev
          xorg.libpthreadstubs
          xorg.libxcb.dev
          xorg.xcbproto
          xorg.xcbutil.dev
          xorg.xcbutilcursor.dev
          xorg.xcbutilerrors
          xorg.xcbutilkeysyms.dev
          xorg.xcbutilrenderutil.dev
          xorg.xcbutilwm.dev
          xorg.xorgproto
        ]);
      nativeBuildInputs = with pkgs; [
        ccache # not sccache since it doesn't support precompiled headers.
        cmake
        mold-wrapped # Faster linker.
        ninja
        pkg-config
      ];
    in
    {
      devShells.${system}.default = pkgs.mkShell {
        inherit buildInputs nativeBuildInputs system;
        # To override fmod rpath.
        LD_LIBRARY_PATH = pkgs.lib.makeLibraryPath [
          pkgs.alsa-lib
          pkgs.libpulseaudio
        ];
      };
    };
}
