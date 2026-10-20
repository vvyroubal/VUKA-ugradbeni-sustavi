# idf-shell.ps1 -- PowerShell with ESP-IDF active, like the "ESP-IDF 5.5
# PowerShell" shortcut (path C, handout H1), but usable anywhere: the VS Code
# terminal profile "ESP-IDF 5.5 PowerShell" (.vscode/settings.json) starts it.
#
# Usage:
#     powershell -NoExit -ExecutionPolicy Bypass -File scripts\idf-shell.ps1
# or dot-source it from another script to activate ESP-IDF there.
#
# The ESP-IDF Tools Installer records the selected installation in
# esp_idf.json under IDF_TOOLS_PATH (default C:\Espressif); its
# Initialize-Idf.ps1 needs that id.
# ASCII only: Windows PowerShell 5.1 reads scripts without a BOM as ANSI.

function Enable-EspIdf {
    if (Get-Command idf.py -ErrorAction SilentlyContinue) { return $true }
    $tools = if ($env:IDF_TOOLS_PATH) { $env:IDF_TOOLS_PATH } else { "C:\Espressif" }
    $config = Join-Path $tools "esp_idf.json"
    $init = Join-Path $tools "Initialize-Idf.ps1"
    if (-not ((Test-Path $config) -and (Test-Path $init))) { return $false }
    $id = (Get-Content $config -Raw | ConvertFrom-Json).idfSelectedId
    . $init -IdfId $id *> $null
    return [bool](Get-Command idf.py -ErrorAction SilentlyContinue)
}

if (Enable-EspIdf) {
    Write-Host "ESP-IDF is active ($env:IDF_PATH)"
} else {
    Write-Host "idf-shell.ps1: ESP-IDF not found -- install it with the ESP-IDF Tools Installer (H1, path C)"
}
