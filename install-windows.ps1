[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$RepositoryUrl = "https://github.com/furo-hughes/Website-Path-Checker-CPP.git"
$InstallDirectory = Join-Path $env:LOCALAPPDATA "Programs\Website-Path-Checker-CPP"
$VcpkgDirectory = Join-Path $InstallDirectory ".tools\vcpkg"

function Write-Status([string]$Message) {
    Write-Host "[Website Path Checker] $Message" -ForegroundColor Cyan
}

function Install-WingetPackage([string]$PackageId, [string]$Override = "") {
    if (-not (Get-Command winget -ErrorAction SilentlyContinue)) {
        throw "winget is missing. Install App Installer from the Microsoft Store and run this script again."
    }

    Write-Status "Installing or updating $PackageId"
    $Arguments = @("install", "--exact", "--id", $PackageId, "--accept-package-agreements", "--accept-source-agreements")
    if ($Override) {
        $Arguments += @("--override", $Override)
    }

    & winget @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "winget could not install $PackageId."
    }
}

function Find-Executable([string]$CommandName, [string]$FallbackPath) {
    $Command = Get-Command $CommandName -ErrorAction SilentlyContinue
    if ($Command) {
        return $Command.Source
    }
    if (Test-Path $FallbackPath) {
        return $FallbackPath
    }
    throw "$CommandName could not be found after installation."
}

if (-not (Get-Command git -ErrorAction SilentlyContinue) -and
    -not (Test-Path "$env:ProgramFiles\Git\cmd\git.exe")) {
    Install-WingetPackage "Git.Git"
}

if (-not (Get-Command cmake -ErrorAction SilentlyContinue) -and
    -not (Test-Path "$env:ProgramFiles\CMake\bin\cmake.exe")) {
    Install-WingetPackage "Kitware.CMake"
}

$VsWherePath = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (-not (Test-Path $VsWherePath)) {
    Install-WingetPackage "Microsoft.VisualStudio.2022.BuildTools" `
        "--wait --passive --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended"
}

$Git = Find-Executable "git" "$env:ProgramFiles\Git\cmd\git.exe"
$Cmake = Find-Executable "cmake" "$env:ProgramFiles\CMake\bin\cmake.exe"

if (Test-Path (Join-Path $InstallDirectory ".git")) {
    Write-Status "Updating existing repository"
    & $Git -C $InstallDirectory pull --ff-only
    if ($LASTEXITCODE -ne 0) {
        throw "Repository update failed. Resolve local Git changes in $InstallDirectory and run this script again."
    }
} elseif (Test-Path $InstallDirectory) {
    throw "The installation directory exists but is not the expected Git repository: $InstallDirectory"
} else {
    New-Item -ItemType Directory -Path (Split-Path -Parent $InstallDirectory) -Force | Out-Null
    Write-Status "Cloning repository"
    & $Git clone $RepositoryUrl $InstallDirectory
    if ($LASTEXITCODE -ne 0) {
        throw "Repository clone failed."
    }
}

if (-not (Test-Path (Join-Path $InstallDirectory "CMakeLists.txt"))) {
    throw "The repository does not contain CMakeLists.txt and cannot be built as this C++ project."
}

if (-not (Test-Path $VcpkgDirectory)) {
    New-Item -ItemType Directory -Path (Split-Path -Parent $VcpkgDirectory) -Force | Out-Null
    Write-Status "Downloading vcpkg"
    & $Git clone --depth 1 https://github.com/microsoft/vcpkg.git $VcpkgDirectory
    if ($LASTEXITCODE -ne 0) {
        throw "vcpkg clone failed."
    }
}

$Vcpkg = Join-Path $VcpkgDirectory "vcpkg.exe"
if (-not (Test-Path $Vcpkg)) {
    Write-Status "Bootstrapping vcpkg"
    & (Join-Path $VcpkgDirectory "bootstrap-vcpkg.bat") -disableMetrics
    if ($LASTEXITCODE -ne 0) {
        throw "vcpkg bootstrap failed."
    }
}

Write-Status "Installing libcurl"
& $Vcpkg install curl:x64-windows
if ($LASTEXITCODE -ne 0) {
    throw "libcurl installation failed."
}

$BuildDirectory = Join-Path $InstallDirectory "build"
Write-Status "Configuring release build"
& $Cmake -S $InstallDirectory -B $BuildDirectory -G "Visual Studio 17 2022" -A x64 `
    "-DCMAKE_TOOLCHAIN_FILE=$VcpkgDirectory\scripts\buildsystems\vcpkg.cmake"
if ($LASTEXITCODE -ne 0) {
    throw "CMake configuration failed."
}

Write-Status "Building release executable"
& $Cmake --build $BuildDirectory --config Release
if ($LASTEXITCODE -ne 0) {
    throw "Build failed."
}

$Executable = Join-Path $BuildDirectory "Release\website-path-checker.exe"
$DependencyDirectory = Join-Path $VcpkgDirectory "installed\x64-windows\bin"
Copy-Item (Join-Path $DependencyDirectory "*.dll") (Split-Path -Parent $Executable) -Force

Write-Host "Installation completed." -ForegroundColor Green
Write-Host "Program: $Executable" -ForegroundColor Green
& $Executable --help
