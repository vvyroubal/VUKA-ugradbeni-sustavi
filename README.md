# Ugradbeni računalni sustavi — alatni lanac i polazne datoteke vježbi

*English version: [README.en.md](README.en.md)*

Repozitorij uz kolegij **Ugradbeni računalni sustavi** (Veleučilište u
Karlovcu, Stručni diplomski studij Mehatronika). Sadrži sve što treba da na
vlastitom računalu izgradite i pokrenete laboratorijske vježbe — na Linuxu,
macOS-u ili Windowsu, s Dockerom ili bez njega. Ovo su sažete upute; iste
korake, s objašnjenjima, opisuje priručnik **H1** (Instalacija alatnog lanca)
na stranicama kolegija.

| Datoteka ili direktorij | Sadržaj |
|---|---|
| `Dockerfile` | razvojna slika: ESP-IDF v5.5, Espressifov QEMU, alati za LV7 |
| `scripts/` | `docker-shell.sh` / `docker-shell.ps1` (ljuska u kontejneru), `verify.sh` / `verify.ps1` (provjera instalacije), `vscode-task.*` i `idf-shell.ps1` (za VS Code) |
| `.vscode/` | postavke, zadaci i preporučena proširenja za VS Code |
| `labs/labN_*/starter/` | polazni projekti vježbi LV1–LV6 |
| `labs/common/` | skripte za emulator (`run_qemu*.sh`, za put C `run_qemu*.ps1`) i zajedničke komponente |
| `labs/lab6_mqtt/broker/` | MQTT posrednik i certifikati za LV6 |
| `labs/lab7_linux_ota/` | Buildroot konfiguracija i OTA alati za LV7 |

Priručnici vježbi, udžbenik i ostali dokumenti kolegija dijele se odvojeno, na
stranicama kolegija. Ciljna platforma je **ESP32-C3** (RISC-V); svaka vježba
može se izvesti i bez pločice, u emulatoru **QEMU**. Alatni lanac je
**ESP-IDF v5.5** (dopušten je i noviji v5.5.x).

## Koji put odabrati

Upute nude tri puta:

- **Put A — s Dockerom**: alatni lanac dolazi gotov u slici, isti na
  svim sustavima; najjednostavniji put.
- **Put B — bez Dockera**: ESP-IDF se instalira izravno na Linux, u WSL2 ili na
  macOS; više koraka, ali izravno vidi pločicu. Putovi A i B mogu postojati
  usporedno.
- **Put C — Windows bez virtualizacije**: za računala na kojima WSL2 i Docker
  ne rade; ESP-IDF se instalira izravno u Windows, a skripte kolegija pokreću se
  u PowerShell inačicama istog imena (`.ps1`).

Skripte kolegija pisane su za `bash` na Linuxu (izravno, u WSL2 ili u
kontejneru), a za put C postoje njihove PowerShell inačice; Git Bash i MSYS2 na
Windowsu nisu podržani.

| Sustav | Preporučeni put | Pločica preko USB-a |
|---|---|---|
| **Windows 10/11** | **WSL2** (Ubuntu), a u njemu upute za Linux: put A (Docker Desktop) ili put B | u WSL2, put B + `usbipd` |
| **macOS** | **put A** (Docker Desktop) | `esptool` s macOS-a ili put B |
| **Linux** | **put A** ili put B | put A uz `URS_PORT` ili put B |
| **Windows bez virtualizacije** | **put C** (LV1–LV6) | izravno, `COM<n>` |

Na Windowsu se put A može izvesti i izravno iz PowerShella
(`docker-shell.ps1`), bez Ubuntua u WSL2: dovoljan je za izgradnju i emulator u LV1–LV5
i za LV7, ali ne i za LV6; pločica se tada programira alatom `esptool` iz
Windowsa. Što koji put pokriva:

| | A: Docker (Linux, WSL2, macOS) | A: Docker (Windows, PowerShell) | B: bez Dockera (Linux, WSL2, macOS) | C: Windows bez virtualizacije |
|---|---|---|---|---|
| Izgradnja i emulator, LV1–LV5 | da | da | da | da |
| LV6 (MQTT preko TLS-a) | da | ne — upotrijebite WSL2 | da (posrednik ipak radi u Dockeru) | da, uz Mosquitto za Windows |
| LV7 (Buildroot, ugradbeni Linux) | da | da | Linux i WSL2 uz dodatne pakete; macOS preko Dockera | ne |
| Programiranje pločice | na Linuxu; na macOS-u `esptool` s računala | `esptool` iz Windowsa | da | da |

