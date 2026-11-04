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
| `scripts/` | `docker-shell.sh` / `docker-shell.ps1` (ljuska u kontejneru), `verify.sh` / `verify.ps1` (provjera instalacije), `vscode-task.*`, `gdb.sh` / `gdb.cmd` i `idf-shell.ps1` (za VS Code) |
| `.vscode/` | postavke, zadaci, konfiguracije debuggera (F5 za LV1–LV6) i preporučena proširenja za VS Code |
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
| **Linux** | **put A** ili put B | put A uz `URS_PORT` ili put B |
| **macOS** | **put A** (Docker Desktop) | `esptool` s macOS-a ili put B |
| **Windows 10/11** | **WSL2** (Ubuntu), a u njemu upute za Linux: put A (Docker Desktop) ili put B | u WSL2, put B + `usbipd` |
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

**Redoslijed:** na svakom putu najprije Korak 0, zatim odjeljak svojega puta
(A, B ili C), pa *Provjera* i *Rad na vježbi*; za put C provjera i prvo
pokretanje opisani su u odjeljku *Put C*. *Rad u VS Codeu* i *Programiranje
pločice* dolaze na red kad okruženje radi.

## Korak 0: priprema računala i preuzimanje repozitorija

Na svakom sustavu trebate dvoje: **uređivač koda**, jer vježbe traže izmjene
izvornog koda (preporučen je [VS Code](https://code.visualstudio.com), a
odgovara i bilo koji drugi; rad u njemu opisuje odjeljak *Rad u VS Codeu*), i
**Git**, kojim preuzimate repozitorij. Što još treba, ovisi o sustavu i putu:

| Sustav | Instalirajte prije početka | Administrator | Prostor |
|---|---|---|---|
| Linux | Git; za put A Docker Engine (*Put A*) | `sudo` za pakete | put A oko 8 GB, put B oko 7 GB |
| macOS | Git; za put A Docker Desktop (*Put A*) | pri instalaciji Docker Desktopa | put A oko 8 GB, put B oko 7 GB |
| Windows, WSL2 (putovi A i B) | WSL2 s Ubuntuom (`wsl --install`); za put A Docker Desktop; za pločicu `usbipd` (`winget install usbipd` ili [GitHub](https://github.com/dorssel/usbipd-win/releases)) | za `wsl --install` i `usbipd` | Ubuntu nekoliko GB; put A oko 8 GB, put B oko 7 GB |
| Windows, Docker iz PowerShella (put A) | [Git for Windows](https://git-scm.com/download/win) (ili `winget install Git.Git`) i [Docker Desktop](https://www.docker.com/products/docker-desktop/) (Windows 10 22H2 ili Windows 11 23H2, 8 GB RAM-a, uključena virtualizacija) | jednom, kad Docker Desktop prvi put uključuje WSL2 | oko 8 GB uz Docker Desktop |
| Windows, put C | Git for Windows, ESP-IDF Tools Installer v5.5, emulator QEMU i, za LV6, Mosquitto za Windows (sve u odjeljku *Put C*) | za Mosquitto (instalira se kao servis) | oko 8 GB, uz 1,6 GB preuzimanja i oko 0,3 GB po vježbi |

**Linux.** Git na Debianu i Ubuntuu: `sudo apt-get install -y git` (Fedora:
`sudo dnf install git`, Arch: `sudo pacman -S git`). Pakete za put B na
distribucijama izvan Debiana i Ubuntua navodi
[dokumentacija ESP-IDF-a](https://docs.espressif.com/projects/esp-idf/en/v5.5/esp32c3/get-started/linux-macos-setup.html);
ondje je najjednostavniji put A.

**macOS.** U Terminalu upišite `git --version`; ako naredba ne postoji,
instalirajte je s `xcode-select --install`.

**Windows** nudi tri mogućnosti (vidi *Koji put odabrati*):

- **WSL2** (preporučeno; putovi A i B kao na Linuxu): u PowerShellu pokrenutom
  kao administrator izvedite `wsl --install`, ponovno pokrenite računalo,
  otvorite aplikaciju **Ubuntu** i zadajte korisničko ime i lozinku za Linux.
  Sve daljnje naredbe pišu se u terminalu Ubuntu, a repozitorij klonirajte u
  Linuxov datotečni sustav (`~/`), nikako u `/mnt/c/...`. Ako virtualizacija
  nije uključena, uključite je u BIOS-u/UEFI-ju (*Intel VT-x*, *AMD-V*); ako je
  procesor ne podržava, odaberite put C. Git je u Ubuntuu već instaliran.
- **Docker iz PowerShella** (put A bez Ubuntua u WSL2): Git za Windows i Docker
  Desktop iz tablice; repozitorij se klonira u PowerShellu (niže).
- **Put C** (bez virtualizacije): instalaciju i preuzimanje repozitorija opisuje
  odjeljak *Put C*; od ovog odjeljka za njega vrijede samo tablica i uređivač
  koda.

Git i VS Code na Windowsu mogu se instalirati i bez administratorskih prava;
`winget` pri prvom pokretanju traži da prihvaćanje uvjeta potvrdite slovom `Y`.

**Preuzimanje repozitorija.** U terminalu (Linux, macOS) odnosno terminalu
Ubuntu (WSL2):

```sh
cd ~
git clone https://github.com/vvyroubal/VUKA-ugradbeni-sustavi.git
cd VUKA-ugradbeni-sustavi
```

Za Docker iz PowerShella izvedite ista tri retka u **novom** PowerShellu,
otvorenom nakon instalacije Gita. Datoteka `.gitattributes` čuva Linuxove
završetke redaka u skriptama i kad repozitorij klonira Git za Windows. Sve
naredbe u nastavku pišu se iz korijena repozitorija
(`~/VUKA-ugradbeni-sustavi`), osim ako piše drukčije.

## Put A: s Dockerom

1. Instalirajte Docker:
   - **Linux:** Docker Engine prema [službenim uputama](https://docs.docker.com/engine/install/),
     a zatim dodajte korisnika u grupu `docker` i odjavite se pa ponovno
     prijavite (ili u tekućem terminalu izvedite `newgrp docker`):
     ```sh
     sudo usermod -aG docker $USER
     ```
   - **macOS:** [Docker Desktop](https://www.docker.com/products/docker-desktop/).
   - **Windows:** [Docker Desktop](https://www.docker.com/products/docker-desktop/)
     s WSL2 pozadinom (zadano); instalacijski program na kraju traži odjavu
     ili ponovno pokretanje računala — učinite to. Radite li u WSL2, u Docker
     Desktopu uključite
     *Settings → Resources → WSL integration* za **Ubuntu** (za Docker iz
     PowerShella taj korak ne treba).

   Pri prvom pokretanju Docker Desktop traži prihvaćanje uvjeta korištenja i
   nudi prijavu na Docker račun; prijava nije potrebna (*Continue without
   signing in*). Na Windowsu i macOS-u **Docker Desktop mora biti pokrenut**
   dok radite.
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
   razmaka, zagrada i dijakritičkih znakova (kraća od 90 znakova); na ostalim
   stranicama ostavite zadane postavke. Sve naredbe
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

**Terminal s ESP-IDF-om.** Naredbe za izgradnju i emulator pišu se u
terminalu u kojem je ESP-IDF aktivan. Kako se takav terminal otvara, ovisi o
putu:

- **put A:** iz korijena repozitorija pokrenite `./scripts/docker-shell.sh`
  (na Windowsu iz PowerShella `docker-shell.ps1`, vidi *Put A*). Otvara se
  ljuska u kontejneru, u kojoj je repozitorij pod `/work`; sve sljedeće
  naredbe pišu se u njoj.
- **put B:** u svakom novom terminalu najprije izvedite
  `. ~/esp-idf/export.sh`.
- **put C:** otvorite prečac **ESP-IDF 5.5 PowerShell**. Skripte kolegija na
  tom putu imaju istoimenu inačicu `.ps1` (tablica u odjeljku *Put C*).

**Izgradnja i emulator.** Na primjeru LV1, na putovima A i B (za put C vidi
odjeljak *Put C*):

```sh
cd labs/lab1_toolchain/starter
idf.py set-target esp32c3                  # samo prvi put
idf.py build
bash ../../common/run_qemu.sh build        # emulator, 30 s
```

`idf.py set-target esp32c3` odabire ciljni čip i izvodi se samo prvi put, jer
se odabir pamti u datoteci `sdkconfig`. Pokrenete li ga ponovno, `sdkconfig`
nastaje iznova i gube se promjene napravljene u `menuconfig`-u. `idf.py build`
gradi program u direktorij `build/`.

`run_qemu.sh` iz zadnje izgradnje sastavlja sliku flash memorije
(`build/flash.bin`) i pokreće je u emulatoru, pa se uvijek izvodi najnoviji
program. Emulator se sam zaustavlja nakon 30 s; drugim argumentom zadaje se
drukčije vrijeme (npr. `run_qemu.sh build 60`). Završni redak
`run_qemu.sh: timeout (30s) reached after output was produced — treating as success`
znači da je sve u redu. Želite li ga zaustaviti prije, pritisnite **Ctrl-A, a
zatim X**. Cijeli ispis sprema se i u `build/qemu.log`.

Gradite li isti projekt naizmjence na putu A i na putu B, prije promjene puta
izvedite `idf.py fullclean`: direktorij `build/` pamti putanju projekta, a ona
je u kontejneru drukčija (`/work/...`).

| Vježba | Direktorij | Napomena |
|---|---|---|
| LV1 | `labs/lab1_toolchain/` | alatni lanac, prva izgradnja |
| LV2 | `labs/lab2_gpio_irq/` | registri, prekidi, GPTimer; GDB u drugom terminalu |
| LV3 | `labs/lab3_bme280/` | I2C i BME280; snimke signala u `captures/` |
| LV4 | `labs/lab4_adc_pwm/` | ADC i PWM |
| LV5 | `labs/lab5_rtos/` | FreeRTOS |
| LV6 | `labs/lab6_mqtt/` | MQTT preko TLS-a; emulator s mrežom `run_qemu_net.sh` |
| LV7 | `labs/lab7_linux_ota/` | Buildroot i OTA; prva izgradnja traje 30–60 min |

Što u vježbi treba dovršiti i kako se to provjerava, opisuje priručnik vježbe.

**GDB iz terminala (LV2).** U VS Codeu debugger se spaja tipkom F5 (odjeljak
*Debugger: GDB spojen na QEMU*). Bez VS Codea trebaju vam dva terminala, oba
u direktoriju `labs/lab2_gpio_irq/starter`:

1. **Terminal 1 pokreće QEMU koji čeka debugger:**
   `bash ../../common/run_qemu_gdb.sh build` (put C:
   `../../common/run_qemu_gdb.ps1 build`). Skripta ispiše naredbu za spajanje
   i čeka.
2. **Terminal 2 pokreće GDB.** Otvorite drugi terminal s aktivnim ESP-IDF-om:
   na putu A ponovno pokrenite `./scripts/docker-shell.sh`, koji se spaja na
   isti kontejner; na putu B u novom terminalu izvedite
   `. ~/esp-idf/export.sh`; na putu C otvorite još jedan prozor prečaca
   **ESP-IDF 5.5 PowerShell**. Zatim prijeđite u `labs/lab2_gpio_irq/starter`
   i upišite naredbu koju je ispisao terminal 1:
   `riscv32-esp-elf-gdb build/lab2.elf -ex "target remote :1234"`.

**LV3 i LV4 na pločici.** U emulatoru LV3 koristi simulirani senzor, a LV4
simulirani ADC. Inačica za pločicu gradi se u **zasebnom** direktoriju
`build-hw`, kako bi izgradnja za QEMU u `build/` ostala netaknuta. Iz
direktorija `starter` vježbe:

```sh
idf.py -B build-hw -D SDKCONFIG=build-hw/sdkconfig -D SDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.hw" build
idf.py -B build-hw -D SDKCONFIG=build-hw/sdkconfig -p <port> flash monitor
```

Zaseban direktorij potreban je zato što se `sdkconfig.defaults` i
`sdkconfig.hw` primjenjuju samo kad `sdkconfig` nastaje, a `idf.py` popis tih
datoteka pamti u direktoriju izgradnje. Izgradite li inačicu za pločicu u
`build/`, program i dalje koristi simulator, bez ikakve poruke o tome, a ni
povratak na QEMU više ne radi kako treba.

Uz Docker na macOS-u i Windowsu kontejner ne vidi pločicu: prvu naredbu
izvedite u kontejneru, a program upišite alatom `esptool` s računala, kako
opisuje odjeljak *Programiranje pločice*, ali iz direktorija `build-hw`
(npr. `labs/lab3_bme280/starter/build-hw`), ne iz `build`.

**LV6.** Redoslijed je ovaj:

1. **Certifikati**, iz direktorija `labs/lab6_mqtt/starter`, na računalu ili
   u ljusci kontejnera: `bash ../broker/gen_certs.sh`.
2. **Posrednik MQTT**, u **terminalu računala** (Linux, macOS, WSL2), ne u
   ljusci kontejnera, jer skripta sama poziva `docker`. Iz korijena
   repozitorija: `bash labs/lab6_mqtt/broker/run_broker.sh`. Posrednik radi
   dok ga ne zaustavite s Ctrl-C, pa za ostale korake otvorite drugi terminal.
3. **Program**, u terminalu s ESP-IDF-om, iz `labs/lab6_mqtt/starter`:
   `idf.py build`, zatim `bash ../../common/run_qemu_net.sh build`.

Posrednik uvijek radi u Dockeru (službena slika `eclipse-mosquitto`), pa i
put B za LV6 treba Docker. Ako kontejner kolegija (`vuka-ugradbeni-sustavi`)
radi, posrednik se priključuje njegovoj mreži: QEMU u kontejneru vidi ga na
`10.0.2.2:8883`, a alati na računalu na `127.0.0.1:8883`. Bez kontejnera
kolegija (put B) posrednik prosljeđuje port 8883 izravno na računalo. S
Dockerom iz PowerShella LV6 nije moguća, jer skripte trebaju `bash`: radite je
u WSL2 ili na putu C.

**LV7.** Buildroot je najjednostavnije izgraditi u kontejneru kolegija.
Naredbe niže pišu se u **terminalu računala**, iz korijena repozitorija, a ne
u ljusci `docker-shell`, jer same pokreću kontejner. U VS Codeu isto radi
zadatak *LV7: izgradnja Buildroota* (*Terminal → Run Task…*), koji sam bira
način: u prozoru priključenom na kontejner gradi u njemu (na macOS-u i
Windowsu na volumenu `lab7_br`, koji `docker-shell` montira na `/br`), a u
običnom prozoru pokreće kontejner kolegija. Na Linuxu bez slike kolegija gradi
izravno, uz pakete navedene niže. Na putu C LV7 nije moguća.

**Linux i WSL2** — istom skriptom kao za ostale vježbe. Kontejner tada radi
pod vašim korisnikom, pa datoteke izgradnje pripadaju vama (izravan
`docker run` bez `--user` gradio bi kao root, a takve datoteke ne možete
obrisati bez `sudo`):
```sh
./scripts/docker-shell.sh make -C labs/lab7_linux_ota -f Makefile.buildroot all
```

**macOS** (Docker Desktop) — izgradnja na imenovanom volumenu `lab7_br`:
```sh
docker run --rm -v "$PWD:/work" -w /work -v lab7_br:/br embedded-course bash -lc 'make -C labs/lab7_linux_ota -f Makefile.buildroot all BR_DIR=/br'
```

**Windows** (PowerShell, Docker Desktop) — ista naredba, u jednom retku:
```powershell
docker run --rm -v "${PWD}:/work" -w /work -v lab7_br:/br embedded-course bash -lc 'make -C labs/lab7_linux_ota -f Makefile.buildroot all BR_DIR=/br'
```

**Bez Dockera** (samo Linux i WSL2) potrebni su još paketi
`sudo apt-get install -y make gcc g++ patch cpio unzip rsync bc file wget qemu-system-misc`,
a izgradnja se zatim pokreće izravno:
`make -C labs/lab7_linux_ota -f Makefile.buildroot all`.

## Rad u VS Codeu

Repozitorij sadrži postavke za VS Code (`.vscode/`): preporučena proširenja,
zadatke za izgradnju i emulator, konfiguracije debuggera i postavke za
dovršavanje koda. VS Code mora raditi **ondje gdje je ESP-IDF**, pa se
repozitorij otvara različito prema putu:

- **Put A (Docker, svi sustavi).** Alati su samo u kontejneru, pa se VS Code
  na njega priključuje. U terminalu računala, u korijenu repozitorija,
  pokrenite `./scripts/docker-shell.sh` (u PowerShellu `docker-shell.ps1`) i
  taj terminal ne zatvarajte: kad ljuska završi, kontejner se gasi. Zatim u VS
  Codeu odaberite F1 → *Dev Containers: Attach to Running Container…* →
  `vuka-ugradbeni-sustavi`, a u novom prozoru *File → Open Folder* → `/work`.
  Dolje lijevo u prozoru piše *Container …*. Isto ponavljate na početku
  svakog rada.
- **Put B, Linux i macOS.** *File → Open Folder* → korijen repozitorija.
- **Put B, WSL2.** U terminalu Ubuntu, u korijenu repozitorija, upišite
  `code .`. VS Code je instaliran na Windowsu, a proširenje *WSL* spaja ga s
  Ubuntuom; dolje lijevo piše *WSL: Ubuntu*.
- **Put C.** *File → Open Folder* → `C:\vuka\VUKA-ugradbeni-sustavi`.

Na pitanje vjerujete li autorima mape odgovorite *Yes, I trust the authors*: u
ograničenom načinu (*Restricted Mode*) zadaci ne rade. VS Code zatim ponudi
preporučena proširenja. *C/C++* treba na svim putovima, *WSL* samo za WSL2, a
*Dev Containers* samo za put A. Ponudu paketa *C/C++ Extension Pack* možete
odbiti. Na putu A i u WSL2 proširenje *C/C++* mora biti instalirano **u
kontejneru** odnosno **u WSL-u**: u pregledu *Extensions* (Ctrl-Shift-X) za
*C/C++* kliknite *Install in Container…* odnosno *Install in WSL: Ubuntu*.
Espressifovo proširenje *ESP-IDF* nije potrebno i ne instalirajte ga: ono
postavlja vlastiti, drukčiji ESP-IDF.

**Prečaci na macOS-u.** Prečaci VS Codea u ovim uputama navedeni su za Windows
i Linux. Na macOS-u umjesto **Ctrl** pritišćete **Cmd** (Cmd-Shift-B,
Cmd-Shift-D, Cmd-Shift-Y, Cmd-Shift-M, Cmd-Shift-X), a izbornik naredbi (F1)
otvara Cmd-Shift-P. Funkcijske tipke (F5, Shift-F5, F10, F11) na Macovoj
tipkovnici traže i tipku **fn** (npr. fn-F5), osim ako je uključeno *System
Settings → Keyboard → Keyboard Shortcuts… → Function Keys → Use F1, F2, etc.
keys as standard function keys*. U terminalu i emulatoru Ctrl ostaje Ctrl:
QEMU se i na macOS-u zaustavlja s Ctrl-A, zatim X, a monitor s Ctrl-].

**Pokretanje vježbe:**

1. Pritisnite **Ctrl-Shift-B** (macOS: Cmd-Shift-B; zadatak *Izgradnja i QEMU*) i s popisa
   odaberite vježbu, npr. `labs/lab1_toolchain/starter`.
2. U ploči *Terminal* otvara se kartica zadatka. U njoj teče izgradnja, a
   zatim ispis programa iz emulatora. Prva izgradnja vježbe traje nekoliko
   minuta, sljedeće nekoliko sekundi.
3. Emulator se sam zaustavlja nakon 30 sekundi. Poruka
   `run_qemu.sh: timeout (30s) reached after output was produced — treating as success`
   je normalna. Ranije ga zaustavljate tako da kliknete u karticu zadatka i
   pritisnete **Ctrl-A, a zatim X**.
4. Pogreške prevođenja ispisuju se u ploči *Problems* (Ctrl-Shift-M). Klik na
   pogrešku otvara datoteku na tom retku.

Svi zadaci su u izborniku *Terminal → Run Task…*. Zadaci bez oznake vježbe u
imenu zajednički su za LV1–LV6: vježbu birate s popisa koji se pojavi **nakon**
odabira zadatka. Debugger nije u tom izborniku, nego u pregledu *Run and Debug*
(odjeljak *Debugger*, niže).

| Zadatak | Što radi |
|---|---|
| Izgradnja i QEMU (Ctrl-Shift-B) | `idf.py build`, zatim `run_qemu.sh` (LV6: `run_qemu_net.sh`) |
| Izgradnja | `idf.py build` |
| QEMU | pokreće zadnju izgradnju u emulatoru |
| menuconfig | `idf.py menuconfig` |
| idf.py reconfigure | nakon novog `Kconfig.projbuild` (LV1, LV4) |
| idf.py fullclean | kad isti projekt prelazi s Dockera na izgradnju bez njega ili obrnuto |
| QEMU za GDB, čeka debugger | zadnja izgradnja u emulatoru koji čeka GDB (odjeljak *Debugger*) |
| LV7: izgradnja Buildroota | izgradnja LV7 (odjeljak *Rad na vježbi*, LV7) |
| LV1–LV6: izgradnja i QEMU za GDB | izgradnja vježbe, zatim emulator koji čeka GDB; pokreće ih i F5 (odjeljak *Debugger*) |

Zadaci sami nalaze okruženje (skripta `scripts/vscode-task.sh`, na Windowsu
`.ps1`). Redom pokušavaju aktivan ESP-IDF (i u kontejneru), `~/esp-idf`
(put B) i instalaciju na Windowsu (put C). Ako ESP-IDF-a nema (put A, a VS Code
nije priključen na kontejner), zadaci za izgradnju i emulator koriste kontejner
kolegija. Zadatak se tada spaja na kontejner koji već radi, pa zadatke pokrećite
jedan po jedan.

**Terminal:** na putu A `./scripts/docker-shell.sh`; u prozoru priključenom na
kontejner terminal je već u kontejneru, a ESP-IDF u njemu aktivirate s
`. $IDF_PATH/export.sh`. Na putu B `. ~/esp-idf/export.sh`; na putu C
profil **ESP-IDF 5.5 PowerShell** (strelica uz `+` u ploči terminala).

**Dovršavanje koda (IntelliSense)** čita `build/compile_commands.json` vježbe, pa
radi nakon prve izgradnje (do tada *Problems* za neizgrađene vježbe javlja
*Cannot find … compile_commands.json*). Na putu A putanje su iz kontejnera
(`/work/...`), pa dovršavanje radi samo u prozoru priključenom na kontejner.

### Debugger: GDB spojen na QEMU

Debugger čine dva programa koja rade istodobno, a spaja ih mrežni priključak
(port) **1234**:

- **QEMU u načinu za ispravljanje pogrešaka** (skripta `run_qemu_gdb.sh`, na
  putu C `run_qemu_gdb.ps1`) učita izgradnju vježbe, zaustavi procesor prije
  prve instrukcije i čeka da se na port 1234 spoji debugger. Dok čeka, program
  ne radi i ništa ne ispisuje. Zadnji redak skripte prije čekanja je
  `QEMU starting HALTED with gdbstub on :1234`.
- **GDB** (`riscv32-esp-elf-gdb`) iz datoteke `build/<projekt>.elf` čita imena
  funkcija i varijabli te retke izvornog koda, a zatim se spaja na
  `localhost:1234` i upravlja emulatorom. U VS Codeu GDB pokreće proširenje
  *C/C++* (preko `scripts/gdb.sh`, na Windowsu `scripts\gdb.cmd`), a vi ga
  upravljate klikovima.

Zato vrijede dva pravila: QEMU mora čekati **prije** nego što se GDB spaja, a
oba moraju raditi na istom računalu, na putu A u istom kontejneru.

**Priprema.** Repozitorij mora biti otvoren onako kako je za vaš put opisano
na početku odjeljka *Rad u VS Codeu*, s proširenjem *C/C++* instaliranim
ondje gdje VS Code radi: na putu A prozor priključen na kontejner (dolje
lijevo *Container …*), u WSL2 prozor WSL-a (dolje lijevo *WSL: Ubuntu*), na
putu B na Linuxu i macOS-u te na putu C obični prozor. Na putu A u prozoru
koji **nije** priključen na kontejner debugger ne može raditi (GDB bi radio
na računalu, a QEMU u kontejneru), pa zadaci za GDB tada javljaju
`with Docker, debug from VS Code attached to the course container`.

**Spajanje debuggera, korak po korak (bilo koja vježba):**

1. **Izgradite vježbu:** Ctrl-Shift-B ili zadatak *Izgradnja*.
2. **Postavite prijelomnu točku:** u kodu vježbe kliknite lijevo od broja
   retka na kojem program treba stati. Pojavi se crvena točka (prijelomna
   točka, *breakpoint*). Postavite je prije koraka 4, inače program prolazi
   bez zaustavljanja. Točka na retku bez koda (komentar, prazan redak,
   `(void)n;`) premješta se na sljedeći redak s kodom.
3. **Pokrenite QEMU koji čeka:** *Terminal → Run Task…* → **QEMU za GDB, čeka
   debugger** → odaberite vježbu. U kartici zadatka pojavi se
   `QEMU starting HALTED with gdbstub on :1234` i emulator sada čeka. Isto
   radi naredba u terminalu, iz direktorija vježbe:
   `bash ../../common/run_qemu_gdb.sh build` (putovi A i B) odnosno
   `../../common/run_qemu_gdb.ps1 build` (put C).
4. **Spojite debugger:** otvorite pregled *Run and Debug* (Ctrl-Shift-D; macOS: Cmd-Shift-D), u
   padajućem izborniku na vrhu odaberite **GDB: spoji se na QEMU koji čeka**,
   pritisnite **F5** (macOS: fn-F5) i s popisa odaberite program **iste** vježbe, npr.
   `labs/lab1_toolchain/starter/build/lab1.elf`. GDB se spoji na port 1234,
   emulator krene i program stane na prijelomnoj točki.

U LV6 emulator za GDB nema mrežnu karticu, pa se program ne može spojiti na
posrednik.

**Prečac: F5 za svaku vježbu (LV1–LV6).** Za svaku vježbu postoji
konfiguracija **LV*n*: GDB u QEMU-u** (npr. *LV3: GDB u QEMU-u*), koja korake
1, 3 i 4 izvodi sama. Postavite prijelomnu točku u kodu vježbe, u *Run and
Debug* odaberite konfiguraciju te vježbe i pritisnite **F5**. VS Code pokrene
zadatak *LV*n*: izgradnja i QEMU za GDB* (izgradnja, zatim QEMU koji
čeka), pričeka redak `gdbstub on :1234` i spoji GDB. Prva izgradnja vježbe
traje nekoliko minuta. U LV2 program ulazi u prekidnu rutinu `timer_alarm_cb`
tek kad je riješen TODO 3 (T4); prije toga stanite npr. na prvom retku
funkcije `app_main`. U LV5 `app_main` je u datoteci primjera odabranog u
`menuconfig` (zadano `pipeline.c`).

**Rad sa spojenim debuggerom:**

- Kad program stane na prijelomnoj točki, redak je označen žuto. Lijevo su
  *Variables* (varijable) i *Call Stack* (stog poziva, *backtrace*).
- Traka za ispravljanje pogrešaka na vrhu: *Continue* (F5, do sljedeće
  prijelomne točke), *Step Over* (F10, sljedeći redak), *Step Into* (F11,
  ulazak u funkciju), *Stop* (Shift-F5).
- Naredbe GDB-a iz priručnika vježbe upisujte u *Debug Console* (Ctrl-Shift-Y)
  s prefiksom `-exec`, npr. `-exec bt`, `-exec info registers` i
  `-exec print *edata`. Odatle kopirajte i transkript za predaju.
- Ispis programa iz emulatora je u ploči *Terminal*, u kartici zadatka.
- **Stop** (Shift-F5) zaustavlja GDB i emulator, a time završava i zadatak.
  Za novo spajanje ponovite korake 3 i 4 ili samo F5.

**Kad spajanje ne uspije:**

| Poruka ili simptom | Uzrok i rješenje |
|---|---|
| `Unable to start debugging. Unexpected GDB output from command "-target-select remote localhost:1234". could not connect …` | QEMU ne čeka: zadatak *QEMU za GDB* nije pokrenut ili je već završio. Pokrenite ga (korak 3), pa ponovno F5. |
| `Unable to start debugging. Program path '…' is missing or invalid.` | Odabrana vježba nije izgrađena (korak 1) ili ste odabrali program druge vježbe. |
| `build dir not found: build` u kartici zadatka | Vježba nije izgrađena (korak 1). |
| `gdb.sh: riscv32-esp-elf-gdb not found` | VS Code ne radi ondje gdje je ESP-IDF: na putu A prozor nije priključen na kontejner, u WSL2 prozor nije otvoren u WSL-u. |
| `with Docker, debug from VS Code attached to the course container` | Put A bez priključenog prozora (*Priprema*, gore). |
| program prolazi bez zaustavljanja | Prijelomna točka nije postavljena ili je na retku koji se ne izvodi. |
| program ne staje, a pogreške nema; u *Call Stack* nema ničega | Debugger nije spojen: zadatak *QEMU za GDB* ili *LV*n*: izgradnja i QEMU za GDB* pokrenut iz *Run Task* samo pokreće emulator koji čeka. Spojite debugger: *Run and Debug* → konfiguracija → F5 (korak 4). |
| `port 1234 is already in use` u kartici zadatka | Stari emulator još čeka: zatvorite njegovu karticu zadatka (ikona koša) ili u njoj pritisnite Ctrl-A, zatim X, pa ponovno F5. |

Bez VS Codea isto se radi iz dva terminala (odjeljak *Rad na vježbi*, *GDB iz
terminala*).

## Ažuriranje repozitorija

Repozitorij se tijekom semestra dopunjuje (upute, zadaci za VS Code, skripte),
a vaš rad u datotekama vježbi pritom ostaje. Novo stanje preuzimate u
**terminalu računala**, iz korijena repozitorija (na putu C u PowerShellu ili
prečacu ESP-IDF 5.5 PowerShell):

```sh
git pull
```

Ako git ne završi, nego javi jednu od ovih poruka:

| Poruka | Uzrok i rješenje |
|---|---|
| `Need to specify how to reconcile divergent branches` | Svoj rad ste commitali. Spojite ga s novim stanjem: `git pull --no-rebase --no-edit`. |
| `untracked working tree files would be overwritten by merge: …` | Napravili ste datoteku koju sada donosi repozitorij (npr. vlastiti `.vscode/launch.json`). Preimenujte je (npr. u `launch.json.moj`) i ponovite `git pull`. |
| `Your local changes to the following files would be overwritten by merge: …` | Promijenili ste datoteku koju je promijenio i repozitorij. Sklonite svoje promjene s `git stash`, zatim `git pull`, pa ih vratite s `git stash pop`. |

Nakon ažuriranja:

- **VS Code** nove zadatke i konfiguracije učita sam. Ako popis nije osvježen,
  F1 (macOS: Cmd-Shift-P) → *Developer: Reload Window*.
- **Put A:** zatvorite terminal u kojem radi `./scripts/docker-shell.sh` i
  pokrenite ga ponovno, jer tek novi kontejner preuzima promjene te skripte.
  Prozor VS Codea zatim ponovno priključite na kontejner. Kad nastavnik najavi
  novu razvojnu sliku (promjena `Dockerfile`), najprije obrišite staru s
  `docker rmi embedded-course`; `docker-shell.sh` zatim gradi novu (15–30 min).

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

### Logički analizator (LV3 i LV4)

Na klupi su logički analizatori s čipom Cypress FX2 (osam kanala, do 24 MHz),
a koriste se s besplatnim programom PulseView projekta sigrok. U emulatoru vam
ne trebaju: snimke za LV3 dane su u `labs/lab3_bme280/captures`.

**Instalacija.**

- **Linux (Debian, Ubuntu):** `sudo apt install pulseview sigrok-firmware-fx2lafw`,
  zatim analizator iskopčajte i ponovno priključite. Ako ga PulseView ne nađe
  ili javlja da nema pristup, nedostaju pravila udev: datoteke
  `60-libsigrok.rules` i `61-libsigrok-uaccess.rules` iz direktorija `contrib`
  izvornog koda libsigroka kopirajte u `/etc/udev/rules.d/` i izvedite
  `sudo udevadm control --reload-rules && sudo udevadm trigger`.
- **Windows:** sa stranice <https://sigrok.org/wiki/Downloads> preuzmite
  64-bitni instalacijski program PulseViewa. Analizatoru treba upravljački
  program WinUSB, koji se postavlja alatom Zadig (instalira se uz PulseView i
  nalazi u izborniku Start): s priključenim analizatorom u Zadigu odaberite
  *Options → List All Devices*, s popisa odaberite analizator, desno *WinUSB* i
  *Replace Driver*. Odaberite analizator, **nikako pločicu ESP32-C3**: njoj
  WinUSB uklanja port COM (vidi gore). Kod analizatora FX2 to ponekad treba
  ponoviti, jer se nakon što PulseView u njega upiše firmware javlja kao drugi
  uređaj. Postavka vrijedi samo za USB priključak u kojem je napravljena.
- **macOS:** sa iste stranice preuzmite PulseView (`.dmg`) i aplikaciju
  prevucite u *Applications*. To je inačica za procesore x86-64; na Macu s
  čipom Apple izvodi se preko Rosette 2, koju macOS ponudi instalirati pri
  prvom pokretanju. Ako macOS aplikaciju odbije otvoriti, prvi put je otvorite
  desnim klikom → *Open*.

**Spajanje.** Analizator samo sluša. Najprije spojite njegov GND s GND-om
pločice, jer bez zajedničke mase snimka nema smisla. Zatim za LV3 (I2C)
spojite kanal 0 na `GPIO5` (SDA) i kanal 1 na `GPIO6` (SCL), a za LV4 (PWM)
kanal 0 na `GPIO8`. Razine od 3,3 V analizator čita izravno (logičku jedinicu
prepoznaje od 2,0 V, a podnosi do 5 V). Izvode za napajanje na analizatoru,
ako ih ima, ne spajajte nikamo. U PulseViewu se kanali zovu D0–D7.

**Snimanje.** U PulseViewu odaberite *Connect to Device*, upravljački program
`fx2lafw` i *Scan for devices*; analizator se obično javlja kao Saleae Logic.
Uključite samo kanale koje koristite i postavite brzinu uzorkovanja i broj
uzoraka: za LV3 2 MHz i 10 M uzoraka (5 s, barem dva očitanja senzora; uz I2C
na 100 kHz to je 20 uzoraka po bitu), za LV4 4 MHz i 20 M uzoraka (5 s, više
od jednog ciklusa disanja od oko 4,1 s; korak od 0,25 µs je 1/800 perioda od
200 µs). Zatim *Run*. Ako snimanje stane prije kraja, smanjite brzinu
uzorkovanja, ne koristite USB razdjelnik (hub) i probajte drugi kabel.

**Dekoderi.** *Add protocol decoder → I²C*, uz SCL = D1 i SDA = D0: iznad
signala pojave se START, adresa, podatkovni bajtovi i ACK/NACK, isti slijed
koji u LV3 dekodirate ručno. Za LV4 dekoder *PWM* na kanalu D0 ispisuje period
i faktor ispune svakog impulsa; iste vrijednosti možete očitati i pokazivačima
(*cursors*). Za predaju priložite snimku zaslona ili spremljenu snimku
(*File → Save*, datoteka `.sr`).

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
