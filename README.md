# Ugradbeni računalni sustavi — alatni lanac i polazne datoteke vježbi

*English version: [README.en.md](README.en.md)*

Repozitorij uz kolegij **Ugradbeni računalni sustavi** (Veleučilište u
Karlovcu, Stručni diplomski studij Mehatronika). Sadrži sve što treba da na
vlastitom računalu izgradite i pokrenete laboratorijske vježbe — na Linuxu,
macOS-u ili Windowsu, s Dockerom ili bez njega:

| Put | Sadržaj |
|---|---|
| `Dockerfile` | razvojna slika: ESP-IDF v5.5, Espressifov QEMU, alati za LV7 |
| `scripts/` | `docker-shell.sh` / `docker-shell.ps1` (ulazak u kontejner), `verify.sh` (provjera instalacije) |
| `labs/labN_*/starter/` | polazni projekti vježbi LV1–LV6 |
| `labs/common/` | skripte za emulator (`run_qemu*.sh`) i zajedničke komponente |
| `labs/lab6_mqtt/broker/` | MQTT posrednik i certifikati za LV6 |
| `labs/lab7_linux_ota/` | Buildroot konfiguracija i OTA alati za LV7 |

Priručnici vježbi, udžbenik i ostali dokumenti kolegija dijele se odvojeno, na
stranicama kolegija. Ciljna platforma je **ESP32-C3** (RISC-V); svaka vježba
može se izvesti i bez pločice, u emulatoru **QEMU**.

## Koji put odabrati

| | Docker | Bez Dockera |
|---|---|---|
| Postavljanje | jedna naredba, isto na svim sustavima | instalacija ESP-IDF-a i QEMU-a na računalo |
| Izgradnja i emulator (LV1–LV6) | da | da |
| LV7 (Buildroot, ugradbeni Linux) | da | samo na Linuxu i u WSL2 |
| Programiranje pločice preko USB-a | samo na Linuxu | da |

Preporuka: krenite s **Dockerom**. Kad dobijete pločicu, na macOS-u i Windowsu
dodajte i instalaciju bez Dockera (odnosno WSL2) — mogu postojati usporedno.

Na Windowsu repozitorij klonirajte unutar WSL2 ili, ako ga klonirate Gitom za
Windows, ostavite ga takvim kakav jest: datoteka `.gitattributes` čuva
Linuxove završetke redaka u skriptama.

```sh
git clone https://github.com/vvyroubal/urs-toolchain.git
cd urs-toolchain
```

## Put A: s Dockerom

