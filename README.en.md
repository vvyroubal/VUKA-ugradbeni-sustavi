# Embedded Computer Systems — toolchain and lab starter files

*Hrvatska inačica: [README.md](README.md)*

Companion repository for the course **Ugradbeni računalni sustavi / Embedded
Computer Systems** (Karlovac University of Applied Sciences, Professional
Graduate Study of Mechatronics). It contains everything you need to build and
run the laboratory sessions on your own computer — on Linux, macOS or Windows,
with or without Docker. These are condensed instructions; handout **H1**
(Installing the Toolchain) on the course pages describes the same steps with
explanations.

| Path | Contents |
|---|---|
| `Dockerfile` | development image: ESP-IDF v5.5, Espressif's QEMU, the Lab 7 tools |
| `scripts/` | `docker-shell.sh` / `docker-shell.ps1` (shell in the container), `verify.sh` (installation check) |
| `labs/labN_*/starter/` | starter projects for Labs 1–6 |
| `labs/common/` | emulator scripts (`run_qemu*.sh`) and shared components |
| `labs/lab6_mqtt/broker/` | MQTT broker and certificates for Lab 6 |
| `labs/lab7_linux_ota/` | Buildroot configuration and OTA tools for Lab 7 |

The lab manuals, the textbook and the other course documents are distributed
separately, on the course pages. The target platform is the **ESP32-C3**
(RISC-V); every lab can also be done without a board, in the **QEMU**
emulator. The toolchain is **ESP-IDF v5.5** (a newer v5.5.x is also fine).

## Which path to choose

| System | Recommended path | Board over USB |
|---|---|---|
| **Windows 10/11** | **WSL2** (Ubuntu), and inside it the Linux instructions: Docker (Docker Desktop) or the installation without Docker | in WSL2, installation without Docker + `usbipd` |
| **macOS** | **Docker Desktop** | `esptool` from macOS, or the installation without Docker |
| **Linux** | **Docker** or the installation without Docker | Docker with `URS_PORT`, or without Docker |

All course scripts are written for `bash` on Linux (in WSL2 or in the
container); Git Bash and MSYS2 on Windows are not supported. Docker from
Windows PowerShell (`docker-shell.ps1`) is enough for building and the
emulator in Labs 1–5 and for Lab 7, but not for Lab 6 and not for the board.

| | Docker (Linux, WSL2, macOS) | Docker from PowerShell | Without Docker (Linux, WSL2, macOS) |
|---|---|---|---|
| Building and the emulator, Labs 1–5 | yes | yes | yes |
| Lab 6 (MQTT over TLS) | yes | no — use WSL2 | yes (the broker still runs in Docker) |
| Lab 7 (Buildroot, embedded Linux) | yes | yes | Linux and WSL2 with extra packages; macOS via Docker |
| Flashing the board | only on Linux | no | yes |

Docker and the installation without Docker can coexist. When you move the
same project from one kind of build to the other, run `idf.py fullclean`
first: the `build/` directory remembers the path it was configured for
(inside the container that is `/work/...`).

## Step 0: preparing the computer and getting the repository

**Windows (WSL2).** In PowerShell started as administrator run
`wsl --install`, restart the computer, open the **Ubuntu** app and choose a
Linux user name and password. Every further command is typed in the Ubuntu
terminal, and the repository is cloned into the Linux file system (`~/`),
never into `/mnt/c/...`.

**macOS.** If the `git` command does not exist, install it with
`xcode-select --install`. **Linux** (Debian/Ubuntu): `sudo apt-get install -y git`.

Get the course repository in a terminal (Linux, macOS) or in the Ubuntu
terminal (WSL2):

```sh
cd ~
git clone https://github.com/vvyroubal/urs-toolchain.git
cd urs-toolchain
```

**Docker from PowerShell only (without WSL2):** in a regular PowerShell first
install Git with `winget install Git.Git`, then open a **new** PowerShell and
run the same three lines.

The `.gitattributes` file keeps Linux line endings in the scripts even when
the repository is cloned by Git for Windows. All commands below are typed in
the repository root (`~/urs-toolchain`) unless stated otherwise.

## Path A: with Docker

