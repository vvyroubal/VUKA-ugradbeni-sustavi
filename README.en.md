# Embedded Computer Systems — toolchain and lab starter files

*Hrvatska inačica: [README.md](README.md)*

Companion repository for the course **Ugradbeni računalni sustavi / Embedded
Computer Systems** (Karlovac University of Applied Sciences, Professional
Graduate Study of Mechatronics). It contains everything you need to build and
run the laboratory sessions on your own computer — on Linux, macOS or Windows,
with or without Docker:

| Path | Contents |
|---|---|
| `Dockerfile` | development image: ESP-IDF v5.5, Espressif's QEMU, the Lab 7 tools |
| `scripts/` | `docker-shell.sh` / `docker-shell.ps1` (enter the container), `verify.sh` (installation check) |
| `labs/labN_*/starter/` | starter projects for Labs 1–6 |
| `labs/common/` | emulator scripts (`run_qemu*.sh`) and shared components |
| `labs/lab6_mqtt/broker/` | MQTT broker and certificates for Lab 6 |
| `labs/lab7_linux_ota/` | Buildroot configuration and OTA tools for Lab 7 |

The lab manuals, the textbook and the other course documents are distributed
separately, on the course pages. The target platform is the **ESP32-C3**
(RISC-V); every lab can also be done without a board, in the **QEMU**
emulator.

## Which path to choose

| | Docker | Without Docker |
|---|---|---|
| Setup | one command, the same on every system | install ESP-IDF and QEMU on the computer |
| Building and the emulator (Labs 1–6) | yes | yes |
| Lab 7 (Buildroot, embedded Linux) | yes | only on Linux and in WSL2 |
| Flashing the board over USB | only on Linux | yes |

Recommendation: start with **Docker**. Once you have a board, on macOS and
Windows add the installation without Docker (or WSL2) as well — the two can
coexist.

On Windows, clone the repository inside WSL2 or, if you clone it with Git for
Windows, leave it as it is: the `.gitattributes` file keeps Linux line endings
in the scripts.

```sh
git clone https://github.com/vvyroubal/urs-toolchain.git
cd urs-toolchain
```

## Path A: with Docker

