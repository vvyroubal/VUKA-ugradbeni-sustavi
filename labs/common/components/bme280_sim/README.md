# bme280_sim

Synthetic implementation of the shared `bme280.h` API (see
`labs/common/components/bme280_defs`), used wherever a real BME280 is not
available: QEMU (no I2C bus device), CI, or bench-free desk work.

## Sim vs. real driver selection mechanism

Both this component's `src/bme280_sim.c` and the lab3 project's real driver
(`main/bme280_i2c.c`, solution and starter alike) implement the exact same
two functions, `bme280_init()` and `bme280_read()`. Only one of them may
ever be compiled into a given firmware image, or the link step fails with
duplicate symbols.

Selection is a single Kconfig bool, `CONFIG_LAB_SIM_BME280` (declared in
this directory's `Kconfig`, default **y**), used as a whole-file guard at
the top of each source file:

```c
#include "sdkconfig.h"
#if CONFIG_LAB_SIM_BME280
  /* ... entire bme280_sim.c body ... */
#endif
```

```c
#include "sdkconfig.h"
#if !CONFIG_LAB_SIM_BME280
  /* ... entire bme280_i2c.c body ... */
#endif
```

When `CONFIG_LAB_SIM_BME280` is unset, ESP-IDF's Kconfig machinery simply
omits the `#define CONFIG_LAB_SIM_BME280 1` line from the generated
`sdkconfig.h` — an undefined macro reads as `0` in a preprocessor `#if`, so
both guards resolve correctly with no extra `#ifdef`/`#else` plumbing
needed. The two files are therefore mutually exclusive at compile time:
whichever one is "on" contributes the API's only definition; the other
compiles to an empty translation unit.

- `CONFIG_LAB_SIM_BME280=y` (default, and what the lab3 solution's
  `sdkconfig.defaults` sets explicitly): `bme280_sim.c` is live,
  `bme280_i2c.c` is empty. This is what boots in QEMU.
- `CONFIG_LAB_SIM_BME280` unset/n (lab3 solution's `sdkconfig.hw` fragment):
  `bme280_i2c.c` is live, `bme280_sim.c` is empty. This is the real-hardware
  path — build with
  `idf.py -D SDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.hw" build`.

`app_main()` in `main.c` calls only `bme280_init()`/`bme280_read()` from
`bme280.h` and never needs to know which implementation it is linked
against.

## Synthetic data model

Deterministic (no RNG, so logs/captures are reproducible run to run),
driven solely by `esp_timer_get_time()` (microseconds since boot):

| Quantity    | Center      | Amplitude | Period |
|-------------|-------------|-----------|--------|
| Temperature | 22.5 degC   | +/-2.5 degC (20-25 degC) | 90 s |
| Pressure    | 1013 hPa    | +/-1.5 hPa  | 150 s |
| Humidity    | 45 %RH      | +/-3 %RH    | 120 s |
