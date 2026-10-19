# run_broker.ps1 -- Windows PowerShell counterpart of run_broker.sh for path C
# (Windows without WSL2 or Docker, handout H1): runs the Lab 6 MQTT broker
# with TLS on :8883 using the native Mosquitto for Windows instead of a
# Docker container. QEMU reaches it at 10.0.2.2:8883, tools on this computer
# at 127.0.0.1:8883.
#
# Usage, in a separate PowerShell window (Ctrl-C stops the broker), e.g. from
# labs/lab6_mqtt/starter:
#     ../broker/run_broker.ps1
#
# Needs Mosquitto for Windows (H1, path C) and the certificates from
# gen_certs.ps1. ASCII only: Windows PowerShell 5.1 reads scripts without a
# BOM as ANSI.

$ScriptDir = $PSScriptRoot
$RepoRoot = (Resolve-Path (Join-Path $ScriptDir "..\..\..")).Path
$Conf = Join-Path $ScriptDir "mosquitto-windows.conf"

$mosquitto = (Get-Command mosquitto -ErrorAction SilentlyContinue).Source
if (-not $mosquitto) {
    $candidate = Join-Path $env:ProgramFiles "mosquitto\mosquitto.exe"
    if (Test-Path $candidate) { $mosquitto = $candidate }
}
if (-not $mosquitto) {
    [Console]::Error.WriteLine("run_broker.ps1: mosquitto.exe not found -- install Mosquitto for Windows (H1, path C)")
    exit 1
}
if (-not (Test-Path (Join-Path $ScriptDir "certs\server.crt"))) {
    [Console]::Error.WriteLine("run_broker.ps1: no certs found -- run gen_certs.ps1 first")
    exit 1
}

Write-Host "== run_broker.ps1: starting Mosquitto with TLS on :8883 on this computer (Ctrl-C to stop) =="
# mosquitto-windows.conf names the certificates relative to the repository
# root, so the broker runs from there whatever directory you called it from.
Push-Location $RepoRoot
try {
    & $mosquitto -v -c $Conf
} finally {
    Pop-Location
}
