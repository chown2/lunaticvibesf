# Lunatic Vibes

Lunatic Vibes is a rhythm game that plays community-made charts in BMS format.

The project is basically a rewritten version of the Lunatic Rave 2 (beta3 100201). 

The project is still in development stage. Please do not expect a bug-free experience. Feel free to open issues.


## Features

* Multi-threaded update procedure
* Async input handling
* Full Unicode support
* Built-in difficulty table support
* Mixed skin resolution support (SD, HD, FHD)
* ARENA Mode over LAN / VLAN

For LR2 feature compatibility list, check out [the wiki](https://github.com/yaasdf/lunaticvibes/wiki/LR2-Features-Compatibility).


## Requirements
  * **Do NOT use this application to load unauthorized copyrighted contents (e.g. charts, skins).**
  * Supported OS: Windows 7+


## Quick Start

* Install [Microsoft Visual C++ Redistributable 2015+ x64](https://aka.ms/vs/17/release/vc_redist.x64.exe)
* Download the latest release from [here](https://github.com/yaasdf/lunaticvibes/releases)
* Copy LR2files folder from LR2 (must include default theme; a fresh copy right from LR2 release is recommended)


## Build

### Windows with Visual Studio

Open the project's directory in Visual Studio, it should pick up CMake and install dependencies with vcpkg
automatically.

### Linux

```sh
# export VCPKG_ROOT=/path/to/vcpkg
cmake --preset linux-vcpkg -B ./build
cmake --build ./build --config=Debug -j
ls build/bin/Debug
```

## License
* MIT License
