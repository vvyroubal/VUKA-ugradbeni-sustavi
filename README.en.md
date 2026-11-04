# Embedded Computer Systems — toolchain and lab starter files

*Hrvatska inačica: [README.md](README.md)*

Companion repository for the course **Ugradbeni računalni sustavi / Embedded
Computer Systems** (Karlovac University of Applied Sciences, Professional
Graduate Study of Mechatronics). It contains everything you need to build and
run the laboratory sessions on your own computer — on Linux, macOS or Windows,
with or without Docker. These are condensed instructions; handout **H1**
(Installing the Toolchain) on the course pages describes the same steps with
explanations.

| File or directory | Contents |
|---|---|
| `Dockerfile` | development image: ESP-IDF v5.5, Espressif's QEMU, the Lab 7 tools |
| `scripts/` | `docker-shell.sh` / `docker-shell.ps1` (shell in the container), `verify.sh` / `verify.ps1` (installation check), `vscode-task.*`, `gdb.sh` / `gdb.cmd` and `idf-shell.ps1` (for VS Code) |
| `.vscode/` | VS Code settings, tasks, debugger configurations (F5 for Labs 1–6) and recommended extensions |
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
container), and path C has PowerShell versions of them; Git Bash and MSYS2 on
Windows are not supported.

| System | Recommended path | Board over USB |
|---|---|---|
| **Linux** | **path A** or path B | path A with `URS_PORT`, or path B |
| **macOS** | **path A** (Docker Desktop) | `esptool` from macOS, or path B |
| **Windows 10/11** | **WSL2** (Ubuntu), and inside it the Linux instructions: path A (Docker Desktop) or path B | in WSL2, path B + `usbipd` |
| **Windows without virtualization** | **path C** (Labs 1–6) | directly, `COM<n>` |

On Windows, path A can also be run straight from PowerShell
(`docker-shell.ps1`), without Ubuntu in WSL2: it is enough for building and the emulator
in Labs 1–5 and for Lab 7, but not for Lab 6; the board is then flashed with
`esptool` from Windows. What each path covers:

| | A: Docker (Linux, WSL2, macOS) | A: Docker (Windows, PowerShell) | B: without Docker (Linux, WSL2, macOS) | C: Windows without virtualization |
|---|---|---|---|---|
| Building and the emulator, Labs 1–5 | yes | yes | yes | yes |
| Lab 6 (MQTT over TLS) | yes | no — use WSL2 | yes (the broker still runs in Docker) | yes, with Mosquitto for Windows |
| Lab 7 (Buildroot, embedded Linux) | yes | yes | Linux and WSL2 with extra packages; macOS via Docker | no |
| Flashing the board | on Linux; on macOS `esptool` from the computer | `esptool` from Windows | yes | yes |

**Order:** on every path, first Step 0, then the section of your path (A, B or
C), then *Checking* and *Working on a lab*; for path C the check and the first
run are described in the *Path C* section. *Working in VS Code* and
*Flashing the board* come once the environment works.

## Step 0: preparing the computer and getting the repository

