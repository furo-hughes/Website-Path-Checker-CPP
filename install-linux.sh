#!/usr/bin/env bash

set -euo pipefail

readonly REPOSITORY_URL="https://github.com/furo-hughes/Website-Path-Checker-CPP.git"
readonly DATA_HOME="${XDG_DATA_HOME:-$HOME/.local/share}"
readonly INSTALL_DIRECTORY="$DATA_HOME/Website-Path-Checker-CPP"
readonly EXECUTABLE_LINK="$HOME/.local/bin/website-path-checker"

status() {
    printf '[Website Path Checker] %s\n' "$1"
}

install_dependencies() {
    if command -v apt-get >/dev/null 2>&1; then
        sudo apt-get update
        sudo apt-get install -y git build-essential cmake ninja-build pkg-config libcurl4-openssl-dev
    elif command -v dnf >/dev/null 2>&1; then
        sudo dnf install -y git gcc-c++ cmake ninja-build pkgconf-pkg-config libcurl-devel
    elif command -v pacman >/dev/null 2>&1; then
        sudo pacman -Sy --needed --noconfirm git base-devel cmake ninja pkgconf curl
    else
        printf 'Unsupported package manager. Install Git, a C++20 compiler, CMake, Ninja, pkg-config, and libcurl development files.\n' >&2
        exit 1
    fi
}

if ! command -v git >/dev/null 2>&1 || ! command -v cmake >/dev/null 2>&1 || ! command -v ninja >/dev/null 2>&1; then
    status "Installing prerequisites"
    install_dependencies
fi

if [[ -d "$INSTALL_DIRECTORY/.git" ]]; then
    status "Updating existing repository"
    git -C "$INSTALL_DIRECTORY" pull --ff-only
elif [[ -e "$INSTALL_DIRECTORY" ]]; then
    printf 'Installation directory exists but is not the expected Git repository: %s\n' "$INSTALL_DIRECTORY" >&2
    exit 1
else
    mkdir -p "$(dirname "$INSTALL_DIRECTORY")"
    status "Cloning repository"
    git clone "$REPOSITORY_URL" "$INSTALL_DIRECTORY"
fi

if [[ ! -f "$INSTALL_DIRECTORY/CMakeLists.txt" ]]; then
    printf 'The repository does not contain CMakeLists.txt and cannot be built as this C++ project.\n' >&2
    exit 1
fi

status "Configuring release build"
cmake -S "$INSTALL_DIRECTORY" -B "$INSTALL_DIRECTORY/build" -G Ninja -DCMAKE_BUILD_TYPE=Release

status "Building release executable"
cmake --build "$INSTALL_DIRECTORY/build"
ctest --test-dir "$INSTALL_DIRECTORY/build" --output-on-failure

mkdir -p "$HOME/.local/bin"
ln -sfn "$INSTALL_DIRECTORY/build/website-path-checker" "$EXECUTABLE_LINK"

status "Installation completed"
printf 'Program: %s\n' "$EXECUTABLE_LINK"
"$INSTALL_DIRECTORY/build/website-path-checker" --help
