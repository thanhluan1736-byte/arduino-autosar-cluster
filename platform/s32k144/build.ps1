# build.ps1 - builds the S32K144 firmware with the toolchain of S32 Design Studio 3.6 (no IDE needed)
#   powershell -ExecutionPolicy Bypass -File platform/s32k144/build.ps1          (build)
#   powershell -ExecutionPolicy Bypass -File platform/s32k144/build.ps1 -Clean   (clean first)
param(
    [string]$S32DS = "C:\NXP\S32DS.3.6.11\S32DS",
    [switch]$Clean
)
$ErrorActionPreference = "Stop"
$gcc  = Join-Path $S32DS "build_tools\gcc_v11.4\gcc-11.4-arm32-eabi\bin"
$make = Join-Path $S32DS "build_tools\msys32\usr\bin"
if (-not (Test-Path (Join-Path $gcc "arm-none-eabi-gcc.exe"))) { throw "arm-none-eabi-gcc not found under $gcc (set -S32DS)" }
$env:PATH = "$gcc;$make;$env:PATH"
Push-Location $PSScriptRoot
try {
    if ($Clean) { & make clean }
    $toolchain = "TOOLCHAIN=" + ($gcc -replace '\\', '/')
    & make $toolchain -j4
    if ($LASTEXITCODE -ne 0) { throw "build failed" }
} finally {
    Pop-Location
}
