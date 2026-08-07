# ElvUI-Updater

[![CI](https://github.com/ArjanDeo/ElvUI-Updater/actions/workflows/build.yml/badge.svg)](https://github.com/ArjanDeo/ElvUI-Updater/actions/workflows/build.yml)
[![Release](https://img.shields.io/github/v/release/ArjanDeo/ElvUI-Updater)](https://github.com/ArjanDeo/ElvUI-Updater/releases)
[![License](https://img.shields.io/github/license/ArjanDeo/ElvUI-Updater)](LICENSE.MD)
[![Stars](https://img.shields.io/github/stars/ArjanDeo/ElvUI-Updater)](https://github.com/ArjanDeo/ElvUI-Updater)

ElvUI-Updater is a small cross-platform C++ application that checks the latest ElvUI release from the TukUI API, compares it to the version installed in your World of Warcraft add-ons folder, and installs or updates ElvUI automatically.

## Features

- Detects and stores your World of Warcraft installation directory
- Queries the TukUI API for the latest ElvUI version
- Reads the installed version from the ElvUI TOC file
- Downloads and extracts the latest package
- Removes old ElvUI folders before reinstalling
- Supports Windows, MacOS, and Linux!

## Requirements

- C++20 compatible compiler
- CMake 3.20 or newer
- Ninja (recommended) or another supported generator
- vcpkg with manifest support

## Build

This project uses a vcpkg manifest in [vcpkg.json](vcpkg.json) and a CMake build defined in [CMakeLists.txt](CMakeLists.txt).

### Configure

If your vcpkg installation is available and the VCPKG_ROOT environment variable is set, configure the project with:

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
```

### Build

```bash
cmake --build build --config Release
```

### Run

```bash
./build/ElvUI-Updater
```

On Windows, the executable will typically be produced under build/Release/ or build/ depending on your generator.

## Dependencies

The project depends on:

- curl
- nlohmann-json
- libzip

These dependencies are declared in [vcpkg.json](vcpkg.json) and linked from [CMakeLists.txt](CMakeLists.txt).

## CI/CD

GitHub Actions workflows are included for automated building and release packaging:

- [.github/workflows/build.yml](.github/workflows/build.yml) builds the project on Ubuntu, Windows, and macOS for every push and pull request
- [.github/workflows/release.yml](.github/workflows/release.yml) builds release artifacts for tagged versions and publishes them as GitHub Releases

## Project Layout

- [src](src) contains the application source files
- [include](include) contains the public headers
- [CMakeLists.txt](CMakeLists.txt) defines the executable and dependency linkage
- [vcpkg.json](vcpkg.json) defines the manifest dependencies

## License

This project is licensed under the terms of the [MIT License](LICENSE.txt).
