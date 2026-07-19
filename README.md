# Website Path Checker (C++)

A concurrent C++20 command-line tool that checks a supplied list of website paths. It sends `HEAD` requests to minimize bandwidth and automatically retries a path with `GET` when the server replies with `405 Method Not Allowed`.

> Use this software only for systems you are explicitly authorized to test.

## Highlights

- C++20 and CMake project layout
- libcurl-based HTTP/HTTPS client with TLS validation enabled by default
- Concurrent checks with configurable worker limit
- Local UTF-8 and remote HTTP(S) lists; comments and duplicate paths are ignored
- Remote-list size limit (5 MiB) and UTF-8 BOM support
- Redirect reporting without following redirects during path checks
- Clear colored output, summary, and meaningful exit codes
- Small CTest unit-test target

## Project layout

```text
include/wpc/  Public interfaces and domain types
src/          Application implementation
tests/        Unit tests
```

## Requirements

- CMake 3.20+
- A C++20 compiler
- libcurl development package

### Dependencies

On Ubuntu/Debian:

```bash
sudo apt install cmake g++ libcurl4-openssl-dev
```

With vcpkg:

```bash
vcpkg install curl
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake
```

## Build and test

```bash
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build --output-on-failure
```

On Windows with Visual Studio, append `--config Release` to the `ctest` command too.

## Usage

```text
website-path-checker -url <HTTP_URL> [-list <FILE_OR_URL>] [--workers <1-100>] [--timeout <SECONDS>] [--no-tls-verification]
```

Examples:

```bash
# Default list: subdomains.txt in the current directory
./website-path-checker -url=https://example.com/

# Custom local list
./website-path-checker -url=https://example.com -list=paths.txt --workers=20

# Authorized use with a remote list
./website-path-checker -url=https://example.com -list=https://example.org/paths.txt --timeout=10
```

`-url` accepts either `-url=https://example.com` or `-url https://example.com`. The list contains one path per line; lines beginning with `#` are comments. A `/` entry checks the base URL.

## Exit codes

| Code | Meaning |
| --- | --- |
| 0 | Completed without network errors |
| 1 | Path-list loading error |
| 2 | Invalid command-line argument or configuration |
| 3 | One or more path checks had network errors |

## Original behavior preserved

The port preserves all functional behavior of the Python project: URL validation, path normalization and de-duplication, HTTP status categories, the `HEAD`→`GET` fallback, TLS toggle, list-size protection, concurrent processing, result summary, and exit-code semantics.
