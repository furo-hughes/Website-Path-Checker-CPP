#!/usr/bin/env bash

set -euo pipefail

readonly REPOSITORY_URL="https://github.com/furo-hughes/Website-Path-Checker-CPP.git"
readonly INSTALL_DIRECTORY="$HOME/Library/Application Support/Website-Path-Checker-CPP"
readonly EXECUTABLE_LINK="$HOME/.local/bin/website-path-checker"

status() {
    printf '[Website Path Checker] %s\n' "$1"
}

if ! xcode-select -p >/dev/null 2>&1; then
    xcode-select --install
    printf 'Finish the Xcode Command Line Tools installation prompt, then run this script again.\n' >&2
    exit 1
fi

if ! command -v brew >/dev/null 2>&1; then
    status "Installing Homebrew"
    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
    if [[ -x /opt/homebrew/bin/brew ]]; then
        eval "$(/opt/homebrew/bin/brew shellenv)"
    elif [[ -x /usr/local/bin/brew ]]; then
        eval "$(/usr/local/bin/brew shellenv)"
    else
        printf 'Homebrew installation did not complete successfully.\n' >&2
        exit 1
    fi
fi

status "Installing prerequisites"
brew install git cmake ninja curl pkg-config

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

CURL_ROOT="$(brew --prefix curl)"
status "Configuring release build"
cmake -S "$INSTALL_DIRECTORY" -B "$INSTALL_DIRECTORY/build" -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCURL_ROOT="$CURL_ROOT" \
    -DCMAKE_PREFIX_PATH="$CURL_ROOT"

status "Building release executable"
cmake --build "$INSTALL_DIRECTORY/build"
ctest --test-dir "$INSTALL_DIRECTORY/build" --output-on-failure

mkdir -p "$HOME/.local/bin"
ln -sfn "$INSTALL_DIRECTORY/build/website-path-checker" "$EXECUTABLE_LINK"

status "Installation completed"
printf 'Program: %s\n' "$EXECUTABLE_LINK"
"$INSTALL_DIRECTORY/build/website-path-checker" --help
