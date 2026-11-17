# flash.ps1 - programs build/cluster_ecu.hex into the MaaZEDU board through its J-Link (SWD), then runs it
#   powershell -ExecutionPolicy Bypass -File platform/s32k144/flash.ps1
#   add -Serial <n> when more than one J-Link is connected
param(
    [string]$JLink = "C:\Program Files\SEGGER\JLink_V980\JLink.exe",
    [string]$Serial = ""
)
$ErrorActionPreference = "Stop"
$hex = Join-Path $PSScriptRoot "build\cluster_ecu.hex"
if (-not (Test-Path $hex)) { throw "$hex not found: run build.ps1 first" }
if (-not (Test-Path $JLink)) { throw "JLink.exe not found: install the SEGGER J-Link software or pass -JLink" }

$script = Join-Path $env:TEMP "cluster_ecu_flash.jlink"
@"
r
h
loadfile "$hex"
r
g
exit
"@ | Set-Content -Encoding ascii $script

$args = @("-device", "S32K144", "-if", "SWD", "-speed", "4000", "-autoconnect", "1", "-NoGui", "1", "-CommanderScript", $script)
if ($Serial -ne "") { $args = @("-SelectEmuBySN", $Serial) + $args }
& $JLink @args
if ($LASTEXITCODE -ne 0) { throw "J-Link programming failed" }
