# docker-shell.ps1 -- open a shell (or run one command) in the course container,
# with this repository mounted at /work. Builds the image on first use.
#
# Usage (PowerShell, from anywhere inside the repository):
#     .\scripts\docker-shell.ps1
#     .\scripts\docker-shell.ps1 bash scripts/verify.sh
#
# If PowerShell refuses to run the script, start it as:
#     powershell -ExecutionPolicy Bypass -File .\scripts\docker-shell.ps1

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot
$Image = if ($env:URS_IMAGE) { $env:URS_IMAGE } else { "embedded-course" }

docker image inspect $Image *> $null
if ($LASTEXITCODE -ne 0) {
    Write-Host "== building image '$Image' (first use; takes several minutes)"
    docker build -t $Image $Root
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}

$Cmd = if ($args.Count -gt 0) { $args } else { @("bash") }
docker run --rm -it -v "${Root}:/work" -w /work -e IDF_GIT_SAFE_DIR=/work $Image @Cmd
exit $LASTEXITCODE
