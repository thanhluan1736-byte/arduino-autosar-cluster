# Validates model/Cluster.arxml and regenerates src/Rte with arxml2rte.
# Expects the arxml2rte repo next to this one (..\arxml2rte) or its path in -Generator.
param(
    [string]$Generator = (Join-Path $PSScriptRoot "..\..\arxml2rte")
)
$ErrorActionPreference = "Stop"
$root  = Split-Path $PSScriptRoot -Parent
$model = Join-Path $root "model\Cluster.arxml"
$out   = Join-Path $root "src\Rte"

& powershell -ExecutionPolicy Bypass -File (Join-Path $Generator "validate_arxml.ps1") $model
if ($LASTEXITCODE -ne 0) { throw "ARXML validation failed" }

Get-ChildItem $out -Filter "Rte*" -ErrorAction SilentlyContinue | Remove-Item
python (Join-Path $Generator "rte_generator.py") $model $out
if ($LASTEXITCODE -ne 0) { throw "RTE generation failed" }