1. Install Docker: [Docker Desktop](https://www.docker.com/products/docker-desktop/)
   on Windows (with the WSL2 backend, which is the default) and macOS, or
   Docker Engine on Linux.
2. Open a shell in the container. The first run builds the `embedded-course`
   image, which takes several minutes and downloads a few GB.

   Linux, macOS, WSL2:
   ```sh
   ./scripts/docker-shell.sh
   ```
   Windows (PowerShell):
   ```powershell
   .\scripts\docker-shell.ps1
   ```
3. Inside the container, check the installation:
   ```sh
   bash scripts/verify.sh
   ```
   The script must end with the line `verify.sh: PASS`.

Inside the container the repository is available as `/work`, the ESP-IDF
environment is already active, and everything the build produces stays in the
repository after you leave. You can also build the image by hand
(`docker build -t embedded-course .`) and start it with `docker run` as the
lab manuals show.

## Path B: without Docker

### Linux (and WSL2 on Windows)

On **Windows**, install WSL2 first: in PowerShell started as administrator run
`wsl --install`, restart the computer and open the Ubuntu terminal. Every
further command is typed there, and the repository should be cloned into the
Linux file system (`~/`), not under `/mnt/c/...`.

```sh
# 1. packages ESP-IDF and QEMU need (Debian/Ubuntu)
sudo apt-get update
sudo apt-get install -y git wget flex bison gperf \
    python3 python3-pip python3-venv cmake ninja-build \
    ccache libffi-dev libssl-dev dfu-util libusb-1.0-0 \
    libgcrypt20 libglib2.0-0 libpixman-1-0 libsdl2-2.0-0 libslirp0

# 2. ESP-IDF v5.5 for the ESP32-C3
git clone -b v5.5 --recursive https://github.com/espressif/esp-idf.git ~/esp-idf
~/esp-idf/install.sh esp32c3

# 3. activate the environment -- in EVERY new terminal
. ~/esp-idf/export.sh

# 4. Espressif's QEMU, then re-activate so it lands on PATH
python3 $IDF_PATH/tools/idf_tools.py install qemu-riscv32
. ~/esp-idf/export.sh

# 5. check (from the root of this repository)
bash scripts/verify.sh
```

### macOS

[Homebrew](https://brew.sh) is required.

```sh
# 1. tools and libraries ESP-IDF and QEMU need
brew install cmake ninja dfu-util ccache python3 coreutils \
    libgcrypt glib pixman sdl2 libslirp

# 2. - 5. as on Linux
git clone -b v5.5 --recursive https://github.com/espressif/esp-idf.git ~/esp-idf
~/esp-idf/install.sh esp32c3
. ~/esp-idf/export.sh
python3 $IDF_PATH/tools/idf_tools.py install qemu-riscv32
. ~/esp-idf/export.sh
bash scripts/verify.sh
```

`coreutils` provides the `gtimeout` command the emulator scripts need.

### Windows without WSL2

The official [ESP-IDF Tools Installer](https://dl.espressif.com/dl/esp-idf/)
for **v5.5** installs the toolchain directly into Windows and adds the
*ESP-IDF 5.5 CMD* and *ESP-IDF 5.5 PowerShell* shortcuts. That path is enough
for building (`idf.py build`) and flashing the board, but the scripts in this
repository need `bash`, and Lab 7 needs Linux. So combine it with Docker
(path A) or use WSL2 instead.

## Working on a lab

Example for Lab 1 (from the repository root; the same inside and outside the
container):

```sh
cd labs/lab1_toolchain/starter
idf.py set-target esp32c3
idf.py build
bash ../../common/run_qemu.sh build        # run in the emulator, 30 s
```

The emulator output is also saved to `build/qemu.log`. The other labs have the
same shape; what has to be completed and how it is checked is in each lab's
manual.

| Lab | Directory | Note |
|---|---|---|
| Lab 1 | `labs/lab1_toolchain/` | toolchain, first build |
| Lab 2 | `labs/lab2_gpio_irq/` | registers, interrupts, GPTimer |
| Lab 3 | `labs/lab3_bme280/` | I2C and the BME280; signal captures in `captures/` |
| Lab 4 | `labs/lab4_adc_pwm/` | ADC and PWM |
| Lab 5 | `labs/lab5_rtos/` | FreeRTOS |
| Lab 6 | `labs/lab6_mqtt/` | MQTT over TLS; networked emulator `run_qemu_net.sh` |
| Lab 7 | `labs/lab7_linux_ota/` | Buildroot and OTA; the build takes 30–60 min |

In the emulator, Labs 3 and 4 use a simulated sensor and ADC; for the board
you build with the extra `sdkconfig.hw` settings layer, as the manual
describes.

**Lab 7 on macOS and Windows:** Buildroot has to be built on a Docker named
volume, not in the mounted repository directory:

```sh
docker run --rm -v "$PWD:/work" -w /work -v lab7_br:/br embedded-course \
    bash -lc 'make -C labs/lab7_linux_ota -f Makefile.buildroot all BR_DIR=/br'
```

## Flashing the board

```sh
idf.py -p <port> flash monitor      # leave the monitor with Ctrl-]
```

The ESP32-C3 has a built-in USB interface, so no special driver is normally
needed.

| System | Port | Note |
|---|---|---|
| Linux | `/dev/ttyACM0` | the user must be in the `dialout` group: `sudo usermod -aG dialout $USER`, then log out and in |
| macOS | `/dev/cu.usbmodem*` | |
| Windows (installer) | `COM<n>` | the port number is shown in Device Manager |
| WSL2 | `/dev/ttyACM0` | the board must be passed through with `usbipd`, see below |
| Docker on Linux | `/dev/ttyACM0` | `URS_PORT=/dev/ttyACM0 ./scripts/docker-shell.sh` |
| Docker Desktop (macOS, Windows) | — | USB devices are not passed into containers |

**WSL2:** in PowerShell started as administrator:

```powershell
winget install usbipd
usbipd list                           # find the board's BUSID, e.g. 2-3
usbipd bind --busid <BUSID>           # once per device
usbipd attach --wsl --busid <BUSID>   # after every plug-in
```

## Common problems

- **`idf.py: command not found`** — the environment is not active in this
  terminal: `. ~/esp-idf/export.sh`.
- **`qemu-system-riscv32 not found on PATH`** — after installing QEMU you
  have to run `. ~/esp-idf/export.sh` again.
- **macOS: `neither 'timeout' nor 'gtimeout' found`** — `brew install coreutils`.
- **Windows: `bad interpreter` or `$'\r': command not found`** — the script
  got Windows line endings. Clone the repository again (the `.gitattributes`
  file prevents this) or clone it inside WSL2.
- **Linux, after working in Docker: files under `build/` belong to root** —
  the container runs as root; restore ownership with `sudo chown -R $USER .`
  or delete the `build/` directory from inside the container.
- **`ModuleNotFoundError` when running `idf.py`** — another Python
  environment (venv or conda) is active; deactivate it before `export.sh`.
- **Windows: PowerShell refuses to run `docker-shell.ps1`** —
  `powershell -ExecutionPolicy Bypass -File .\scripts\docker-shell.ps1`.

## A note on the Lab 7 key

`labs/lab7_linux_ota/ota/keys/demo_signing_key.INSECURE.pem` is a signing
private key that is published on purpose. It exists for the lab only; never
use it for anything else. Why this is unacceptable in a real product is the
subject of Lab 7 and of the security chapter.
