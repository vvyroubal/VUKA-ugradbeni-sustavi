# gen_certs.ps1 -- Windows PowerShell counterpart of gen_certs.sh for path C
# (Windows without WSL2 or Docker, handout H1): generates the self-signed TLS
# material for Lab 6's MQTT broker into this directory's certs/ and copies
# ca.crt into the project(s) at main/certs/ca.crt (all .gitignore'd).
#
# Usage, in the "ESP-IDF 5.5 PowerShell" shortcut, e.g. from
# labs/lab6_mqtt/starter:
#     ../broker/gen_certs.ps1
#
# Needs openssl: the one that comes with Git for Windows is found by itself.
# The broker certificate has CN=10.0.2.2 and subjectAltName IP:10.0.2.2,
# IP:127.0.0.1 (QEMU's address of this computer, and this computer itself).
# ASCII only: Windows PowerShell 5.1 reads scripts without a BOM as ANSI.

# openssl prints progress on stderr, which Windows PowerShell 5.1 turns into
# error records -- success is judged by the exit code instead.
$ErrorActionPreference = "Continue"
$ScriptDir = $PSScriptRoot
$LabDir = Split-Path -Parent $ScriptDir
$CertDir = Join-Path $ScriptDir "certs"
$Days = "3650"

$openssl = (Get-Command openssl -ErrorAction SilentlyContinue).Source
if (-not $openssl) {
    foreach ($git in @("$env:ProgramFiles\Git", "$env:LOCALAPPDATA\Programs\Git")) {
        $candidate = Join-Path $git "usr\bin\openssl.exe"
        if (Test-Path $candidate) { $openssl = $candidate; break }
    }
}
if (-not $openssl) {
    [Console]::Error.WriteLine("gen_certs.ps1: openssl not found -- install Git for Windows (H1, path C)")
    exit 1
}

function Invoke-OpenSsl {
    & $openssl @args 2>&1 | ForEach-Object { "$_" } | Out-Null
    if ($LASTEXITCODE -ne 0) { throw "openssl $($args[0]) failed" }
}

New-Item -ItemType Directory -Force $CertDir | Out-Null
Push-Location $CertDir
try {
    Write-Host "== gen_certs.ps1: CA =="
    Invoke-OpenSsl genrsa -out ca.key 2048
    Invoke-OpenSsl req -x509 -new -nodes -key ca.key -sha256 -days $Days `
        -subj "/C=HR/O=VUKA/OU=Ugradbeni racunalni sustavi/CN=VUKA Lab6 CA" -out ca.crt

    Write-Host "== gen_certs.ps1: server cert (CN=10.0.2.2, SAN IP:10.0.2.2,127.0.0.1) =="
    Invoke-OpenSsl genrsa -out server.key 2048
    Invoke-OpenSsl req -new -key server.key `
        -subj "/C=HR/O=VUKA/OU=Ugradbeni racunalni sustavi/CN=10.0.2.2" -out server.csr
    Set-Content -Path server.ext -Value "subjectAltName=IP:10.0.2.2,IP:127.0.0.1" -Encoding Ascii
    Invoke-OpenSsl x509 -req -in server.csr -CA ca.crt -CAkey ca.key -CAcreateserial `
        -days $Days -sha256 -extfile server.ext -out server.crt
    Remove-Item -Force server.csr, server.ext, ca.srl -ErrorAction SilentlyContinue
} finally {
    Pop-Location
}

Write-Host "== gen_certs.ps1: distributing ca.crt to projects =="
foreach ($proj in "starter", "solution") {
    $main = Join-Path $LabDir "$proj\main"
    if (-not (Test-Path $main)) { continue }   # students have no solution/
    New-Item -ItemType Directory -Force (Join-Path $main "certs") | Out-Null
    Copy-Item -Force (Join-Path $CertDir "ca.crt") (Join-Path $main "certs\ca.crt")
    Write-Host "  -> $proj/main/certs/ca.crt"
}
Write-Host "== gen_certs.ps1: done. Certs in $CertDir (not committed) =="
