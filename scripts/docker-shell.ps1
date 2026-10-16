# docker-shell.ps1 -- open a shell (or run one command) in the course container,
# with this repository mounted at /work. Builds the image on first use.
#
# Usage (PowerShell, from the repository root). Windows does not run
# downloaded scripts by default, so start it like this:
#     powershell -ExecutionPolicy Bypass -File .\scripts\docker-shell.ps1
#     powershell -ExecutionPolicy Bypass -File .\scripts\docker-shell.ps1 bash scripts/verify.sh
#
# Second terminal: while a course container is running, running this script
# again attaches to THAT container (needed for GDB in Lab 2). The container is
# named vuka-ugradbeni-sustavi and publishes port 8883 (the Lab 6 MQTT
# broker).

$Root = Split-Path -Parent $PSScriptRoot
$Image = if ($env:URS_IMAGE) { $env:URS_IMAGE } else { "embedded-course" }
$Name = if ($env:URS_CONTAINER) { $env:URS_CONTAINER } else { "vuka-ugradbeni-sustavi" }
$Cmd = if ($args.Count -gt 0) { $args } else { @("bash") }

# `docker ... -q` prints an id or nothing; it never writes to stderr when the
# image/container is simply absent, so this works in Windows PowerShell 5.1.
$running = docker ps -q --filter "name=^/$Name$"
if ($running) {
    Write-Host "== attaching to the running '$Name' container"
    docker exec -it -w /work $Name /opt/esp/entrypoint.sh @Cmd
    exit $LASTEXITCODE
}

$have = docker images -q $Image
if (-not $have) {
    Write-Host "== building image '$Image' (first use: 15-30 min, ~8 GB)"
    docker build -t $Image $Root
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}

docker run --rm -it --name $Name -p 8883:8883 -v "${Root}:/work" -w /work `
    -e IDF_GIT_SAFE_DIR=/work $Image @Cmd
exit $LASTEXITCODE