## Korak 0: priprema računala i preuzimanje repozitorija

**Windows** ima tri mogućnosti (vidi „Koji put odabrati”):

- **WSL2** (preporučeno; putovi A i B kao na Linuxu): u PowerShellu pokrenutom
  kao administrator izvedite `wsl --install`, ponovno pokrenite računalo,
  otvorite aplikaciju **Ubuntu** i zadajte korisničko ime i lozinku za Linux.
  Sve daljnje naredbe pišu se u terminalu Ubuntu, a repozitorij klonirajte u
  Linuxov datotečni sustav (`~/`), nikako u `/mnt/c/...`. Ako virtualizacija
  nije uključena, uključite je u BIOS-u/UEFI-ju (*Intel VT-x*, *AMD-V*); ako je
  procesor ne podržava, odaberite put C.
- **Docker iz PowerShella** (put A bez Ubuntua u WSL2): Git za Windows
  (`winget install Git.Git`) i Docker Desktop (put A, niže); repozitorij se
  klonira u PowerShellu (niže).
- **Put C** (bez virtualizacije): instalaciju i preuzimanje repozitorija opisuje
  odjeljak *Put C*; od ovog odjeljka za njega vrijede samo tablica preduvjeta i
  uređivač koda.

Prije početka instalirajte (Git i VS Code mogu se instalirati i bez
administratorskih prava):

