# run_qemu_gdb.ps1 -- Windows PowerShell counterpart of run_qemu_gdb.sh for
# path C (Windows without WSL2 or Docker, handout H1). Boots an ESP-IDF
# esp32c3 build in Espressif's QEMU HALTED at reset, with the GDB stub on
# tcp::1234, and prints the GDB command for a second shortcut window.
#
# Usage, in the "ESP-IDF 5.5 PowerShell" shortcut, from the project directory:
#     ../../common/run_qemu_gdb.ps1 build
#
# flash.bin is (re)merged on every run, as in run_qemu_gdb.sh. QEMU runs
# until you quit it (Ctrl-A then X) or GDB kills it; output also goes to
# <build_dir>/qemu.log.
#
# ASCII only: Windows PowerShell 5.1 reads scripts without a BOM as ANSI.

param(
    [Parameter(Position = 0)][string]$BuildDir
)
$Timeout = 0   # no time limit: QEMU waits for GDB

$Me = "run_qemu_gdb.ps1"
if (-not $BuildDir) {
    [Console]::Error.WriteLine("Usage: $Me <build_dir>")
    exit 1
}
if (-not (Test-Path -PathType Container $BuildDir)) {
    [Console]::Error.WriteLine("${Me}: build dir not found: $BuildDir")
    exit 1
}
$BuildDir = (Resolve-Path $BuildDir).Path
$FlashArgs = Join-Path $BuildDir "flash_args"
$FlashBin = Join-Path $BuildDir "flash.bin"
$LogFile = Join-Path $BuildDir "qemu.log"

# Always re-merge when the build's flash_args is there: idf.py build does not
# remove an older flash.bin, and booting the previous image after a rebuild
# is the most confusing failure a student can hit.
if (Test-Path $FlashArgs) { Remove-Item -Force $FlashBin -ErrorAction SilentlyContinue }
if (-not (Test-Path $FlashBin)) {
    if (-not (Test-Path $FlashArgs)) {
        [Console]::Error.WriteLine("${Me}: no flash.bin and no flash_args in $BuildDir")
        exit 1
    }
    [Console]::Error.WriteLine("${Me}: merging $FlashBin from $FlashArgs ...")
    Push-Location $BuildDir
    # "@flash_args" must be quoted: PowerShell would treat a bare @ itself.
    python -m esptool --chip esp32c3 merge_bin -o flash.bin --fill-flash-size 4MB "@flash_args" --format raw | Out-Null
    Pop-Location
    if (-not (Test-Path $FlashBin)) {
        [Console]::Error.WriteLine("${Me}: merge_bin failed to produce $FlashBin (is this the ESP-IDF 5.5 PowerShell shortcut?)")
        exit 1
    }
}

$Qemu = Get-Command qemu-system-riscv32 -ErrorAction SilentlyContinue
if (-not $Qemu) {
    [Console]::Error.WriteLine("${Me}: qemu-system-riscv32 not found on PATH (install it, then reopen the shortcut)")
    exit 1
}

$Elf = Get-ChildItem -Path $BuildDir -Filter *.elf -File | Select-Object -First 1
if (-not $Elf) {
    [Console]::Error.WriteLine("${Me}: no .elf found directly in $BuildDir")
    exit 1
}
$rel = Resolve-Path -Relative $Elf.FullName
[Console]::Error.WriteLine("==============================================================")
[Console]::Error.WriteLine("${Me}: QEMU starting HALTED with gdbstub on :1234")
[Console]::Error.WriteLine("In a second 'ESP-IDF 5.5 PowerShell' window, in this directory, run:")
[Console]::Error.WriteLine("==============================================================")
Write-Host "riscv32-esp-elf-gdb $rel -ex `"target remote :1234`""

# QEMU's output goes to the screen and to qemu.log line by line; its input
# stays on this console, so Ctrl-A then X still reaches it.
$psi = New-Object System.Diagnostics.ProcessStartInfo
$psi.FileName = $Qemu.Source
$psi.Arguments = "-M esp32c3 -nographic -s -S -drive `"file=$FlashBin,if=mtd,format=raw`""
$psi.UseShellExecute = $false
$psi.RedirectStandardOutput = $true
$psi.RedirectStandardError = $true
$psi.StandardOutputEncoding = [System.Text.Encoding]::UTF8
$psi.StandardErrorEncoding = [System.Text.Encoding]::UTF8

$log = New-Object System.IO.StreamWriter($LogFile, $false, (New-Object System.Text.UTF8Encoding($false)))
$proc = [System.Diagnostics.Process]::Start($psi)
$deadline = (Get-Date).AddSeconds($Timeout)
$timedOut = $false
$out = $proc.StandardOutput.ReadLineAsync()
$err = $proc.StandardError.ReadLineAsync()
while ($out -or $err) {
    $pending = @($out, $err) | Where-Object { $_ }
    $i = [System.Threading.Tasks.Task]::WaitAny([System.Threading.Tasks.Task[]]$pending, 200)
    if ($i -ge 0) {
        $t = $pending[$i]
        $line = $t.Result
        if ($null -eq $line) {
            if ($t -eq $out) { $out = $null } else { $err = $null }
        } else {
            Write-Host $line
            $log.WriteLine($line); $log.Flush()
            if ($t -eq $out) { $out = $proc.StandardOutput.ReadLineAsync() }
            else { $err = $proc.StandardError.ReadLineAsync() }
        }
    }
    if ($Timeout -gt 0 -and -not $timedOut -and (Get-Date) -ge $deadline -and -not $proc.HasExited) {
        $timedOut = $true
        $proc.Kill()
    }
}
$proc.WaitForExit()
$log.Close()

if ($timedOut) {
    if ((Get-Item $LogFile).Length -gt 0) {
        [Console]::Error.WriteLine("${Me}: timeout (${Timeout}s) reached after output was produced -- treating as success")
        exit 0
    }
    [Console]::Error.WriteLine("${Me}: timeout (${Timeout}s) reached with no output -- treating as failure")
    exit 1
}
if ($proc.ExitCode -eq 0) { exit 0 }
[Console]::Error.WriteLine("${Me}: qemu-system-riscv32 exited with status $($proc.ExitCode)")
exit 1
