# verify.ps1 -- Windows PowerShell counterpart of verify.sh for path C
# (Windows without WSL2 or Docker, handout H1): checks that the toolchain
# works end to end -- idf.py, Espressif's QEMU, a hello_world build for
# esp32c3 and a boot in the emulator.
#
# Usage, in the "ESP-IDF 5.5 PowerShell" shortcut, from the repository root:
#     ./scripts/verify.ps1
#
# The build and the logs go to build/verify-hello_world in the repository.
# ASCII only: Windows PowerShell 5.1 reads scripts without a BOM as ANSI.

$Root = Split-Path -Parent $PSScriptRoot
$BuildDir = Join-Path $Root "build\verify-hello_world"

function Fail([string]$msg) {
    [Console]::Error.WriteLine("verify.ps1: FAIL -- $msg")
    exit 1
}

Write-Host "== 1/4 idf.py"
if (-not (Get-Command idf.py -ErrorAction SilentlyContinue)) {
    Fail "idf.py not found (use the 'ESP-IDF 5.5 PowerShell' shortcut)"
}
$version = (idf.py --version 2>$null | Select-Object -Last 1)
Write-Host "   $version"
if ("$version" -notmatch "v5\.5") { Fail "expected ESP-IDF v5.5, got: $version" }
if (-not $env:IDF_PATH) { Fail "IDF_PATH is not set" }

Write-Host "== 2/4 qemu-system-riscv32"
$qemu = Get-Command qemu-system-riscv32 -ErrorAction SilentlyContinue
if (-not $qemu) {
    Fail ("qemu-system-riscv32 not on PATH (run: python `$env:IDF_PATH\tools\idf_tools.py " +
          "install qemu-riscv32, then reopen the shortcut)")
}
if (-not ((& $qemu.Source -M help 2>$null) -match "esp32c3")) {
    Fail "this qemu-system-riscv32 has no esp32c3 machine (not the Espressif build)"
}
Write-Host "   $($qemu.Source)"

Write-Host "== 3/4 build hello_world for esp32c3 (first build takes a few minutes)"
New-Item -ItemType Directory -Force (Join-Path $Root "build") | Out-Null
$example = Join-Path $env:IDF_PATH "examples\get-started\hello_world"
$buildLog = "$BuildDir.log"
idf.py -C $example -B $BuildDir -D "SDKCONFIG=$BuildDir\sdkconfig" set-target esp32c3 build *> $buildLog
if ($LASTEXITCODE -ne 0) {
    Get-Content $buildLog -Tail 20 | ForEach-Object { [Console]::Error.WriteLine($_) }
    Fail "build failed (full log: $buildLog)"
}

Write-Host "== 4/4 boot in QEMU"
& (Join-Path $Root "labs\common\run_qemu.ps1") $BuildDir 20 *> $null
$qemuLog = Join-Path $BuildDir "qemu.log"
if (-not (Test-Path $qemuLog) -or -not (Select-String -Path $qemuLog -Pattern "Hello world!" -SimpleMatch -Quiet)) {
    Fail "'Hello world!' not found in $qemuLog"
}
Write-Host "verify.ps1: PASS -- toolchain, build and QEMU all work"
