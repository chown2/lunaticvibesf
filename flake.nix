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
          IMGUI_BUILD_SDL3_BINDING = true;
          IMGUI_BUILD_SDL3_RENDERER_BINDING = true;
        }
      );

      buildInputsShared = with pkgs; [
        # Something here is required for GPU acceleration.
        libGL.dev
        libdrm.dev
        libglvnd.dev
        # mesa.dev
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
      ];

      nativeBuildInputsShared = with pkgs; [
        ccache # not sccache since it doesn't support precompiled headers.
        cmake
        gersemi # CMake formatter
        mold-wrapped # Faster linker.
        ninja
      ];
      # To override fmod rpath.
      LD_LIBRARY_PATH = pkgs.lib.makeLibraryPath [
        pkgs.alsa-lib
        pkgs.libpulseaudio
      ];
    in
    {
      devShells.${system} = {
        default = pkgs.mkShell {
          inherit LD_LIBRARY_PATH system;
          buildInputs =
            buildInputsShared
            ++ [
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

              sdl3
              sdl3-image
              sdl3-ttf
              boost
              cereal_1_3_2
              curl
              ffmpeg
              gbenchmark
              gtest

              openssl
              plog
              re2
              sqlite
              yaml-cpp
            ]);
          nativeBuildInputs =
            nativeBuildInputsShared
            ++ (with pkgs; [
              pkg-config
            ]);
        };
        vcpkgDeps = pkgs.mkShell {
          inherit LD_LIBRARY_PATH system;
          buildInputs =
            # vcpkg will silently build SDL without graphics support if it couldn't
            # find a god know what dependency.
            # ..and that seems to be the case here...
            buildInputsShared ++ pkgs.sdl3.buildInputs;
          nativeBuildInputs =
            nativeBuildInputsShared
            ++ (with pkgs; [
              autoconf # libxcrypt building
              automake # aclocal, libxcrypt building
              jinja2-cli # libsystemd
              libtool # libxcrypt building
              nasm # ffmpeg building
              # libcurl fails with wrapped on 2025.02.14 - 2025.06.13 but not 2025.01.13
              # Mentioned here https://github.com/microsoft/vcpkg/issues/42332#issuecomment-2651993455
              pkg-config-unwrapped
              vcpkg
              wayland-scanner # sdl3
            ]);
          VCPKG_ROOT = "${pkgs.vcpkg}/share/vcpkg";
        };
      };
    };
}
