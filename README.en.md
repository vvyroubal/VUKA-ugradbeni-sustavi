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
| `scripts/` | `docker-shell.sh` / `docker-shell.ps1` (shell in the container), `verify.sh` / `verify.ps1` (installation check), `vscode-task.*` and `idf-shell.ps1` (for VS Code) |
| `.vscode/` | VS Code settings, tasks and recommended extensions |
| `labs/labN_*/starter/` | starter projects for Labs 1–6 |
| `labs/common/` | emulator scripts (`run_qemu*.sh`, for path C `run_qemu*.ps1`) and shared components |
| `labs/lab6_mqtt/broker/` | MQTT broker and certificates for Lab 6 |
| `labs/lab7_linux_ota/` | Buildroot configuration and OTA tools for Lab 7 |

The lab manuals, the textbook and the other course documents are distributed
separately, on the course pages. The target platform is the **ESP32-C3**
(RISC-V); every lab can also be done without a board, in the **QEMU**
emulator. The toolchain is **ESP-IDF v5.5** (a newer v5.5.x is also fine).

## Which path to choose

The instructions offer three paths:

- **Path A — with Docker**: the toolchain comes ready-made in an image,
  the same on every system; the simplest path.
- **Path B — without Docker**: ESP-IDF is installed directly on Linux, in WSL2
  or on macOS; more steps, but it sees the board directly. Paths A and B can
  coexist.
- **Path C — Windows without virtualization**: for computers on which WSL2 and
  Docker do not work; ESP-IDF is installed directly in Windows, and the course
  scripts run in PowerShell versions of the same name (`.ps1`).

The course scripts are written for `bash` on Linux (directly, in WSL2 or in the
container); Git Bash and MSYS2 on Windows are not supported.

| System | Recommended path | Board over USB |
|---|---|---|
| **Windows 10/11** | **WSL2** (Ubuntu), and inside it the Linux instructions: path A (Docker Desktop) or path B | in WSL2, path B + `usbipd` |
| **macOS** | **path A** (Docker Desktop) | `esptool` from macOS, or path B |
| **Linux** | **path A** or path B | path A with `URS_PORT`, or path B |
| **Windows without virtualization** | **path C** (Labs 1–6) | directly, `COM<n>` |

On Windows, path A can also be run straight from PowerShell
(`docker-shell.ps1`), without Ubuntu in WSL2: it is enough for building and the emulator
in Labs 1–5 and for Lab 7, but not for Lab 6 and not for the board. What each
path covers:

| | A: Docker (Linux, WSL2, macOS) | A: Docker (Windows, PowerShell) | B: without Docker (Linux, WSL2, macOS) | C: Windows without virtualization |
|---|---|---|---|---|
| Building and the emulator, Labs 1–5 | yes | yes | yes | yes |
| Lab 6 (MQTT over TLS) | yes | no — use WSL2 | yes (the broker still runs in Docker) | yes, with Mosquitto for Windows |
| Lab 7 (Buildroot, embedded Linux) | yes | yes | Linux and WSL2 with extra packages; macOS via Docker | no |
| Flashing the board | only on Linux | no | yes | yes |

When you move the same project from a path A build to a path B build or the
other way round, run `idf.py fullclean` first: the `build/` directory remembers
the path it was configured for (inside the container that is `/work/...`).

## Step 0: preparing the computer and getting the repository

**Windows** has three options (see "Which path to choose"):

- **WSL2** (recommended; paths A and B as on Linux): in PowerShell started as
  administrator run `wsl --install`, restart the computer, open the **Ubuntu**
  app and choose a Linux user name and password. Every further command is typed
  in the Ubuntu terminal, and the repository is cloned into the Linux file
  system (`~/`), never into `/mnt/c/...`. If virtualization is not enabled,
  enable it in the BIOS/UEFI (*Intel VT-x*, *AMD-V*); if the processor does not
  support it, choose path C.
- **Docker from PowerShell** (path A without Ubuntu in WSL2): Git for Windows
  (`winget install Git.Git`) and Docker Desktop (path A, below); the repository
  is cloned in PowerShell (below).
- **Path C** (no virtualization): has its own preparation, including getting
  the repository — follow the *Path C* section and skip the rest of this one.

Before you start, install (Git and VS Code can also be installed without
administrator rights):

