# Ugradbeni računalni sustavi — alatni lanac i polazne datoteke vježbi

*English version: [README.en.md](README.en.md)*

Repozitorij uz kolegij **Ugradbeni računalni sustavi** (Veleučilište u
Karlovcu, Stručni diplomski studij Mehatronika). Sadrži sve što treba da na
vlastitom računalu izgradite i pokrenete laboratorijske vježbe — na Linuxu,
macOS-u ili Windowsu, s Dockerom ili bez njega. Ovo su sažete upute; iste
korake, s objašnjenjima, opisuje priručnik **H1** (Instalacija alatnog lanca)
na stranicama kolegija.

| Put | Sadržaj |
|---|---|
| `Dockerfile` | razvojna slika: ESP-IDF v5.5, Espressifov QEMU, alati za LV7 |
| `scripts/` | `docker-shell.sh` / `docker-shell.ps1` (ljuska u kontejneru), `verify.sh` (provjera instalacije) |
| `labs/labN_*/starter/` | polazni projekti vježbi LV1–LV6 |
| `labs/common/` | skripte za emulator (`run_qemu*.sh`) i zajedničke komponente |
| `labs/lab6_mqtt/broker/` | MQTT posrednik i certifikati za LV6 |
| `labs/lab7_linux_ota/` | Buildroot konfiguracija i OTA alati za LV7 |

Priručnici vježbi, udžbenik i ostali dokumenti kolegija dijele se odvojeno, na
stranicama kolegija. Ciljna platforma je **ESP32-C3** (RISC-V); svaka vježba
može se izvesti i bez pločice, u emulatoru **QEMU**. Alatni lanac je
**ESP-IDF v5.5** (dopušten je i noviji v5.5.x).

## Koji put odabrati

| Sustav | Preporučeni put | Pločica preko USB-a |
|---|---|---|
| **Windows 10/11** | **WSL2** (Ubuntu), a u njemu upute za Linux: Docker (Docker Desktop) ili instalacija bez Dockera | u WSL2, instalacija bez Dockera + `usbipd` |
| **macOS** | **Docker Desktop** | `esptool` s macOS-a ili instalacija bez Dockera |
| **Linux** | **Docker** ili instalacija bez Dockera | Docker uz `URS_PORT` ili bez Dockera |

Sve skripte kolegija pisane su za `bash` na Linuxu (u WSL2 ili u kontejneru);
Git Bash i MSYS2 na Windowsu nisu podržani. Docker iz Windowsova PowerShella
(`docker-shell.ps1`) dovoljan je za izgradnju i emulator u LV1–LV5 i za LV7,
ali ne i za LV6 ni za pločicu.

| | Docker (Linux, WSL2, macOS) | Docker iz PowerShella | Bez Dockera (Linux, WSL2, macOS) |
|---|---|---|---|
| Izgradnja i emulator, LV1–LV5 | da | da | da |
| LV6 (MQTT preko TLS-a) | da | ne — upotrijebite WSL2 | da (posrednik ipak radi u Dockeru) |
| LV7 (Buildroot, ugradbeni Linux) | da | da | Linux i WSL2 uz dodatne pakete; macOS preko Dockera |
| Programiranje pločice | samo na Linuxu | ne | da |

Docker i instalacija bez Dockera mogu postojati usporedno. Kad isti projekt
prebacujete s jedne izgradnje na drugu, najprije izvedite `idf.py fullclean`:
direktorij `build/` pamti putanju za koju je konfiguriran (u kontejneru je to
`/work/...`).

## Korak 0: priprema računala i preuzimanje repozitorija

**Windows (WSL2).** U PowerShellu pokrenutom kao administrator izvedite
`wsl --install`, ponovno pokrenite računalo, otvorite aplikaciju **Ubuntu** i
zadajte korisničko ime i lozinku za Linux. Sve daljnje naredbe pišu se u
terminalu Ubuntu, a repozitorij klonirajte u Linuxov datotečni sustav (`~/`),
nikako u `/mnt/c/...`.

**macOS.** Ako naredba `git` ne postoji, instalirajte je s
`xcode-select --install`. **Linux** (Debian/Ubuntu): `sudo apt-get install -y git`.

Repozitorij kolegija preuzmite u terminalu (Linux, macOS) odnosno terminalu
Ubuntu (WSL2):

```sh
cd ~
git clone https://github.com/vvyroubal/urs-toolchain.git
cd urs-toolchain
```

**Samo za Docker iz PowerShella (bez WSL2):** u običnom PowerShellu najprije
instalirajte Git naredbom `winget install Git.Git`, zatim otvorite **novi**
PowerShell i izvedite ista tri retka.