Every system needs two things: a **code editor**, because the labs ask you to
change source code ([VS Code](https://code.visualstudio.com) is recommended,
and any other will do; working in it is described in *Working in VS Code*),
and **Git**, to get the repository. What else you need depends on the system
and the path:

| System | Install before you start | Administrator | Disk space |
|---|---|---|---|
| Linux | Git; for path A, Docker Engine (*Path A*) | `sudo` for packages | path A about 8 GB, path B about 7 GB |
| macOS | Git; for path A, Docker Desktop (*Path A*) | when installing Docker Desktop | path A about 8 GB, path B about 7 GB |
| Windows, WSL2 (paths A and B) | WSL2 with Ubuntu (`wsl --install`); for path A, Docker Desktop; for the board, `usbipd` (`winget install usbipd` or [GitHub](https://github.com/dorssel/usbipd-win/releases)) | for `wsl --install` and `usbipd` | Ubuntu a few GB; path A about 8 GB, path B about 7 GB |
| Windows, Docker from PowerShell (path A) | [Git for Windows](https://git-scm.com/download/win) (or `winget install Git.Git`) and [Docker Desktop](https://www.docker.com/products/docker-desktop/) (Windows 10 22H2 or Windows 11 23H2, 8 GB of RAM, virtualization enabled) | once, when Docker Desktop first turns on WSL2 | about 8 GB plus Docker Desktop |
| Windows, path C | Git for Windows, the ESP-IDF Tools Installer v5.5, the QEMU emulator and, for Lab 6, Mosquitto for Windows (all in the *Path C* section) | for Mosquitto (it installs as a service) | about 8 GB, plus a 1.6 GB download and about 0.3 GB per lab |

**Linux.** Git on Debian and Ubuntu: `sudo apt-get install -y git` (Fedora:
`sudo dnf install git`, Arch: `sudo pacman -S git`). The path B packages for
distributions other than Debian and Ubuntu are listed in the
[ESP-IDF documentation](https://docs.espressif.com/projects/esp-idf/en/v5.5/esp32c3/get-started/linux-macos-setup.html);
path A is the simplest choice there.

**macOS.** Type `git --version` in Terminal; if the command does not exist,
install it with `xcode-select --install`.

**Windows** offers three options (see *Which path to choose*):

- **WSL2** (recommended; paths A and B as on Linux): in PowerShell started as
  administrator run `wsl --install`, restart the computer, open the **Ubuntu**
  app and choose a Linux user name and password. Every further command is typed
  in the Ubuntu terminal, and the repository is cloned into the Linux file
  system (`~/`), never into `/mnt/c/...`. If virtualization is not enabled,
  enable it in the BIOS/UEFI (*Intel VT-x*, *AMD-V*); if the processor does not
  support it, choose path C. Git is already installed in Ubuntu.
- **Docker from PowerShell** (path A without Ubuntu in WSL2): Git for Windows
  and Docker Desktop from the table; the repository is cloned in PowerShell
  (below).
- **Path C** (no virtualization): the installation and getting the repository
  are in the *Path C* section; from this section only the table and the code
  editor apply to it.

On Windows, Git and VS Code can also be installed without administrator
rights; on its first run `winget` asks you to accept its terms — answer `Y`.

**Getting the repository.** In a terminal (Linux, macOS) or in the Ubuntu
terminal (WSL2):

```sh
cd ~
git clone https://github.com/vvyroubal/VUKA-ugradbeni-sustavi.git
cd VUKA-ugradbeni-sustavi
```

For Docker from PowerShell, run the same three lines in a **new** PowerShell,
opened after Git was installed. The `.gitattributes` file keeps Linux line
endings in the scripts even when the repository is cloned by Git for Windows.
All commands below are typed in the repository root
(`~/VUKA-ugradbeni-sustavi`) unless stated otherwise.

## Path A: with Docker

1. Install Docker:
   - **Linux:** Docker Engine following the [official instructions](https://docs.docker.com/engine/install/),
     then add your user to the `docker` group and log out and back in (or
     run `newgrp docker` in the current terminal):
     ```sh
     sudo usermod -aG docker $USER
     ```
   - **macOS:** [Docker Desktop](https://www.docker.com/products/docker-desktop/).
   - **Windows:** [Docker Desktop](https://www.docker.com/products/docker-desktop/)
     with the WSL2 backend (the default); the installer ends by asking you to
     log out or restart the computer — do so. If you work in WSL2, enable
     *Settings → Resources → WSL integration* for **Ubuntu** in Docker Desktop
     (Docker from PowerShell does not need this step).

   On its first start Docker Desktop asks you to accept the terms of use and
   offers a Docker account sign-in; signing in is not needed (*Continue
   without signing in*). On Windows and macOS **Docker Desktop must be
   running** while you work.
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
   without spaces, parentheses or diacritics (shorter than 90 characters);
   leave the defaults on the installer's other pages. Every
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
| `bash ../../common/run_qemu.sh build` | runs the built program in the emulator |
| `bash ../../common/run_qemu_net.sh build` | the same, with a network card (Lab 6) |
| `bash ../../common/run_qemu_gdb.sh build` | the emulator waits for GDB on port 1234 (Lab 2) |
| `bash ../broker/gen_certs.sh` | creates the broker's TLS certificates (Lab 6) |
| `bash ../broker/run_broker.sh` | starts the MQTT broker (Lab 6) |
| `bash scripts/verify.sh` | checks the toolchain, the build and QEMU |

*Path C — Windows, the ESP-IDF 5.5 PowerShell shortcut:*

| Command | What it does |
|---|---|
| `../../common/run_qemu.ps1 build` | runs the built program in the emulator |
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

This section and the next apply to paths A and B; on path C, checking and
working on the labs are described in the *Path C* section.

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

**A terminal with ESP-IDF.** Build and emulator commands are typed in a
terminal where ESP-IDF is active. How you open one depends on the path:

- **path A:** from the repository root run `./scripts/docker-shell.sh`
  (on Windows from PowerShell, `docker-shell.ps1`, see *Path A*). It opens a
  shell in the container, with the repository under `/work`; all following
  commands are typed there.
- **path B:** in every new terminal, first run `. ~/esp-idf/export.sh`.
- **path C:** open the **ESP-IDF 5.5 PowerShell** shortcut. On this path the
  course scripts have a `.ps1` version of the same name (table in section
  *Path C*).

**Building and the emulator.** Using Lab 1 as the example, on paths A and B
(for path C see section *Path C*):

```sh
cd labs/lab1_toolchain/starter
idf.py set-target esp32c3                  # first time only
idf.py build
bash ../../common/run_qemu.sh build        # emulator, 30 s
```

`idf.py set-target esp32c3` selects the target chip and is run only the first
time, because the choice is remembered in the `sdkconfig` file. If you run it
again, `sdkconfig` is created anew and the changes made in `menuconfig` are
lost. `idf.py build` builds the program into the `build/` directory.

`run_qemu.sh` assembles the flash image (`build/flash.bin`) from the latest
build and runs it in the emulator, so the newest program always runs. The
emulator stops by itself after 30 s; a second argument sets a different time
(e.g. `run_qemu.sh build 60`). The final line
`run_qemu.sh: timeout (30s) reached after output was produced — treating as success`
means everything is fine. To stop the emulator earlier press **Ctrl-A, then
X**. The whole output is also saved to `build/qemu.log`.

If you build the same project alternately on path A and on path B, run
`idf.py fullclean` before switching: the `build/` directory remembers the
project's path, and inside the container that path is different
(`/work/...`).

| Lab | Directory | Note |
|---|---|---|
| Lab 1 | `labs/lab1_toolchain/` | toolchain, first build |
| Lab 2 | `labs/lab2_gpio_irq/` | registers, interrupts, GPTimer; GDB in a second terminal |
| Lab 3 | `labs/lab3_bme280/` | I2C and the BME280; signal captures in `captures/` |
| Lab 4 | `labs/lab4_adc_pwm/` | ADC and PWM |
| Lab 5 | `labs/lab5_rtos/` | FreeRTOS |
| Lab 6 | `labs/lab6_mqtt/` | MQTT over TLS; networked emulator `run_qemu_net.sh` |
| Lab 7 | `labs/lab7_linux_ota/` | Buildroot and OTA; the first build takes 30–60 min |

What to complete in a lab and how it is checked is described in the lab's
manual.

**GDB from the terminal (Lab 2).** In VS Code the debugger attaches with F5
(section *Debugger: GDB attached to QEMU*). Without VS Code you need two
terminals, both in the `labs/lab2_gpio_irq/starter` directory:

1. **Terminal 1 runs QEMU waiting for the debugger:**
   `bash ../../common/run_qemu_gdb.sh build` (path C:
   `../../common/run_qemu_gdb.ps1 build`). The script prints the command to
   connect with and waits.
2. **Terminal 2 runs GDB.** Open a second terminal with ESP-IDF active: on
   path A run `./scripts/docker-shell.sh` again, which attaches to the same
   container; on path B run `. ~/esp-idf/export.sh` in a new terminal; on
   path C open another **ESP-IDF 5.5 PowerShell** shortcut window. Then change
   to `labs/lab2_gpio_irq/starter` and type the command terminal 1 printed:
   `riscv32-esp-elf-gdb build/lab2.elf -ex "target remote :1234"`.

**Labs 3 and 4 on the board.** In the emulator Lab 3 uses a simulated sensor
and Lab 4 a simulated ADC. The board variant is built in a **separate**
`build-hw` directory, so that the QEMU build in `build/` stays intact. From
the lab's `starter` directory:

```sh
idf.py -B build-hw -D SDKCONFIG=build-hw/sdkconfig -D SDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.hw" build
idf.py -B build-hw -D SDKCONFIG=build-hw/sdkconfig -p <port> flash monitor
```

A separate directory is needed because `sdkconfig.defaults` and
`sdkconfig.hw` only apply when an `sdkconfig` is created, and `idf.py`
remembers the list of those files in the build directory. If you build the
board variant in `build/`, the program still uses the simulator, without any
message saying so, and going back to QEMU no longer works properly either.

With Docker on macOS and Windows the container cannot see the board: run the
first command in the container, then flash with `esptool` from the computer,
as described in *Flashing the board*, but from the `build-hw` directory (e.g.
`labs/lab3_bme280/starter/build-hw`), not from `build`.

**Lab 6.** The order is:

1. **Certificates**, from `labs/lab6_mqtt/starter`, on the computer or in the
   container shell: `bash ../broker/gen_certs.sh`.
2. **The MQTT broker**, in a **terminal on the computer** (Linux, macOS,
   WSL2), not in the container shell, because the script calls `docker`
   itself. From the repository root: `bash labs/lab6_mqtt/broker/run_broker.sh`.
   The broker runs until you stop it with Ctrl-C, so open another terminal for
   the remaining steps.
3. **The program**, in a terminal with ESP-IDF, from `labs/lab6_mqtt/starter`:
   `idf.py build`, then `bash ../../common/run_qemu_net.sh build`.

The broker always runs in Docker (the official `eclipse-mosquitto` image), so
path B needs Docker for Lab 6 too. If the course container
(`vuka-ugradbeni-sustavi`) is running, the broker joins its network: QEMU in
the container reaches it at `10.0.2.2:8883`, and tools on the computer at
`127.0.0.1:8883`. Without the course container (path B) the broker publishes
port 8883 directly on the computer. With Docker from PowerShell Lab 6 is not
possible, because the scripts need `bash`: do it in WSL2 or on path C.

**Lab 7.** Buildroot is easiest to build in the course container. The commands
below are typed in a **terminal on the computer**, from the repository root,
not in the `docker-shell` shell, because they start the container themselves.
In VS Code the task *LV7: izgradnja Buildroota* (*Terminal → Run Task…*) does
the same and picks the method itself: in a window attached to the container it
builds there (on macOS and Windows on the `lab7_br` volume, which
`docker-shell` mounts at `/br`), and in a regular window it starts the course
container. On Linux without the course image it builds directly, with the
packages listed below. Lab 7 is not possible on path C.

**Linux and WSL2** — with the same script as the other labs. The container
then runs as your user, so the build files belong to you (a plain `docker run`
without `--user` would build as root, and you could not delete those files
without `sudo`):
```sh
./scripts/docker-shell.sh make -C labs/lab7_linux_ota -f Makefile.buildroot all
```

**macOS** (Docker Desktop) — building on the named volume `lab7_br`:
```sh
docker run --rm -v "$PWD:/work" -w /work -v lab7_br:/br embedded-course bash -lc 'make -C labs/lab7_linux_ota -f Makefile.buildroot all BR_DIR=/br'
```

**Windows** (PowerShell, Docker Desktop) — the same command, on one line:
```powershell
docker run --rm -v "${PWD}:/work" -w /work -v lab7_br:/br embedded-course bash -lc 'make -C labs/lab7_linux_ota -f Makefile.buildroot all BR_DIR=/br'
```

**Without Docker** (Linux and WSL2 only) you also need the packages
`sudo apt-get install -y make gcc g++ patch cpio unzip rsync bc file wget qemu-system-misc`,
and the build is then run directly:
`make -C labs/lab7_linux_ota -f Makefile.buildroot all`.

## Working in VS Code

The repository contains VS Code settings (`.vscode/`): recommended extensions,
tasks for building and the emulator, debugger configurations and
code-completion settings. VS Code must run **where ESP-IDF is**, so how you
open the repository depends on the path:

- **Path A (Docker, all systems).** The tools exist only in the container, so
  VS Code attaches to it. In a terminal on your computer, from the repository
  root, run `./scripts/docker-shell.sh` (`docker-shell.ps1` in PowerShell) and
  keep that terminal open: when the shell exits, the container stops. Then in
  VS Code choose F1 → *Dev Containers: Attach to Running Container…* →
  `vuka-ugradbeni-sustavi`, and in the new window *File → Open Folder* →
  `/work`. The bottom-left corner of the window shows *Container …*. Repeat
  this at the start of every session.
- **Path B, Linux and macOS.** *File → Open Folder* → the repository root.
- **Path B, WSL2.** In the Ubuntu terminal, from the repository root, type
  `code .`. VS Code is installed on Windows, and the *WSL* extension connects
  it to Ubuntu; the bottom-left corner shows *WSL: Ubuntu*.
- **Path C.** *File → Open Folder* → `C:\vuka\VUKA-ugradbeni-sustavi`.

When asked whether you trust the authors of the folder, answer *Yes, I trust
the authors*: in *Restricted Mode* the tasks do not run. VS Code then offers
the recommended extensions. *C/C++* is needed on every path, *WSL* only for
WSL2 and *Dev Containers* only for path A. You can decline the *C/C++
Extension Pack* offer. On path A and in WSL2 the *C/C++* extension must be
installed **in the container** or **in WSL**: in the *Extensions* view
(Ctrl-Shift-X), click *Install in Container…* or *Install in WSL: Ubuntu* for
*C/C++*. Espressif's *ESP-IDF* extension is not needed; do not install it: it
sets up its own, different ESP-IDF.

**Shortcuts on macOS.** The VS Code shortcuts in these instructions are given
for Windows and Linux. On macOS press **Cmd** instead of **Ctrl** (Cmd-Shift-B,
Cmd-Shift-D, Cmd-Shift-Y, Cmd-Shift-M, Cmd-Shift-X), and Cmd-Shift-P opens the
command palette (F1). On a Mac keyboard the function keys (F5, Shift-F5, F10,
F11) also need the **fn** key (e.g. fn-F5), unless *System Settings → Keyboard
→ Keyboard Shortcuts… → Function Keys → Use F1, F2, etc. keys as standard
function keys* is on. In the terminal and the emulator Ctrl stays Ctrl: QEMU
stops with Ctrl-A, then X on macOS too, and the monitor with Ctrl-].

**Running a lab:**

1. Press **Ctrl-Shift-B** (macOS: Cmd-Shift-B; the *Build and run in QEMU* task) and choose the lab
   from the list, e.g. `labs/lab1_toolchain/starter`.
2. A task tab opens in the *Terminal* panel. It shows the build, then the
   program's output from the emulator. A lab's first build takes a few minutes;
   later builds take a few seconds.
3. The emulator stops by itself after 30 seconds. The message
   `run_qemu.sh: timeout (30s) reached after output was produced — treating as success`
   means everything is fine. To stop it earlier, click in the task tab and press
   **Ctrl-A, then X**.
4. Compiler errors are listed in the *Problems* panel (Ctrl-Shift-M). Clicking
   an error opens the file at that line.

All tasks are under *Terminal → Run Task…*. Tasks without a lab in their name
are shared by Labs 1–6: you choose the lab from the list that appears **after**
choosing the task. The debugger is not in that menu but in the *Run and Debug*
view (section *Debugger*, below).

| Task | What it does |
|---|---|
| Build and run in QEMU (Ctrl-Shift-B) | `idf.py build`, then `run_qemu.sh` (Lab 6: `run_qemu_net.sh`) |
| Build | `idf.py build` |
| QEMU | runs the latest build in the emulator |
| menuconfig | `idf.py menuconfig` |
| idf.py reconfigure | after a new `Kconfig.projbuild` (Labs 1 and 4) |
| idf.py fullclean | when you build the same project alternately with and without Docker |
| QEMU for GDB, waits for the debugger | the latest build in an emulator that waits for GDB (section *Debugger*) |
| LV7: Buildroot build | builds Lab 7 (section *Working on a lab*, Lab 7) |
| LV1–LV6: build and QEMU for GDB | builds the lab, then an emulator that waits for GDB; F5 starts them too (section *Debugger*) |

The tasks find the environment by themselves (the `scripts/vscode-task.sh`
script, `.ps1` on Windows). They try, in order, an active ESP-IDF (also in the
container), `~/esp-idf` (path B) and the installation on Windows (path C). If
there is no ESP-IDF (path A with VS Code not attached to the container), the
build and emulator tasks use the course container. They all run in the same
container then, so run them one at a time: a task started while the previous
one is still running ends together with it.

**The terminal in VS Code.** On path A run `./scripts/docker-shell.sh` in the
terminal; in the window attached to the container the terminal is already in
the container, and you activate ESP-IDF there with `. $IDF_PATH/export.sh`. On
path B run `. ~/esp-idf/export.sh`, and on path C open the
**ESP-IDF 5.5 PowerShell** profile (the arrow next to `+` in the terminal
panel).

**Code completion (IntelliSense)** reads the lab's `build/compile_commands.json`,
so it works after the first build (until then *Problems* shows *Cannot find …
compile_commands.json* for labs not yet built). On path A the paths come from the
container (`/work/...`), so code completion works only in the window attached to
the container.

### Debugger: GDB attached to QEMU

The debugger consists of two programs running at the same time, connected
through network port **1234**:

- **QEMU in debug mode** (the `run_qemu_gdb.sh` script, `run_qemu_gdb.ps1` on
  path C) loads the lab's build, halts the processor before the first
  instruction and waits for a debugger to connect to port 1234. While it
  waits, the program does not run and prints nothing. The script's last line
  before waiting is `QEMU starting HALTED with gdbstub on :1234`.
- **GDB** (`riscv32-esp-elf-gdb`) reads the names of functions and variables
  and the source lines from `build/<project>.elf`, then connects to
  `localhost:1234` and controls the emulator. In VS Code the *C/C++* extension
  starts GDB (through `scripts/gdb.sh`, `scripts\gdb.cmd` on Windows), and you
  control it by clicking.

Hence two rules: QEMU must be waiting **before** GDB connects, and both must
run on the same computer, on path A in the same container.

**Preparation.** The repository must be open the way the beginning of
section *Working in VS Code* describes for your path, with the *C/C++*
extension installed where VS Code runs: on path A a window attached to the
container (bottom left *Container …*), in WSL2 a WSL window (bottom left
*WSL: Ubuntu*), on path B on Linux and macOS and on path C an ordinary window.
On path A, in a window **not** attached to the container, the debugger cannot
work (GDB would run on the computer and QEMU in the container), so the GDB
tasks then report
`with Docker, debug from VS Code attached to the course container`.

**Attaching the debugger, step by step (any lab):**

1. **Build the lab:** Ctrl-Shift-B or the *Build* task.
2. **Set a breakpoint:** in the lab's code, click left of the line number
   where the program should stop. A red dot appears (a breakpoint). Set it
   before step 4, otherwise the program runs without stopping. A breakpoint on
   a line without code (a comment, an empty line, `(void)n;`) moves to the
   next line with code.
3. **Start QEMU waiting:** *Terminal → Run Task…* → **QEMU for GDB, waits for
   the debugger** → choose the lab. The task tab shows
   `QEMU starting HALTED with gdbstub on :1234` and the emulator now waits.
   The command in a terminal, from the lab directory, does the same:
   `bash ../../common/run_qemu_gdb.sh build` (paths A and B) or
   `../../common/run_qemu_gdb.ps1 build` (path C).
4. **Attach the debugger:** open the *Run and Debug* view (Ctrl-Shift-D; macOS: Cmd-Shift-D),
   choose **GDB: attach to the waiting QEMU** in the drop-down at the top,
   press **F5** (macOS: fn-F5) and choose the program of the **same** lab from the list, e.g.
   `labs/lab1_toolchain/starter/build/lab1.elf`. GDB connects to port 1234,
   the emulator starts and the program stops at the breakpoint.

In Lab 6 the emulator for GDB has no network card, so the program cannot
connect to the broker.

**Shortcut: F5 for every lab (Labs 1–6).** Every lab has a configuration
**LV*n*: GDB in QEMU** (e.g. *LV3: GDB in QEMU*) that does steps 1, 3 and 4 by
itself. Set a breakpoint in the lab's code, choose that lab's configuration in
*Run and Debug* and press **F5**. VS Code runs the task *LV*n*: build
and QEMU for GDB* (build, then QEMU waiting), waits for the
`gdbstub on :1234` line and attaches GDB. A lab's first build takes a few
minutes. In Lab 2 the program enters the `timer_alarm_cb` interrupt handler
only once TODO 3 (T4) is solved; before that, set a breakpoint e.g. on the first
line of `app_main`. In Lab 5 `app_main` is in the file of the demo chosen in
`menuconfig` (`pipeline.c` by default).

**Working with the attached debugger:**

- When the program stops at a breakpoint, the line is highlighted in yellow.
  On the left are *Variables* and *Call Stack* (the backtrace).
- The debug toolbar at the top: *Continue* (F5, to the next breakpoint),
  *Step Over* (F10, next line), *Step Into* (F11, into a function), *Stop*
  (Shift-F5).
- Type the GDB commands from the lab manual in the *Debug Console*
  (Ctrl-Shift-Y) with the `-exec` prefix, e.g. `-exec bt`,
  `-exec info registers` and `-exec print *edata`. Copy the transcript for
  your submission from there too.
- The program's output from the emulator is in the *Terminal* panel, in the
  task tab.
- **Stop** (Shift-F5) stops GDB and the emulator, which also ends the task.
  To attach again, repeat steps 3 and 4, or just F5.

**When attaching fails:**

| Message or symptom | Cause and fix |
|---|---|
| `Unable to start debugging. Unexpected GDB output from command "-target-select remote localhost:1234". could not connect …` | QEMU is not waiting: the *QEMU for GDB* task was not started or has already ended. Start it (step 3), then F5 again. |
| `Unable to start debugging. Program path '…' is missing or invalid.` | The chosen lab is not built (step 1), or you chose another lab's program. |
| `build dir not found: build` in the task tab | The lab is not built (step 1). |
| `gdb.sh: riscv32-esp-elf-gdb not found` | VS Code does not run where ESP-IDF is: on path A the window is not attached to the container, in WSL2 the window is not a WSL window. |
| `with Docker, debug from VS Code attached to the course container` | Path A without an attached window (*Preparation*, above). |
| the program runs without stopping | No breakpoint is set, or it is on a line that never runs. |
| the program does not stop, there is no error, and *Call Stack* is empty | The debugger is not attached: the *QEMU for GDB* or *LV*n*: build and QEMU for GDB* task run from *Run Task* only starts an emulator that waits. Attach the debugger: *Run and Debug* → configuration → F5 (step 4). |
| `port 1234 is already in use` in the task tab | An old emulator is still waiting: close its task tab (the bin icon) or press Ctrl-A, then X in it, then F5 again. |

Without VS Code the same is done from two terminals (section *Working on a
lab*, *GDB from the terminal*).

## Updating the repository

The repository is extended during the semester (instructions, VS Code tasks,
scripts), and your work in the lab files stays. You get the new state in a
**host terminal**, from the repository root (on path C in PowerShell or the
ESP-IDF 5.5 PowerShell shortcut):

```sh
git pull
```

If git does not finish but reports one of these messages:

| Message | Cause and fix |
|---|---|
| `Need to specify how to reconcile divergent branches` | You committed your work. Merge it with the new state: `git pull --no-rebase --no-edit`. |
| `untracked working tree files would be overwritten by merge: …` | You created a file the repository now brings (e.g. your own `.vscode/launch.json`). Rename it (e.g. to `launch.json.mine`) and run `git pull` again. |
| `Your local changes to the following files would be overwritten by merge: …` | You changed a file the repository also changed. Put your changes aside with `git stash`, then `git pull`, then bring them back with `git stash pop`. |

After updating:

- **VS Code** loads the new tasks and configurations by itself. If the list is
  not refreshed, F1 (macOS: Cmd-Shift-P) → *Developer: Reload Window*.
- **Path A:** close the terminal running `./scripts/docker-shell.sh` and start
  it again, since only a new container picks up changes to that script. Then
  attach the VS Code window to the container again. When the instructor
  announces a new development image (a `Dockerfile` change), first remove the
  old one with `docker rmi embedded-course`; `docker-shell.sh` then builds the
  new one (15–30 min).

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

### Logic analyser (Labs 3 and 4)

The bench has logic analysers built around a Cypress FX2 chip (eight channels,
up to 24 MHz), used with the free PulseView program of the sigrok project. You
do not need one in the emulator: the Lab 3 captures are provided in
`labs/lab3_bme280/captures`.

**Installation.**

- **Linux (Debian, Ubuntu):** `sudo apt install pulseview sigrok-firmware-fx2lafw`,
  then unplug the analyser and plug it in again. If PulseView does not find it
  or reports no access, the udev rules are missing: copy `60-libsigrok.rules`
  and `61-libsigrok-uaccess.rules` from the `contrib` directory of the
  libsigrok sources to `/etc/udev/rules.d/` and run
  `sudo udevadm control --reload-rules && sudo udevadm trigger`.
- **Windows:** download the 64-bit PulseView installer from
  <https://sigrok.org/wiki/Downloads>. The analyser needs the WinUSB driver,
  set with Zadig (installed with PulseView, in the Start menu): with the
  analyser plugged in, choose *Options → List All Devices* in Zadig, select
  the analyser, pick *WinUSB* on the right and *Replace Driver*. Select the
  analyser, **never the ESP32-C3 board**: WinUSB removes the board's COM port
  (see above). An FX2 analyser sometimes needs this twice, because once
  PulseView has uploaded its firmware it shows up as a different device. The
  setting applies only to the USB port it was made on.
- **macOS:** download PulseView (`.dmg`) from the same page and drag the
  application to *Applications*. It is an x86-64 build; on a Mac with Apple
  silicon it runs through Rosetta 2, which macOS offers to install on first
  launch. If macOS refuses to open it, open it the first time with a
  right-click → *Open*.

**Wiring.** The analyser only listens. First connect its GND to the board's
GND, because without a common ground the capture is meaningless. Then for
Lab 3 (I2C) connect channel 0 to `GPIO5` (SDA) and channel 1 to `GPIO6` (SCL),
and for Lab 4 (PWM) channel 0 to `GPIO8`. The analyser reads 3.3 V levels
directly (it sees a logic one from 2.0 V and tolerates up to 5 V). Leave any
supply pins on the analyser unconnected. PulseView names the channels D0–D7.

**Capturing.** In PulseView choose *Connect to Device*, the `fx2lafw` driver
and *Scan for devices*; the analyser usually shows up as a Saleae Logic.
Enable only the channels you use and set the sample rate and sample count: for
Lab 3 2 MHz and 10 M samples (5 s, at least two sensor reads; with I2C at
100 kHz that is 20 samples per bit), for Lab 4 4 MHz and 20 M samples (5 s,
more than one breathing cycle of about 4.1 s; a 0.25 µs step is 1/800 of the
200 µs period). Then *Run*. If the capture stops early, lower the sample rate,
avoid USB hubs and try another cable.

**Decoders.** *Add protocol decoder → I²C*, with SCL = D1 and SDA = D0: START,
the address, the data bytes and ACK/NACK appear above the signals, the same
sequence you decode by hand in Lab 3. For Lab 4 the *PWM* decoder on channel D0
shows the period and duty cycle of every pulse; you can read the same values
with the cursors. For the submission attach a screenshot or the saved capture
(*File → Save*, a `.sr` file).

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