| Mogućnost | Instalirajte prije početka | Administrator | Prostor |
|---|---|---|---|
| WSL2 (putovi A i B) | WSL2 s Ubuntuom (`wsl --install`); za put A Docker Desktop; za pločicu `usbipd` (`winget install usbipd` ili [GitHub](https://github.com/dorssel/usbipd-win/releases)) | za `wsl --install` i `usbipd` | Ubuntu nekoliko GB; put A oko 8 GB, put B oko 7 GB |
| Docker iz PowerShella (put A) | [Git for Windows](https://git-scm.com/download/win) (ili `winget install Git.Git`) i [Docker Desktop](https://www.docker.com/products/docker-desktop/) (Windows 10 22H2 ili Windows 11 23H2, 8 GB RAM-a, uključena virtualizacija) | jednom, kad Docker Desktop prvi put uključuje WSL2 | oko 8 GB uz Docker Desktop |
| Put C | Git for Windows, ESP-IDF Tools Installer v5.5, emulator QEMU i, za LV6, Mosquitto za Windows (sve u odjeljku *Put C*) | za Mosquitto (instalira se kao servis) | oko 8 GB, uz 1,6 GB preuzimanja i oko 0,3 GB po vježbi |

**Uređivač koda:** preporučen je [VS Code](https://code.visualstudio.com), a
odgovara i bilo koji drugi. Kako se repozitorij otvara i koristi u VS Codeu,
opisuje odjeljak *Rad u VS Codeu*.

**macOS.** Ako naredba `git` ne postoji, instalirajte je s
`xcode-select --install`. **Linux** (Debian/Ubuntu): `sudo apt-get install -y git`
(Fedora: `sudo dnf install git`, Arch: `sudo pacman -S git`; pakete za put B na
tim distribucijama navodi [dokumentacija ESP-IDF-a](https://docs.espressif.com/projects/esp-idf/en/v5.5/esp32c3/get-started/linux-macos-setup.html),
a najjednostavniji je ondje put A).

Repozitorij kolegija preuzmite u terminalu (Linux, macOS) odnosno terminalu
Ubuntu (WSL2):

```sh
cd ~
git clone https://github.com/vvyroubal/VUKA-ugradbeni-sustavi.git
cd VUKA-ugradbeni-sustavi
```

**Samo za Docker iz PowerShella (bez Ubuntua u WSL2):** u običnom PowerShellu najprije
instalirajte Git naredbom `winget install Git.Git`, zatim otvorite **novi**
PowerShell i izvedite ista tri retka.

Datoteka `.gitattributes` čuva Linuxove završetke redaka u skriptama i kad
repozitorij klonira Git za Windows. Sve naredbe u nastavku pišu se iz korijena
repozitorija (`~/VUKA-ugradbeni-sustavi`), osim ako piše drukčije.

## Put A: s Dockerom

1. Instalirajte Docker:
   - **Windows:** [Docker Desktop](https://www.docker.com/products/docker-desktop/)
     s WSL2 pozadinom (zadano). Radite li u WSL2, u Docker Desktopu uključite
     *Settings → Resources → WSL integration* za **Ubuntu** (za Docker iz
     PowerShella taj korak ne treba).
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

Što skripta `docker-shell` radi: pokreće kontejner imena `vuka-ugradbeni-sustavi`,
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
cd ~/VUKA-ugradbeni-sustavi
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
cd ~/VUKA-ugradbeni-sustavi
bash scripts/verify.sh
```

`coreutils` donosi naredbu `gtimeout`, koju trebaju skripte za emulator.

## Put C: Windows bez virtualizacije (bez WSL2 i Dockera)

Za računala na kojima WSL2 i Docker ne rade jer procesor ne podržava
virtualizaciju. Najprije provjerite nije li ona samo isključena u BIOS-u/UEFI-ju
(*Intel VT-x* ili *AMD-V*) — ako se može uključiti, slijedite upute za WSL2.
Emulatoru QEMU virtualizacija ne treba (čip oponaša programski), pa ovaj put
pokriva LV1–LV6 i pločicu; LV7 (Buildroot) treba Linux, pa je izvedite na
računalu s WSL2, Dockerom ili Linuxom (dogovorite se s nastavnikom). Potreban je
64-bitni Windows 10 ili 11.

1. **Git for Windows** (<https://git-scm.com/download/win> ili
   `winget install Git.Git`); donosi i `openssl`, koji za certifikate u LV6
   koristi skripta `gen_certs.ps1`.
2. Službeni [ESP-IDF Tools Installer](https://dl.espressif.com/dl/esp-idf/),
   inačica **v5.5** (na stranici: *ESP-IDF v5.5.x Offline Installer*). Instalacijski direktorij: zadani ili kratka putanja bez
   razmaka, zagrada i dijakritičkih znakova (kraća od 90 znakova). Sve naredbe
   `idf.py` pišu se u prečacu **ESP-IDF 5.5 PowerShell**.
3. U tom prečacu instalirajte emulator, pa prečac zatvorite i otvorite ponovno:
   ```powershell
   python $env:IDF_PATH\tools\idf_tools.py install qemu-riscv32
   ```
4. Klonirajte repozitorij u putanju bez razmaka i dijakritičkih znakova
   (korisnička mapa poput `C:\Users\Ivan Čović` ne odgovara ESP-IDF-u):
   ```powershell
   mkdir C:\vuka
   cd C:\vuka
   git clone https://github.com/vvyroubal/VUKA-ugradbeni-sustavi.git
   cd VUKA-ugradbeni-sustavi
   ```
5. Za LV6: [Mosquitto za Windows](https://mosquitto.org/download/) (64-bitni;
   traži administratora jer se instalira kao servis). Instalacijski program
   nije potpisan, pa na upozorenje *Windows protected your PC* odaberite
   *More info* → *Run anyway*.

**Skripte kolegija na Windowsu.** Za svaku skriptu kolegija postoji PowerShell
inačica **istog imena s nastavkom `.ps1`**, s istim argumentima i istim
ponašanjem (emulator se sam zaustavlja nakon zadanog vremena, Ctrl-A pa X ga
zaustavlja ranije, a ispis se sprema i u `build/qemu.log`). Pokreću se u
prečacu *ESP-IDF 5.5 PowerShell*, koji dopušta pokretanje skripti. Prečac se
otvara u direktoriju ESP-IDF-a, pa pri svakom otvaranju najprije prijeđite u
repozitorij.

**Provjera**, PowerShell inačicom skripte `verify.sh`:
```powershell
cd C:\vuka\VUKA-ugradbeni-sustavi
./scripts/verify.ps1
```
Bitan je zadnji redak, `verify.ps1: PASS -- toolchain, build and QEMU all work`.
Prvo pokretanje LV1 zatim izgleda ovako:
```powershell
cd labs\lab1_toolchain\starter
idf.py set-target esp32c3
idf.py build
../../common/run_qemu.ps1 build
```

**Naredbe iz priručnika vježbi** pisane su za bash, kao na putovima A i B. Na
putu C pokrećete PowerShell inačice istog imena: izostavite `bash`, nastavak
`.sh` zamijenite s `.ps1` i naredbu pokrenite iz istog direktorija.

*Putovi A i B — Linux, macOS, WSL2 i kontejner (bash):*

| Naredba | Što radi |
|---|---|
| `bash ../../common/run_qemu.sh build` | pokreće izgradnju u emulatoru |
| `bash ../../common/run_qemu_net.sh build` | isto, s mrežnom karticom (LV6) |
| `bash ../../common/run_qemu_gdb.sh build` | emulator čeka GDB na portu 1234 (LV2) |
| `bash ../broker/gen_certs.sh` | izrađuje TLS certifikate posrednika (LV6) |
| `bash ../broker/run_broker.sh` | pokreće MQTT posrednik (LV6) |
| `bash scripts/verify.sh` | provjerava alatni lanac, izgradnju i QEMU |

*Put C — Windows, prečac ESP-IDF 5.5 PowerShell:*

| Naredba | Što radi |
|---|---|
| `../../common/run_qemu.ps1 build` | pokreće izgradnju u emulatoru |
| `../../common/run_qemu_net.ps1 build` | isto, s mrežnom karticom (LV6) |
| `../../common/run_qemu_gdb.ps1 build` | emulator čeka GDB (LV2); GDB pokrenite u drugom prozoru prečaca, naredbom koju skripta ispiše |
| `../broker/gen_certs.ps1` | izrađuje TLS certifikate posrednika (LV6) |
| `../broker/run_broker.ps1` | pokreće posrednik Mosquitto za Windows (LV6), u zasebnom prozoru prečaca |
| `./scripts/verify.ps1` | provjerava alatni lanac, izgradnju i QEMU |

Naredbu koju priručnik prelama znakom `\` na putu C upišite u jednom retku.

Za brzo isprobavanje radi i ESP-IDF-ova naredba `idf.py qemu monitor` (izgradi,
pokrene i prikaže ispis; izlaz Ctrl-]).

**LV6:** posrednik (`run_broker.ps1`) emulator vidi na `10.0.2.2:8883`, alati na
računalu na `127.0.0.1:8883`. Ako Windows upita, dopustite pristup u privatnoj
mreži. `mosquitto_sub` i `mosquitto_pub` su u `C:\Program Files\mosquitto`, pa se
naredba iz priručnika LV6, iz `labs\lab6_mqtt\starter`, piše ovako:
```powershell
& "C:\Program Files\mosquitto\mosquitto_sub.exe" -h 127.0.0.1 -p 8883 --cafile ../broker/certs/ca.crt -t "vuka/sensors/#" -v
```

**Pločica:** `idf.py -p COM<n> flash monitor`, port iz Upravitelja uređaja.

## Provjera: `verify.sh`

Ovaj i sljedeći odjeljak vrijede za putove A i B; na putu C provjeru i rad na
vježbama opisuje odjeljak *Put C*.

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

Put A: najprije `./scripts/docker-shell.sh` iz korijena repozitorija (u
PowerShellu `docker-shell.ps1`) — sve sljedeće naredbe izvode se u toj ljusci
(pod `/work`). Put B: `. ~/esp-idf/export.sh`. Primjer za LV1:

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

Kad isti projekt prebacujete s izgradnje na putu A na izgradnju na putu B ili
obrnuto, najprije izvedite `idf.py fullclean`: direktorij `build/` pamti
putanju za koju je konfiguriran (u kontejneru je to `/work/...`).

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
korijena repozitorija; zaustavlja se s Ctrl-C. Ako kontejner `vuka-ugradbeni-sustavi`
radi, posrednik se pridružuje njegovoj mreži (QEMU ga u kontejneru vidi na
`10.0.2.2:8883`, alati na računalu na `127.0.0.1:8883`); inače objavljuje
port 8883 na računalu (put bez Dockera). S Dockerom iz PowerShella LV6 nije
moguć (skripte trebaju `bash`): radite je u WSL2 ili na putu C.

**LV7.** Buildroot je najjednostavnije graditi u kontejneru tečaja, u
**terminalu računala** iz korijena repozitorija.

Linux i WSL2 — istom skriptom kao ostale vježbe; kontejner radi pod vašim
korisnikom, pa datoteke izgradnje pripadaju vama (izravan `docker run` bez
`--user` gradio bi kao root i ostavio datoteke koje ne možete obrisati bez
`sudo`):
```sh
./scripts/docker-shell.sh make -C labs/lab7_linux_ota -f Makefile.buildroot all
```

Na macOS-u i Windowsu naredba sama pokreće kontejner (ne izvodi se u ljusci
`docker-shell`).

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

## Rad u VS Codeu

Repozitorij sadrži postavke za VS Code (`.vscode/`): preporučena proširenja,
zadatke i postavke za dovršavanje koda. Otvorite **korijen repozitorija**
(*File → Open Folder*; u WSL2 `code .` iz terminala Ubuntu, uz proširenje
*WSL*; na putu C `C:\vuka\VUKA-ugradbeni-sustavi`). Na pitanje vjerujete li autorima mape
odgovorite *Yes, I trust the authors* — u ograničenom načinu (*Restricted
Mode*) zadaci ne rade. VS Code zatim ponudi proširenja *C/C++*, *WSL* (treba
samo na Windowsu) i *Dev Containers* (za put A); ponudu paketa *C/C++ Extension
Pack* možete odbiti. Espressifovo proširenje *ESP-IDF* nije potrebno i ne
instalirajte ga — ono postavlja vlastiti, drukčiji ESP-IDF.

**Zadaci** (*Terminal → Run Task…*, Ctrl-Shift-B za izgradnju i pokretanje),
nakon kojih se bira vježba:

| Zadatak | Što radi |
|---|---|
| Izgradnja i QEMU (Ctrl-Shift-B) | `idf.py build`, zatim `run_qemu.sh` (LV6: `run_qemu_net.sh`) |
| Izgradnja | `idf.py build` |
| QEMU | pokreće zadnju izgradnju u emulatoru |
| menuconfig | `idf.py menuconfig` |
| idf.py reconfigure | nakon novog `Kconfig.projbuild` (LV1, LV4) |
| idf.py fullclean | kad isti projekt prelazi s Dockera na izgradnju bez njega ili obrnuto |

Zadaci sami nalaze okruženje (skripta `scripts/vscode-task.sh`, na Windowsu
`.ps1`): aktivan ESP-IDF, `~/esp-idf` (put B), instalaciju na Windowsu (put C),
a ako ESP-IDF-a nema, kontejner tečaja (put A). Na putu A zadatak se spaja na
kontejner koji već radi, pa zadatke pokrećite jedan po jedan.

**Terminal:** na putu A `./scripts/docker-shell.sh`, na putu B
`. ~/esp-idf/export.sh`; na putu C profil **ESP-IDF 5.5 PowerShell** (strelica
uz `+` u ploči terminala).

**Dovršavanje koda (IntelliSense)** čita `build/compile_commands.json` vježbe, pa
radi nakon prve izgradnje (do tada *Problems* za neizgrađene vježbe javlja
*Cannot find … compile_commands.json*). Na putu A putanje su iz kontejnera (`/work/...`):
pokrenite `./scripts/docker-shell.sh`, zatim F1 → *Dev Containers: Attach to
Running Container…* → `vuka-ugradbeni-sustavi` i otvorite mapu `/work`.

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
| Windows, put C | `COM<n>` | Upravitelj uređaja → *USB Serial Device (COMn)* |
| Windows, Docker iz PowerShella | `COM<n>` | `esptool` iz Windowsa (niže) |
| Docker Desktop (macOS, Windows) | — | USB uređaji ne prosljeđuju se u kontejner |

**Windows, Docker iz PowerShella.** Kontejner ne vidi USB, pa se program
izgrađen u kontejneru upisuje alatom `esptool` iz Windowsa. Instalirajte
ESP-IDF Tools Installer (put C, korak 2) i u prečacu *ESP-IDF 5.5 PowerShell*,
u direktoriju izgradnje projekta (npr. `labs\lab1_toolchain\starter\build`),
izvedite (`COM5` zamijenite svojim portom; navodnici oko `@flash_args` su
nužni, jer PowerShell znak `@` inače tumači sam):
```powershell
python -m esptool --chip esp32c3 -p COM5 write_flash "@flash_args"
python -m serial.tools.miniterm COM5 115200      # izlaz: Ctrl-]
```

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
- **Windows, put C: `running scripts is disabled on this system`** — skripta
  `.ps1` pokrenuta je u običnom PowerShellu; pokrenite je u prečacu
  *ESP-IDF 5.5 PowerShell*, koji pokretanje skripti dopušta.
- **Windows, put C: `Python was not found; run without arguments to install
  from the Microsoft Store` ili `idf.py` nije prepoznat** — naredba je upisana u
  običnom PowerShellu; otvorite prečac *ESP-IDF 5.5 PowerShell*. Ako nema ni
  `qemu-system-riscv32`, prečac nakon instalacije QEMU-a nije ponovno otvoren.
- **WSL2: nema `/dev/ttyACM0`** — `usbipd attach` nije pokrenut ili je
  pločica preko drugog kabela/priključka; u PowerShellu provjerite
  `usbipd list` (stanje *Attached*).

## Napomena o ključu u LV7

`labs/lab7_linux_ota/ota/keys/demo_signing_key.INSECURE.pem` namjerno je
objavljen privatni ključ za potpisivanje. Služi isključivo vježbi; nikad ga ne
upotrebljavajte ni za što drugo. Zašto je to u stvarnom proizvodu nedopustivo,
tema je vježbe LV7 i poglavlja o sigurnosti.