1. Instalirajte Docker: [Docker Desktop](https://www.docker.com/products/docker-desktop/)
   na Windowsu (s WSL2 pozadinom, što je zadano) i macOS-u, odnosno Docker
   Engine na Linuxu.
2. Otvorite ljusku u kontejneru. Prvo pokretanje gradi sliku
   `embedded-course`, što traje nekoliko minuta i preuzima nekoliko GB.

   Linux, macOS, WSL2:
   ```sh
   ./scripts/docker-shell.sh
   ```
   Windows (PowerShell):
   ```powershell
   .\scripts\docker-shell.ps1
   ```
3. U kontejneru provjerite instalaciju:
   ```sh
   bash scripts/verify.sh
   ```
   Skripta mora završiti retkom `verify.sh: PASS`.

Repozitorij je u kontejneru dostupan kao `/work`, okruženje ESP-IDF-a već je
aktivno, a sve što izgradnja stvori ostaje u repozitoriju i nakon izlaska.
Sliku možete izgraditi i ručno (`docker build -t embedded-course .`) te je
pokretati naredbom `docker run` kako piše u priručnicima vježbi.

## Put B: bez Dockera

### Linux (i WSL2 na Windowsu)

Na **Windowsu** najprije instalirajte WSL2: u PowerShellu pokrenutom kao
administrator izvedite `wsl --install`, ponovno pokrenite računalo i otvorite
terminal Ubuntu. Sve daljnje naredbe pišu se u njemu, a repozitorij klonirajte
u Linuxov datotečni sustav (`~/`), ne u `/mnt/c/...`.

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
bash scripts/verify.sh
```

### macOS

Potreban je [Homebrew](https://brew.sh).

```sh
# 1. alati i biblioteke koje trebaju ESP-IDF i QEMU
brew install cmake ninja dfu-util ccache python3 coreutils \
    libgcrypt glib pixman sdl2 libslirp

# 2. - 5. kao na Linuxu
git clone -b v5.5 --recursive https://github.com/espressif/esp-idf.git ~/esp-idf
~/esp-idf/install.sh esp32c3
. ~/esp-idf/export.sh
python3 $IDF_PATH/tools/idf_tools.py install qemu-riscv32
. ~/esp-idf/export.sh
bash scripts/verify.sh
```

`coreutils` donosi naredbu `gtimeout`, koju trebaju skripte za emulator.

### Windows bez WSL2

Službeni [ESP-IDF Tools Installer](https://dl.espressif.com/dl/esp-idf/) za
**v5.5** instalira alatni lanac izravno u Windows i dodaje prečace
*ESP-IDF 5.5 CMD* i *ESP-IDF 5.5 PowerShell*. Taj je put dovoljan za izgradnju
(`idf.py build`) i programiranje pločice, ali skripte ovoga repozitorija
trebaju `bash`, a LV7 treba Linux. Zato ga kombinirajte s Dockerom (put A)
ili umjesto njega upotrijebite WSL2.

## Rad na vježbi

Primjer za LV1 (iz korijena repozitorija; isto vrijedi u kontejneru i izvan
njega):

```sh
cd labs/lab1_toolchain/starter
idf.py set-target esp32c3
idf.py build
bash ../../common/run_qemu.sh build        # pokretanje u emulatoru, 30 s
```

Ispis emulatora sprema se i u `build/qemu.log`. Ostale vježbe imaju isti
oblik; što treba dovršiti i kako se provjerava piše u priručniku pojedine
vježbe.

| Vježba | Direktorij | Napomena |
|---|---|---|
| LV1 | `labs/lab1_toolchain/` | alatni lanac, prvi build |
| LV2 | `labs/lab2_gpio_irq/` | registri, prekidi, GPTimer |
| LV3 | `labs/lab3_bme280/` | I2C i BME280; snimke signala u `captures/` |
| LV4 | `labs/lab4_adc_pwm/` | ADC i PWM |
| LV5 | `labs/lab5_rtos/` | FreeRTOS |
| LV6 | `labs/lab6_mqtt/` | MQTT preko TLS-a; mrežni emulator `run_qemu_net.sh` |
| LV7 | `labs/lab7_linux_ota/` | Buildroot i OTA; izgradnja traje 30–60 min |

LV3 i LV4 u emulatoru koriste simulirani senzor odnosno ADC; za rad na
pločici gradi se s dodatnim slojem postavki `sdkconfig.hw`, kako piše u
priručniku.

**LV7 na macOS-u i Windowsu:** Buildroot se mora graditi na Dockerovu
imenovanom volumenu, ne u montiranom direktoriju repozitorija:

```sh
docker run --rm -v "$PWD:/work" -w /work -v lab7_br:/br embedded-course \
    bash -lc 'make -C labs/lab7_linux_ota -f Makefile.buildroot all BR_DIR=/br'
```

## Programiranje pločice

```sh
idf.py -p <port> flash monitor      # izlaz iz monitora: Ctrl-]
```

ESP32-C3 ima ugrađeno USB sučelje, pa poseban upravljački program u pravilu
nije potreban.

| Sustav | Port | Napomena |
|---|---|---|
| Linux | `/dev/ttyACM0` | korisnik mora biti u grupi `dialout`: `sudo usermod -aG dialout $USER`, pa odjava i prijava |
| macOS | `/dev/cu.usbmodem*` | |
| Windows (instalacijski program) | `COM<n>` | broj porta vidi se u Upravitelju uređaja |
| WSL2 | `/dev/ttyACM0` | pločicu treba proslijediti alatom `usbipd`, vidi niže |
| Docker na Linuxu | `/dev/ttyACM0` | `URS_PORT=/dev/ttyACM0 ./scripts/docker-shell.sh` |
| Docker Desktop (macOS, Windows) | — | USB uređaji se ne prosljeđuju u kontejner |

**WSL2:** u PowerShellu pokrenutom kao administrator:

```powershell
winget install usbipd
usbipd list                           # pronađite BUSID pločice, npr. 2-3
usbipd bind --busid <BUSID>           # jednom po uređaju
usbipd attach --wsl --busid <BUSID>   # nakon svakog priključivanja
```

## Česti problemi

- **`idf.py: command not found`** — okruženje nije aktivirano u ovom
  terminalu: `. ~/esp-idf/export.sh`.
- **`qemu-system-riscv32 not found on PATH`** — nakon instalacije QEMU-a
  treba ponovno pokrenuti `. ~/esp-idf/export.sh`.
- **macOS: `neither 'timeout' nor 'gtimeout' found`** — `brew install coreutils`.
- **Windows: `bad interpreter` ili `$'\r': command not found`** — skripta je
  dobila Windowsove završetke redaka. Ponovno klonirajte repozitorij (datoteka
  `.gitattributes` to sprečava) ili ga klonirajte unutar WSL2.
- **Linux, nakon rada u Dockeru: datoteke u `build/` pripadaju korisniku
  root** — kontejner radi kao root; vratite vlasništvo naredbom
  `sudo chown -R $USER .` ili obrišite direktorij `build/` iz kontejnera.
- **`ModuleNotFoundError` pri pokretanju `idf.py`** — aktivna je neka druga
  Python okolina (venv ili conda); deaktivirajte je prije `export.sh`.
- **Windows: PowerShell odbija pokrenuti `docker-shell.ps1`** —
  `powershell -ExecutionPolicy Bypass -File .\scripts\docker-shell.ps1`.

## Napomena o ključu u LV7

`labs/lab7_linux_ota/ota/keys/demo_signing_key.INSECURE.pem` namjerno je
objavljen privatni ključ za potpisivanje. Služi isključivo vježbi; nikad ga ne
upotrebljavajte ni za što drugo. Zašto je to u stvarnom proizvodu nedopustivo,
tema je vježbe LV7 i poglavlja o sigurnosti.