1. Install Docker:
   - **Windows:** [Docker Desktop](https://www.docker.com/products/docker-desktop/)
     with the WSL2 backend (the default). In Docker Desktop enable
     *Settings → Resources → WSL integration* for **Ubuntu**.
   - **macOS:** [Docker Desktop](https://www.docker.com/products/docker-desktop/).
   - **Linux:** Docker Engine following the [official instructions](https://docs.docker.com/engine/install/),
     then add your user to the `docker` group and log out and back in (or
     run `newgrp docker` in the current terminal):
     ```sh
     sudo usermod -aG docker $USER
     ```

   On Windows and macOS **Docker Desktop must be running** while you work.
   Check: `docker run --rm hello-world` prints `Hello from Docker!`.
2. Open a shell in the container. The first run builds the `embedded-course`
   image: it takes **15–30 min** and uses **about 8 GB**.

   Linux, macOS, WSL2 (terminal):
   ```sh
   ./scripts/docker-shell.sh
   ```
   Windows (PowerShell, in the repository root):
   ```powershell
   powershell -ExecutionPolicy Bypass -File .\scripts\docker-shell.ps1
   ```
3. In the container shell, check the installation:
   ```sh
   bash scripts/verify.sh
   ```
   Success is the last line `verify.sh: PASS -- toolchain, build and QEMU all work`.

What the `docker-shell` script does: it starts a container named
`urs-toolchain`, the repository is its `/work` directory, the ESP-IDF
environment is already active, and port 8883 (the Lab 6 broker) is published
on the computer. Everything a build creates stays in the repository after you
leave (`exit` or Ctrl-D). On Linux and in WSL2 the container runs as your own
user, so the files it creates belong to you; the prompt
`I have no name!@...:/work$` is then normal. **Running the same script again
while the container runs opens a second terminal in the same container**
(that is how Lab 2 opens the terminal for GDB).

You can also run a single command without an interactive shell, e.g.
`./scripts/docker-shell.sh bash scripts/verify.sh`.

## Path B: without Docker

### Linux and WSL2

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

# 4. Espressif's QEMU, then activate again so it lands on PATH
python3 $IDF_PATH/tools/idf_tools.py install qemu-riscv32
. ~/esp-idf/export.sh

# 5. check (from the root of this repository)
cd ~/urs-toolchain
bash scripts/verify.sh
```

While cloning ESP-IDF, the warning `refs/tags/v5.5 ... is not a commit!` and
the long message about a *detached HEAD* are expected. The installation uses
**about 7 GB** (`~/esp-idf` 3.9 GB, `~/.espressif` 2.7 GB). In our test the
build and QEMU also worked without the `ninja-build`, `gperf`, `ccache` and
`dfu-util` packages (ESP-IDF brings its own tools); if you have no `sudo`,
the QEMU libraries matter most (`libgcrypt20`, `libglib2.0-0`,
`libpixman-1-0`, `libsdl2-2.0-0`, `libslirp0`).

### macOS

You need [Homebrew](https://brew.sh).

```sh
# 1. tools and libraries ESP-IDF and QEMU need
brew install cmake ninja dfu-util ccache python3 coreutils libgcrypt glib pixman sdl2 libslirp

# 2. - 5. as on Linux
git clone -b v5.5 --recursive https://github.com/espressif/esp-idf.git ~/esp-idf
~/esp-idf/install.sh esp32c3
. ~/esp-idf/export.sh
python3 $IDF_PATH/tools/idf_tools.py install qemu-riscv32
. ~/esp-idf/export.sh
cd ~/urs-toolchain
bash scripts/verify.sh
```

`coreutils` provides the `gtimeout` command the emulator scripts need.

### Windows without WSL2 (extra, optional)

The official [ESP-IDF Tools Installer](https://dl.espressif.com/dl/esp-idf/)
for **v5.5** installs ESP-IDF directly into Windows and adds an
*ESP-IDF 5.5 PowerShell* shortcut. It is useful only for flashing the board
with `idf.py` directly from Windows (in that shortcut, from the project
directory: `idf.py build`, then `idf.py -p COM<n> flash monitor`; clean a
project built in Docker first with `idf.py fullclean`). The course scripts
(`run_qemu.sh`, `verify.sh`, the Lab 6 broker) cannot run there — they need
`bash` on Linux (WSL2 or the container), and ESP-IDF does not support Git
Bash.

## Checking: `verify.sh`

`bash scripts/verify.sh` (from the repository root, with the environment
active; in the container it always is) checks `idf.py` and version v5.5,
Espressif's `qemu-system-riscv32`, builds the `hello_world` example and boots
it in the emulator. The first build takes a few minutes. The build and the
logs are in the repository: `build/verify-hello_world.log` (build) and
`build/verify-hello_world/qemu.log` (emulator). Success is the last line:

```text
verify.sh: PASS -- toolchain, build and QEMU all work
```

## Working on a lab

Docker: first `./scripts/docker-shell.sh` from the repository root — all
following commands run in that shell (under `/work`). Without Docker:
`. ~/esp-idf/export.sh`. Example for Lab 1:

```sh
cd labs/lab1_toolchain/starter
idf.py set-target esp32c3                  # first time in a project only
idf.py build
bash ../../common/run_qemu.sh build        # emulator, 30 s
```

`idf.py set-target esp32c3` is run once per project (the course projects
already set the target chip in `sdkconfig.defaults`, and it is remembered in
`sdkconfig`); running it again regenerates `sdkconfig` and discards changes
made in `menuconfig`.

`run_qemu.sh <build_dir> [timeout=30]` re-merges `flash.bin` on every run, so
the latest build always boots. The emulator ends by itself after the
timeout; the final line
`run_qemu.sh: timeout (30s) reached after output was produced — treating as success`
is normal. To stop it earlier press **Ctrl-A, then X**. The output is also
saved to `build/qemu.log`.

| Lab | Directory | Note |
|---|---|---|
| Lab 1 | `labs/lab1_toolchain/` | toolchain, first build |
| Lab 2 | `labs/lab2_gpio_irq/` | registers, interrupts, GPTimer; GDB in a second terminal |
| Lab 3 | `labs/lab3_bme280/` | I2C and the BME280; signal captures in `captures/` |
| Lab 4 | `labs/lab4_adc_pwm/` | ADC and PWM |
| Lab 5 | `labs/lab5_rtos/` | FreeRTOS |
| Lab 6 | `labs/lab6_mqtt/` | MQTT over TLS; networked emulator `run_qemu_net.sh` |
| Lab 7 | `labs/lab7_linux_ota/` | Buildroot and OTA; the first build takes 30–60 min |

What to complete and how it is checked is in each lab's manual.

**Lab 2, GDB.** Terminal 1: `./scripts/docker-shell.sh`,
`cd labs/lab2_gpio_irq/starter`, `bash ../../common/run_qemu_gdb.sh build`.
Terminal 2: `./scripts/docker-shell.sh` again (it attaches to the same
container), `cd labs/lab2_gpio_irq/starter`, then the command terminal 1
printed: `riscv32-esp-elf-gdb build/<project>.elf -ex 'target remote :1234'`.
Without Docker the second terminal needs its own `. ~/esp-idf/export.sh`.

**Labs 3 and 4 on the board.** In the emulator a simulated sensor and ADC
are used. Build the board variant in a **separate** `build-hw` directory so
the QEMU build in `build/` stays intact (from the lab's `starter` directory):

```sh
idf.py -B build-hw -D SDKCONFIG=build-hw/sdkconfig -D SDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.hw" build
idf.py -B build-hw -D SDKCONFIG=build-hw/sdkconfig -p <port> flash monitor
```

`sdkconfig.defaults` and `sdkconfig.hw` only apply when an `sdkconfig` is
created, and `idf.py` remembers the list of defaults files in the build
directory. If you build the board variant in `build/`, it silently stays the
simulator, and the way back to QEMU breaks too.

**Lab 6.** From `labs/lab6_mqtt/starter`: `bash ../broker/gen_certs.sh` (on
the computer or in the container shell), then `idf.py build` and
`bash ../../common/run_qemu_net.sh build`. Start the broker in a **host
terminal** (Linux, macOS, WSL2 — not in the container, because the script
calls `docker`): `bash labs/lab6_mqtt/broker/run_broker.sh` from the
repository root; Ctrl-C stops it. If the `urs-toolchain` container is
running, the broker joins its network (QEMU in the container reaches it at
`10.0.2.2:8883`, tools on the computer at `127.0.0.1:8883`); otherwise it
publishes port 8883 on the computer (the path without Docker). On Windows,
Lab 6 is done in WSL2.

**Lab 7.** Buildroot is built with a command typed in a **host terminal**, in
the repository root (the command starts the container itself; it is not run
in the `docker-shell` shell).

Linux and WSL2:
```sh
docker run --rm -v "$PWD:/work" -w /work embedded-course bash -lc 'make -C labs/lab7_linux_ota -f Makefile.buildroot all'
```

macOS (Docker Desktop) — building on the named volume `lab7_br`:
```sh
docker run --rm -v "$PWD:/work" -w /work -v lab7_br:/br embedded-course bash -lc 'make -C labs/lab7_linux_ota -f Makefile.buildroot all BR_DIR=/br'
```

Windows (PowerShell, Docker Desktop) — the same command on one line:
```powershell
docker run --rm -v "${PWD}:/work" -w /work -v lab7_br:/br embedded-course bash -lc 'make -C labs/lab7_linux_ota -f Makefile.buildroot all BR_DIR=/br'
```

Without Docker (Linux and WSL2 only) you also need the packages
`sudo apt-get install -y make gcc g++ patch cpio unzip rsync bc file wget qemu-system-misc`,
and then `make -C labs/lab7_linux_ota -f Makefile.buildroot all`.

## Flashing the board

The ESP32-C3 has a built-in USB interface (USB-Serial-JTAG), so no special
driver is needed — not even on Windows. **Do not install WinUSB (Zadig)**:
it removes the COM port. If the board does not appear, try another cable
(many are charge-only), another USB port, or plug it in while holding the
**BOOT** button.

From the project directory (built without Docker, or in the container on
Linux):

```sh
idf.py -p <port> flash monitor      # leave the monitor: Ctrl-]
```

| System | Port | How |
|---|---|---|
| Linux, without Docker | `/dev/ttyACM0` | user in the `dialout` group: `sudo usermod -aG dialout $USER`, then log out and in |
| Linux, Docker | `/dev/ttyACM0` | `URS_PORT=/dev/ttyACM0 ./scripts/docker-shell.sh` (a new container: leave the previous one first), then in it `idf.py -p /dev/ttyACM0 flash monitor` |
| WSL2 | `/dev/ttyACM0` | installation without Docker in WSL2, board forwarded with `usbipd` (below) and the `dialout` group |
| macOS | `/dev/cu.usbmodem*` | `esptool` (below) or the installation without Docker |
| Windows, installer | `COM<n>` | Device Manager → *USB Serial Device (COMn)* |
| Docker Desktop (macOS, Windows) | — | USB devices are not passed into the container |

**WSL2 and `usbipd`.** In PowerShell started as administrator run
`winget install usbipd`, then close the window and open a **new** PowerShell
as administrator:

```powershell
usbipd list                                      # the board's BUSID, e.g. 2-3
usbipd bind --busid <BUSID>                      # once per device
usbipd attach --wsl --busid <BUSID> --auto-attach
```

`--auto-attach` forwards the board again after every reset during flashing;
leave that window open while you work. In Ubuntu the board appears as
`/dev/ttyACM0`.

**macOS, image built in Docker.** Install `esptool` once:

```sh
python3 -m venv ~/esptool-venv
~/esptool-venv/bin/pip install esptool
```

Then, in the project's build directory (e.g. `labs/lab1_toolchain/starter/build`):

```sh
~/esptool-venv/bin/python -m esptool --chip esp32c3 -p /dev/cu.usbmodemXXXX write_flash @flash_args
~/esptool-venv/bin/python -m serial.tools.miniterm /dev/cu.usbmodemXXXX 115200   # leave: Ctrl-]
```

`ls /dev/cu.usbmodem*` shows the port name.

## Troubleshooting

- **`idf.py: command not found`** — the environment is not active in this
  terminal: `. ~/esp-idf/export.sh`.
- **`qemu-system-riscv32 not on PATH`** — after installing QEMU you must run
  `. ~/esp-idf/export.sh` again.
- **`permission denied` when calling `docker` (Linux)** — the user is not in
  the `docker` group: `sudo usermod -aG docker $USER`, then log out and in
  (or `newgrp docker`).
- **`Cannot connect to the Docker daemon` / `docker: command not found` in
  WSL2** — Docker Desktop is not running, or *Settings → Resources → WSL
  integration* is not enabled for Ubuntu.
- **Windows: PowerShell refuses to run `docker-shell.ps1`** — start it
  exactly as shown above:
  `powershell -ExecutionPolicy Bypass -File .\scripts\docker-shell.ps1`.
- **macOS: `neither 'timeout' nor 'gtimeout' found`** — `brew install coreutils`.
- **Windows: `bad interpreter` or `$'\r': command not found`** — the script
  got Windows line endings. Clone the repository again (the `.gitattributes`
  file prevents this), preferably inside WSL2.
- **The build reports that `build/` was configured for another path** (e.g.
  `/work/...`) — the project was built in Docker and now without it (or the
  other way round): `idf.py fullclean`, then `idf.py build` again.
- **Linux: files in `build/` belong to root** — left over from an older
  version of `docker-shell.sh`; take ownership back with
  `sudo chown -R $USER .` in the repository root.
- **`ModuleNotFoundError` when running `idf.py`** — another Python
  environment (venv or conda) is active; deactivate it before `export.sh`.
- **WSL2: no `/dev/ttyACM0`** — `usbipd attach` is not running, or the board
  is on another cable/port; in PowerShell check `usbipd list` (state
  *Attached*).

## A note on the Lab 7 key

`labs/lab7_linux_ota/ota/keys/demo_signing_key.INSECURE.pem` is a signing
private key that is published on purpose. It exists for the lab only; never
use it for anything else. Why this is unacceptable in a real product is the
subject of Lab 7 and of the security chapter.