Datoteka `.gitattributes` čuva Linuxove završetke redaka u skriptama i kad
repozitorij klonira Git za Windows. Sve naredbe u nastavku pišu se iz korijena
repozitorija (`~/urs-toolchain`), osim ako piše drukčije.

## Put A: s Dockerom

1. Instalirajte Docker:
   - **Windows:** [Docker Desktop](https://www.docker.com/products/docker-desktop/)
     s WSL2 pozadinom (zadano). U Docker Desktopu uključite
     *Settings → Resources → WSL integration* za **Ubuntu**.
   - **macOS:** [Docker Desktop](https://www.docker.com/products/docker-desktop/).
   - **Linux:** Docker Engine prema [službenim uputama](https://docs.docker.com/engine/install/),
     a zatim dodajte korisnika u grupu `docker` i odjavite se pa ponovno
     prijavite (ili u tekućem terminalu izvedite `newgrp docker`):
     ```sh
     sudo usermod -aG docker $USER
     ```

   Na Windowsu i macOS-u **Docker Desktop mora biti pokrenut** dok radite.
   Provjera: `docker run --rm hello-world` ispisuje `Hello from Docker!`.
2. Otvorite ljusku u kontejneru. Prvo pokretanje gradi sliku
   `embedded-course`: traje **15–30 min** i zauzima **oko 8 GB**.

   Linux, macOS, WSL2 (terminal):
   ```sh
   ./scripts/docker-shell.sh
   ```
   Windows (PowerShell, iz korijena repozitorija):
   ```powershell
   powershell -ExecutionPolicy Bypass -File .\scripts\docker-shell.ps1
   ```
3. U ljusci kontejnera provjerite instalaciju:
   ```sh
   bash scripts/verify.sh
   ```
   Uspjeh je zadnji redak `verify.sh: PASS -- toolchain, build and QEMU all work`.

Što skripta `docker-shell` radi: pokreće kontejner imena `urs-toolchain`,
repozitorij je u njemu direktorij `/work`, okruženje ESP-IDF-a već je aktivno,
a port 8883 (posrednik za LV6) objavljen je na računalu. Sve što izgradnja
stvori ostaje u repozitoriju i nakon izlaska (`exit` ili Ctrl-D). Na Linuxu i
u WSL2 kontejner radi pod vašim korisnikom, pa datoteke koje stvori pripadaju
vama; odzivnik `I have no name!@...:/work$` tada je normalan. **Ponovno
pokretanje iste skripte dok kontejner radi otvara drugi terminal u istom
kontejneru** (tako se u LV2 otvara terminal za GDB).

Jednu naredbu možete izvesti i bez interaktivne ljuske, npr.
`./scripts/docker-shell.sh bash scripts/verify.sh`.

## Put B: bez Dockera

### Linux i WSL2

```sh
# 1. paketi koje trebaju ESP-IDF i QEMU (Debian/Ubuntu)
sudo apt-get update
sudo apt-get install -y git wget flex bison gperf \
    python3 python3-pip python3-venv cmake ninja-build \
    ccache libffi-dev libssl-dev dfu-util libusb-1.0-0 \
    libgcrypt20 libglib2.0-0 libpixman-1-0 libsdl2-2.0-0 libslirp0

# 2. ESP-IDF v5.5 za ESP32-C3
git clone -b v5.5 --recursive https://github.com/espressif/esp-idf.git ~/esp-idf
~/esp-idf/install.sh esp32c3

# 3. aktivacija okruženja -- u SVAKOM novom terminalu
. ~/esp-idf/export.sh

# 4. Espressifov QEMU, pa ponovna aktivacija da dođe na PATH
python3 $IDF_PATH/tools/idf_tools.py install qemu-riscv32
. ~/esp-idf/export.sh

# 5. provjera (iz korijena ovoga repozitorija)
cd ~/urs-toolchain
bash scripts/verify.sh
```

Pri kloniranju ESP-IDF-a upozorenje `refs/tags/v5.5 ... is not a commit!` i
dulja poruka o stanju *detached HEAD* su očekivani. Instalacija zauzima
**oko 7 GB** (`~/esp-idf` 3,9 GB, `~/.espressif` 2,7 GB). Izgradnja i QEMU
radili su u našem testu i bez paketa `ninja-build`, `gperf`, `ccache` i
`dfu-util` (ESP-IDF donosi vlastite alate); ako nemate `sudo`, najvažnije su
biblioteke za QEMU (`libgcrypt20`, `libglib2.0-0`, `libpixman-1-0`,
`libsdl2-2.0-0`, `libslirp0`).

### macOS

Potreban je [Homebrew](https://brew.sh).

```sh
# 1. alati i biblioteke koje trebaju ESP-IDF i QEMU
brew install cmake ninja dfu-util ccache python3 coreutils libgcrypt glib pixman sdl2 libslirp

# 2. - 5. kao na Linuxu
git clone -b v5.5 --recursive https://github.com/espressif/esp-idf.git ~/esp-idf
~/esp-idf/install.sh esp32c3
. ~/esp-idf/export.sh
python3 $IDF_PATH/tools/idf_tools.py install qemu-riscv32
. ~/esp-idf/export.sh
cd ~/urs-toolchain
bash scripts/verify.sh
```

`coreutils` donosi naredbu `gtimeout`, koju trebaju skripte za emulator.

### Windows bez WSL2 (dodatno, nije obavezno)

Službeni [ESP-IDF Tools Installer](https://dl.espressif.com/dl/esp-idf/) za
**v5.5** instalira ESP-IDF izravno u Windows i dodaje prečac
*ESP-IDF 5.5 PowerShell*. Koristan je samo za programiranje pločice
naredbom `idf.py` izravno iz Windowsa (u tom prečacu, iz direktorija
projekta: `idf.py build`, pa `idf.py -p COM<n> flash monitor`; projekt
izgrađen u Dockeru najprije očistite s `idf.py fullclean`). Skripte kolegija (`run_qemu.sh`,
`verify.sh`, posrednik za LV6) ondje se ne mogu pokrenuti — trebaju `bash` na
Linuxu (WSL2 ili kontejner), a ESP-IDF ne podržava Git Bash.

## Provjera: `verify.sh`

`bash scripts/verify.sh` (iz korijena repozitorija, s aktivnim okruženjem;
u kontejneru je uvijek aktivno) provjerava `idf.py` i verziju v5.5,
Espressifov `qemu-system-riscv32`, izgrađuje primjer `hello_world` i pokreće
ga u emulatoru. Prva izgradnja traje nekoliko minuta. Izgradnja i zapisnici
nalaze se u repozitoriju: `build/verify-hello_world.log` (izgradnja) i
`build/verify-hello_world/qemu.log` (emulator). Uspjeh je zadnji redak:

```text
verify.sh: PASS -- toolchain, build and QEMU all work
```

## Rad na vježbi

Docker: najprije `./scripts/docker-shell.sh` iz korijena repozitorija — sve
sljedeće naredbe izvode se u toj ljusci (pod `/work`). Bez Dockera:
`. ~/esp-idf/export.sh`. Primjer za LV1:

```sh
cd labs/lab1_toolchain/starter
idf.py set-target esp32c3                  # samo prvi put u projektu
idf.py build
bash ../../common/run_qemu.sh build        # emulator, 30 s
```

`idf.py set-target esp32c3` izvodi se jednom po projektu (projekti kolegija
ciljni čip već postavljaju u `sdkconfig.defaults`, a on se pamti u
`sdkconfig`); ponovno pokretanje iznova stvara `sdkconfig` i briše promjene
napravljene u `menuconfig`.

`run_qemu.sh <build_dir> [timeout=30]` pri svakom pokretanju iznova spaja
`flash.bin`, pa se uvijek pokreće zadnja izgradnja. Emulator sam završava
nakon isteka vremena; završni redak
`run_qemu.sh: timeout (30s) reached after output was produced — treating as success`
je normalan. Ranije ga zaustavljate s **Ctrl-A, zatim X**. Ispis se sprema i u
`build/qemu.log`.

| Vježba | Direktorij | Napomena |
|---|---|---|
| LV1 | `labs/lab1_toolchain/` | alatni lanac, prvi build |
| LV2 | `labs/lab2_gpio_irq/` | registri, prekidi, GPTimer; GDB u drugom terminalu |
| LV3 | `labs/lab3_bme280/` | I2C i BME280; snimke signala u `captures/` |
| LV4 | `labs/lab4_adc_pwm/` | ADC i PWM |
| LV5 | `labs/lab5_rtos/` | FreeRTOS |
| LV6 | `labs/lab6_mqtt/` | MQTT preko TLS-a; mrežni emulator `run_qemu_net.sh` |
| LV7 | `labs/lab7_linux_ota/` | Buildroot i OTA; prva izgradnja traje 30–60 min |

Što treba dovršiti i kako se provjerava piše u priručniku pojedine vježbe.

**LV2, GDB.** Terminal 1: `./scripts/docker-shell.sh`,
`cd labs/lab2_gpio_irq/starter`, `bash ../../common/run_qemu_gdb.sh build`.
Terminal 2: ponovno `./scripts/docker-shell.sh` (spaja se na isti kontejner),
`cd labs/lab2_gpio_irq/starter`, pa naredba koju je ispisao terminal 1:
`riscv32-esp-elf-gdb build/<projekt>.elf -ex 'target remote :1234'`.
Bez Dockera drugi terminal treba vlastiti `. ~/esp-idf/export.sh`.

**LV3 i LV4 na pločici.** U emulatoru se koristi simulirani senzor odnosno
ADC. Inačicu za pločicu gradite u **zasebnom** direktoriju `build-hw`, da
izgradnja za QEMU u `build/` ostane netaknuta (iz direktorija `starter` vježbe):

```sh
idf.py -B build-hw -D SDKCONFIG=build-hw/sdkconfig -D SDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.hw" build
idf.py -B build-hw -D SDKCONFIG=build-hw/sdkconfig -p <port> flash monitor
```

`sdkconfig.defaults` i `sdkconfig.hw` primjenjuju se samo kad se `sdkconfig`
stvara, a `idf.py` popis datoteka postavki pamti u direktoriju izgradnje.
Gradite li inačicu za pločicu u `build/`, tiho ostaje simulator, a pokvari se
i povratak na QEMU.

**LV6.** Iz direktorija `labs/lab6_mqtt/starter`:
`bash ../broker/gen_certs.sh` (na računalu ili u ljusci kontejnera), zatim
`idf.py build` i `bash ../../common/run_qemu_net.sh build`. Posrednik
pokrenite u **terminalu računala** (Linux, macOS, WSL2 — ne u kontejneru, jer
skripta poziva `docker`): `bash labs/lab6_mqtt/broker/run_broker.sh` iz
korijena repozitorija; zaustavlja se s Ctrl-C. Ako kontejner `urs-toolchain`
radi, posrednik se pridružuje njegovoj mreži (QEMU ga u kontejneru vidi na
`10.0.2.2:8883`, alati na računalu na `127.0.0.1:8883`); inače objavljuje
port 8883 na računalu (put bez Dockera). Na Windowsu se LV6 radi u WSL2.

**LV7.** Buildroot se gradi naredbom u **terminalu računala**, iz korijena
repozitorija (naredba sama pokreće kontejner; ne izvodi se u ljusci
`docker-shell`).

Linux i WSL2:
```sh
docker run --rm -v "$PWD:/work" -w /work embedded-course bash -lc 'make -C labs/lab7_linux_ota -f Makefile.buildroot all'
```

macOS (Docker Desktop) — izgradnja na imenovanom volumenu `lab7_br`:
```sh
docker run --rm -v "$PWD:/work" -w /work -v lab7_br:/br embedded-course bash -lc 'make -C labs/lab7_linux_ota -f Makefile.buildroot all BR_DIR=/br'
```

Windows (PowerShell, Docker Desktop) — ista naredba u jednom retku:
```powershell
docker run --rm -v "${PWD}:/work" -w /work -v lab7_br:/br embedded-course bash -lc 'make -C labs/lab7_linux_ota -f Makefile.buildroot all BR_DIR=/br'
```

Bez Dockera (samo Linux i WSL2) potrebni su još paketi
`sudo apt-get install -y make gcc g++ patch cpio unzip rsync bc file wget qemu-system-misc`,
a zatim `make -C labs/lab7_linux_ota -f Makefile.buildroot all`.

## Programiranje pločice

ESP32-C3 ima ugrađeno USB sučelje (USB-Serial-JTAG), pa poseban upravljački
program nije potreban — ni na Windowsu. **Ne instalirajte WinUSB (Zadig)**:
on uklanja port COM. Ako se pločica ne pojavi, probajte drugi kabel (mnogi
služe samo za punjenje), drugi USB priključak ili je priključite držeći tipku
**BOOT**.

Iz direktorija projekta (izgrađenog bez Dockera, ili u kontejneru na Linuxu):

```sh
idf.py -p <port> flash monitor      # izlaz iz monitora: Ctrl-]
```

| Sustav | Port | Kako |
|---|---|---|
| Linux, bez Dockera | `/dev/ttyACM0` | korisnik u grupi `dialout`: `sudo usermod -aG dialout $USER`, pa odjava i prijava |
| Linux, Docker | `/dev/ttyACM0` | `URS_PORT=/dev/ttyACM0 ./scripts/docker-shell.sh` (novi kontejner: prethodni najprije zatvorite), pa u njemu `idf.py -p /dev/ttyACM0 flash monitor` |
| WSL2 | `/dev/ttyACM0` | instalacija bez Dockera u WSL2, pločica proslijeđena alatom `usbipd` (niže) i grupa `dialout` |
| macOS | `/dev/cu.usbmodem*` | `esptool` (niže) ili instalacija bez Dockera |
| Windows, instalacijski program | `COM<n>` | Upravitelj uređaja → *USB Serial Device (COMn)* |
| Docker Desktop (macOS, Windows) | — | USB uređaji ne prosljeđuju se u kontejner |

**WSL2 i `usbipd`.** U PowerShellu pokrenutom kao administrator izvedite
`winget install usbipd`, zatim zatvorite prozor i otvorite **novi**
PowerShell kao administrator:

```powershell
usbipd list                                      # BUSID pločice, npr. 2-3
usbipd bind --busid <BUSID>                      # jednom po uređaju
usbipd attach --wsl --busid <BUSID> --auto-attach
```

`--auto-attach` pločicu iznova prosljeđuje nakon svakog reseta tijekom
programiranja; ostavite taj prozor otvoren dok radite. U Ubuntuu se pločica
pojavljuje kao `/dev/ttyACM0`.

**macOS, slika izgrađena u Dockeru.** Jednom instalirajte `esptool`:

```sh
python3 -m venv ~/esptool-venv
~/esptool-venv/bin/pip install esptool
```

Zatim u direktoriju izgradnje projekta (npr. `labs/lab1_toolchain/starter/build`):

```sh
~/esptool-venv/bin/python -m esptool --chip esp32c3 -p /dev/cu.usbmodemXXXX write_flash @flash_args
~/esptool-venv/bin/python -m serial.tools.miniterm /dev/cu.usbmodemXXXX 115200   # izlaz: Ctrl-]
```

Ime porta pokazuje `ls /dev/cu.usbmodem*`.

## Česti problemi

- **`idf.py: command not found`** — okruženje nije aktivirano u ovom
  terminalu: `. ~/esp-idf/export.sh`.
- **`qemu-system-riscv32 not on PATH`** — nakon instalacije QEMU-a treba
  ponovno izvesti `. ~/esp-idf/export.sh`.
- **`permission denied` pri pozivu `docker` (Linux)** — korisnik nije u grupi
  `docker`: `sudo usermod -aG docker $USER`, pa odjava i prijava (ili
  `newgrp docker`).
- **`Cannot connect to the Docker daemon` / `docker: command not found` u
  WSL2** — Docker Desktop nije pokrenut ili za Ubuntu nije uključen
  *Settings → Resources → WSL integration*.
- **Windows: PowerShell odbija pokrenuti `docker-shell.ps1`** — pokrenite ga
  točno kako piše gore:
  `powershell -ExecutionPolicy Bypass -File .\scripts\docker-shell.ps1`.
- **macOS: `neither 'timeout' nor 'gtimeout' found`** — `brew install coreutils`.
- **Windows: `bad interpreter` ili `$'\r': command not found`** — skripta je
  dobila Windowsove završetke redaka. Ponovno klonirajte repozitorij (datoteka
  `.gitattributes` to sprečava), najbolje unutar WSL2.
- **Izgradnja javlja da je `build/` konfiguriran za drugu putanju** (npr.
  `/work/...`) — projekt je izgrađen u Dockeru, a sada bez njega (ili
  obrnuto): `idf.py fullclean`, pa ponovno `idf.py build`.
- **Linux: datoteke u `build/` pripadaju korisniku root** — ostale su od
  starije inačice skripte `docker-shell.sh`; vratite vlasništvo naredbom
  `sudo chown -R $USER .` u korijenu repozitorija.
- **`ModuleNotFoundError` pri pokretanju `idf.py`** — aktivna je neka druga
  Python okolina (venv ili conda); deaktivirajte je prije `export.sh`.
- **WSL2: nema `/dev/ttyACM0`** — `usbipd attach` nije pokrenut ili je
  pločica preko drugog kabela/priključka; u PowerShellu provjerite
  `usbipd list` (stanje *Attached*).

## Napomena o ključu u LV7

`labs/lab7_linux_ota/ota/keys/demo_signing_key.INSECURE.pem` namjerno je
objavljen privatni ključ za potpisivanje. Služi isključivo vježbi; nikad ga ne
upotrebljavajte ni za što drugo. Zašto je to u stvarnom proizvodu nedopustivo,
tema je vježbe LV7 i poglavlja o sigurnosti.
