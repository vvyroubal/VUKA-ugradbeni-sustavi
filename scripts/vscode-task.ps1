# vscode-task.ps1 -- Windows counterpart of vscode-task.sh: runs one course
# action for a lab project; the VS Code tasks in .vscode/tasks.json call it on
# Windows (Terminal > Run Task...).
#
# Usage:
#     powershell -ExecutionPolicy Bypass -File scripts\vscode-task.ps1 <action> <project>
#   action:  build | qemu | build-qemu | menuconfig | reconfigure | fullclean
#   project: lab project relative to the repository root,
#            e.g. labs/lab1_toolchain/starter
#
# Where ESP-IDF comes from: the ESP-IDF installation on Windows (path C,
# activated like the "ESP-IDF 5.5 PowerShell" shortcut); otherwise the
# course container through scripts\docker-shell.ps1 (path A from PowerShell).
# ASCII only: Windows PowerShell 5.1 reads scripts without a BOM as ANSI.

param(
    [Parameter(Position = 0)][string]$Action,
    [Parameter(Position = 1)][string]$Project
)

$Root = Split-Path -Parent $PSScriptRoot
$actions = @("build", "qemu", "build-qemu", "menuconfig", "reconfigure", "fullclean")
if (-not $Project -or $actions -notcontains $Action) {
    [Console]::Error.WriteLine("Usage: vscode-task.ps1 <build|qemu|build-qemu|menuconfig|reconfigure|fullclean> <project>")
    exit 1
}
$Project = $Project.TrimEnd("/", "\")
$ProjectDir = Join-Path $Root $Project
if (-not (Test-Path (Join-Path $ProjectDir "CMakeLists.txt"))) {
    [Console]::Error.WriteLine("vscode-task.ps1: not an ESP-IDF project: $Project")
    exit 1
}
$lab6 = $Project -match "lab6_mqtt"

. (Join-Path $PSScriptRoot "idf-shell.ps1") | Out-Null
if (Get-Command idf.py -ErrorAction SilentlyContinue) {
    $qemu = Join-Path $Root ("labs\common\" + $(if ($lab6) { "run_qemu_net.ps1" } else { "run_qemu.ps1" }))
    Set-Location $ProjectDir
    Write-Host "== $Action   (in $Project)"
    switch ($Action) {
        "build"       { idf.py build }
        "qemu"        { & $qemu build }
        "build-qemu"  { idf.py build; if ($LASTEXITCODE -eq 0) { & $qemu build } }
        "menuconfig"  { idf.py menuconfig }
        "reconfigure" { idf.py reconfigure }
        "fullclean"   { idf.py fullclean }
    }
    exit $LASTEXITCODE
}

if (Get-Command docker -ErrorAction SilentlyContinue) {
    $script = if ($lab6) { "run_qemu_net.sh" } else { "run_qemu.sh" }
    $cmd = switch ($Action) {
        "build"       { "idf.py build" }
        "qemu"        { "bash ../../common/$script build" }
        "build-qemu"  { "idf.py build && bash ../../common/$script build" }
        "menuconfig"  { "idf.py menuconfig" }
        "reconfigure" { "idf.py reconfigure" }
        "fullclean"   { "idf.py fullclean" }
    }
    $rel = $Project -replace "\\", "/"
    Write-Host "== $cmd   (in $rel, course container)"
    Set-Location $Root
    & (Join-Path $PSScriptRoot "docker-shell.ps1") bash -c "cd $rel && $cmd"
    exit $LASTEXITCODE
}

[Console]::Error.WriteLine("vscode-task.ps1: no ESP-IDF installation (H1, path C) and no docker (H1, path A)")
exit 1