| Option | Install before you start | Administrator | Disk space |
|---|---|---|---|
| WSL2 (paths A and B) | WSL2 with Ubuntu (`wsl --install`); for path A, Docker Desktop; for the board, `usbipd` (`winget install usbipd` or [GitHub](https://github.com/dorssel/usbipd-win/releases)) | for `wsl --install` and `usbipd` | Ubuntu a few GB; path A about 8 GB, path B about 7 GB |
| Docker from PowerShell (path A) | [Git for Windows](https://git-scm.com/download/win) (or `winget install Git.Git`) and [Docker Desktop](https://www.docker.com/products/docker-desktop/) (Windows 10 22H2 or Windows 11 23H2, 8 GB of RAM, virtualization enabled) | once, when Docker Desktop first turns on WSL2 | about 8 GB plus Docker Desktop |
| Path C | Git for Windows, the ESP-IDF Tools Installer v5.5, the QEMU emulator and, for Lab 6, Mosquitto for Windows (all in the *Path C* section) | for Mosquitto (it installs as a service) | about 8 GB, plus a 1.6 GB download and about 0.3 GB per lab |

**Code editor:** [VS Code](https://code.visualstudio.com) is recommended, and
any other will do. In WSL2, open the project from the Ubuntu terminal with
`code .` at the repository root (VS Code then works directly on the files in
Linux, with the *WSL* extension, which it offers by itself). Tasks and the rest
are described in the *Working in VS Code* section.

**macOS.** If the `git` command does not exist, install it with
`xcode-select --install`. **Linux** (Debian/Ubuntu): `sudo apt-get install -y git`
(Fedora: `sudo dnf install git`, Arch: `sudo pacman -S git`; the path B packages
for those distributions are listed in the [ESP-IDF documentation](https://docs.espressif.com/projects/esp-idf/en/v5.5/esp32c3/get-started/linux-macos-setup.html),
and path A is the simplest choice there).

Get the course repository in a terminal (Linux, macOS) or in the Ubuntu
terminal (WSL2):

```sh
cd ~
git clone https://github.com/vvyroubal/VUKA-ugradbeni-sustavi.git
cd VUKA-ugradbeni-sustavi
```

**Docker from PowerShell only (without Ubuntu in WSL2):** in a regular PowerShell first
install Git with `winget install Git.Git`, then open a **new** PowerShell and
run the same three lines.

The `.gitattributes` file keeps Linux line endings in the scripts even when
the repository is cloned by Git for Windows. All commands below are typed in
the repository root (`~/VUKA-ugradbeni-sustavi`) unless stated otherwise.

## Path A: with Docker

1. Install Docker:
   - **Windows:** [Docker Desktop](https://www.docker.com/products/docker-desktop/)
     with the WSL2 backend (the default). If you work in WSL2, enable
     *Settings → Resources → WSL integration* for **Ubuntu** in Docker Desktop
     (Docker from PowerShell does not need this step).
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
`vuka-ugradbeni-sustavi`, the repository is its `/work` directory, the ESP-IDF
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
cd ~/VUKA-ugradbeni-sustavi
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
cd ~/VUKA-ugradbeni-sustavi
bash scripts/verify.sh
```

`coreutils` provides the `gtimeout` command the emulator scripts need.

## Path C: Windows without virtualization (no WSL2, no Docker)

For computers on which WSL2 and Docker do not work because the processor does
not support virtualization. First check whether it is merely switched off in the
BIOS/UEFI (*Intel VT-x* or *AMD-V*) — if it can be switched on, follow the WSL2
instructions. The QEMU emulator does not need virtualization (it emulates the
chip in software), so this path covers Labs 1–6 and the board; Lab 7 (Buildroot)
needs Linux, so do it on a computer with WSL2, Docker or Linux (arrange it with
the instructor). A 64-bit Windows 10 or 11 is required.

1. **Git for Windows** (<https://git-scm.com/download/win> or
   `winget install Git.Git`); it also brings `openssl`, which the
   `gen_certs.ps1` script uses for the Lab 6 certificates.
2. The official [ESP-IDF Tools Installer](https://dl.espressif.com/dl/esp-idf/),
   version **v5.5** (on the page: *ESP-IDF v5.5.x Offline Installer*). Installation directory: the default, or a short path
   without spaces, parentheses or diacritics (shorter than 90 characters). Every
   `idf.py` command is typed in the **ESP-IDF 5.5 PowerShell** shortcut.
3. In that shortcut install the emulator, then close the shortcut and open it again:
   ```powershell
   python $env:IDF_PATH\tools\idf_tools.py install qemu-riscv32
   ```
4. Clone the repository into a path without spaces or diacritics (a user folder
   like `C:\Users\Ivan Čović` does not suit ESP-IDF):
   ```powershell
   mkdir C:\vuka
   cd C:\vuka
   git clone https://github.com/vvyroubal/VUKA-ugradbeni-sustavi.git
   cd VUKA-ugradbeni-sustavi
   ```
5. For Lab 6: [Mosquitto for Windows](https://mosquitto.org/download/) (64-bit;
   it needs an administrator because it installs as a service). The installer
   is not signed, so at the *Windows protected your PC* warning choose
   *More info* → *Run anyway*.

**The course scripts on Windows.** Every course script has a PowerShell
version **of the same name with the `.ps1` extension**, with the same arguments
and the same behaviour (the emulator stops by itself after the given time,
Ctrl-A then X stops it earlier, and the output is also saved to
`build/qemu.log`). They are run in the *ESP-IDF 5.5 PowerShell* shortcut, which
allows running scripts. The shortcut opens in the ESP-IDF directory, so every
time you open it, first change into the repository.

**Checking**, with the PowerShell version of `verify.sh`:
```powershell
cd C:\vuka\VUKA-ugradbeni-sustavi
./scripts/verify.ps1
```
The last line is what matters, `verify.ps1: PASS -- toolchain, build and QEMU
all work`. The first run of Lab 1 then looks like this:
```powershell
cd labs\lab1_toolchain\starter
idf.py set-target esp32c3
idf.py build
../../common/run_qemu.ps1 build
```

**Commands from the lab manuals** are written for bash, as on paths A and B.
On path C you run the PowerShell versions of the same name: leave out `bash`,
replace the `.sh` extension with `.ps1` and run the command from the same
directory.

*Paths A and B — Linux, macOS, WSL2 and the container (bash):*

| Command | What it does |
|---|---|
| `bash ../../common/run_qemu.sh build` | runs the build in the emulator |
| `bash ../../common/run_qemu_net.sh build` | the same, with a network card (Lab 6) |
| `bash ../../common/run_qemu_gdb.sh build` | the emulator waits for GDB on port 1234 (Lab 2) |
| `bash ../broker/gen_certs.sh` | creates the broker's TLS certificates (Lab 6) |
| `bash ../broker/run_broker.sh` | starts the MQTT broker (Lab 6) |
| `bash scripts/verify.sh` | checks the toolchain, the build and QEMU |

*Path C — Windows, the ESP-IDF 5.5 PowerShell shortcut:*

| Command | What it does |
|---|---|
| `../../common/run_qemu.ps1 build` | runs the build in the emulator |
| `../../common/run_qemu_net.ps1 build` | the same, with a network card (Lab 6) |
| `../../common/run_qemu_gdb.ps1 build` | the emulator waits for GDB (Lab 2); start GDB in a second shortcut window, with the command the script prints |
| `../broker/gen_certs.ps1` | creates the broker's TLS certificates (Lab 6) |
| `../broker/run_broker.ps1` | starts the Mosquitto for Windows broker (Lab 6), in a separate shortcut window |
| `./scripts/verify.ps1` | checks the toolchain, the build and QEMU |

On path C, type a command that the manual continues with `\` on one line.

For a quick try, ESP-IDF's own `idf.py qemu monitor` works too (it builds,
runs and shows the output; leave with Ctrl-]).

**Lab 6:** the emulator reaches the broker (`run_broker.ps1`) at
`10.0.2.2:8883`, tools on the computer at `127.0.0.1:8883`. If Windows asks,
allow access on private networks. `mosquitto_sub` and `mosquitto_pub` are in
`C:\Program Files\mosquitto`, so the Lab 6 manual's command, from
`labs\lab6_mqtt\starter`, reads:
```powershell
& "C:\Program Files\mosquitto\mosquitto_sub.exe" -h 127.0.0.1 -p 8883 --cafile ../broker/certs/ca.crt -t "vuka/sensors/#" -v
```

**The board:** `idf.py -p COM<n> flash monitor`, with the port from Device Manager.

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
repository root; Ctrl-C stops it. If the `vuka-ugradbeni-sustavi` container is
running, the broker joins its network (QEMU in the container reaches it at
`10.0.2.2:8883`, tools on the computer at `127.0.0.1:8883`); otherwise it
publishes port 8883 on the computer (the path without Docker). With Docker
from PowerShell, Lab 6 is not possible (the scripts need `bash`): do it in WSL2
or on path C.

**Lab 7.** Buildroot is built in the course container, in a **host terminal**
at the repository root.

Linux and WSL2 — with the same script as the other labs; the container runs
as your own user, so the build files belong to you (a plain `docker run`
without `--user` would build as root and leave files you cannot delete
without `sudo`):
```sh
./scripts/docker-shell.sh make -C labs/lab7_linux_ota -f Makefile.buildroot all
```

On macOS and Windows the command starts the container itself (it is not run
in the `docker-shell` shell).

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

## Working in VS Code

The repository contains VS Code settings (`.vscode/`): recommended extensions,
tasks and code-completion settings. Open the **repository root** (*File → Open
Folder*; in WSL2 `code .` from the Ubuntu terminal, on path C
`C:\vuka\VUKA-ugradbeni-sustavi`). When asked whether you trust the authors
of the folder, answer *Yes, I trust the authors* — in *Restricted Mode* the
tasks do not run. VS Code then offers the *C/C++*, *WSL* (needed on Windows
only) and *Dev Containers* (for path A) extensions; you can decline the
*C/C++ Extension Pack* offer. Espressif's *ESP-IDF* extension is not needed; do
not install it — it sets up its own, different ESP-IDF.

**Tasks** (*Terminal → Run Task…*, Ctrl-Shift-B to build and run), after which
you choose the lab:

| Task | What it does |
|---|---|
| Build and run in QEMU (Ctrl-Shift-B) | `idf.py build`, then `run_qemu.sh` (Lab 6: `run_qemu_net.sh`) |
| Build | `idf.py build` |
| QEMU | runs the latest build in the emulator |
| menuconfig | `idf.py menuconfig` |
| idf.py reconfigure | after a new `Kconfig.projbuild` (Labs 1 and 4) |
| idf.py fullclean | when the same project moves between Docker and the build without it |

The tasks find the environment by themselves (the `scripts/vscode-task.sh`
script, `.ps1` on Windows): an active ESP-IDF, `~/esp-idf` (path B), the
installation on Windows (path C), and if there is no ESP-IDF, the course
container (path A). On path A a task attaches to a container that is already
running, so run tasks one at a time.

**Terminal:** on path A `./scripts/docker-shell.sh`, on path B
`. ~/esp-idf/export.sh`; on path C the **ESP-IDF 5.5 PowerShell** profile (the
arrow next to `+` in the terminal panel).

**Code completion (IntelliSense)** reads the lab's `build/compile_commands.json`,
so it works after the first build (until then *Problems* shows *Cannot find …
compile_commands.json* for labs not yet built). On path A the paths come from the container
(`/work/...`): run `./scripts/docker-shell.sh`, then F1 → *Dev Containers:
Attach to Running Container…* → `vuka-ugradbeni-sustavi` and open the `/work`
folder.

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
| Windows, path C | `COM<n>` | Device Manager → *USB Serial Device (COMn)* |
| Windows, Docker from PowerShell | `COM<n>` | `esptool` from Windows (below) |
| Docker Desktop (macOS, Windows) | — | USB devices are not passed into the container |

**Windows, Docker from PowerShell.** The container does not see USB, so the
program built in the container is written with `esptool` from Windows. Install
the ESP-IDF Tools Installer (path C, step 2) and, in the *ESP-IDF 5.5 PowerShell*
shortcut, in the project's build directory (e.g.
`labs\lab1_toolchain\starter\build`), run (replace `COM5` with your port; the
quotes around `@flash_args` are needed, because PowerShell interprets `@`
itself):
```powershell
python -m esptool --chip esp32c3 -p COM5 write_flash "@flash_args"
python -m serial.tools.miniterm COM5 115200      # quit: Ctrl-]
```

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
- **Windows, path C: `running scripts is disabled on this system`** — the
  `.ps1` script was started in a regular PowerShell; run it in the
  *ESP-IDF 5.5 PowerShell* shortcut, which allows running scripts.
- **Windows, path C: `Python was not found; run without arguments to install
  from the Microsoft Store`, or `idf.py` is not recognized** — the command was
  typed in a regular PowerShell; open the *ESP-IDF 5.5 PowerShell* shortcut. If
  `qemu-system-riscv32` is missing as well, the shortcut was not reopened after
  installing QEMU.
- **WSL2: no `/dev/ttyACM0`** — `usbipd attach` is not running, or the board
  is on another cable/port; in PowerShell check `usbipd list` (state
  *Attached*).

## A note on the Lab 7 key

`labs/lab7_linux_ota/ota/keys/demo_signing_key.INSECURE.pem` is a signing
private key that is published on purpose. It exists for the lab only; never
use it for anything else. Why this is unacceptable in a real product is the
subject of Lab 7 and of the security chapter.
