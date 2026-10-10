# adc_sim

Synthetic implementation of the shared `lab_adc.h` API (see
`labs/common/components/lab_adc_defs`), used wherever a real analog signal is
not available: QEMU (no ADC input device), CI, or bench-free desk work.

## Sim vs. real driver selection mechanism

Both this component's `src/adc_sim.c` and the lab4 project's real driver
(`main/lab_adc_oneshot.c`, solution and starter alike) implement the exact
same two functions, `lab_adc_init()` and `lab_adc_read_raw()`. Only one of
them may ever be compiled into a given firmware image, or the link step
fails with duplicate symbols.

Selection is a single Kconfig bool, `CONFIG_LAB_SIM_ADC` (declared in this
directory's `Kconfig`, default **y**), used as a whole-file guard at the top
of each source file -- identical mechanism to `bme280_sim`/`bme280_i2c.c` in
Lab 3 (see that component's README for the general explanation of why an
undefined Kconfig macro reads as `0` in `#if`, making the two files mutually
exclusive at compile time with no extra `#ifdef`/`#else` plumbing).

- `CONFIG_LAB_SIM_ADC=y` (default, and what the lab4 solution's
  `sdkconfig.defaults` sets explicitly): `adc_sim.c` is live,
  `lab_adc_oneshot.c` is empty. This is what boots in QEMU.
- `CONFIG_LAB_SIM_ADC` unset/n (lab4 solution's `sdkconfig.hw` fragment):
  `lab_adc_oneshot.c` is live, `adc_sim.c` is empty. This is the
  real-hardware path -- build with
  `idf.py -D SDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.hw" build`.

`app_main()` in `main.c` calls only `lab_adc_init()`/`lab_adc_read_raw()`
from `lab_adc.h` and never needs to know which implementation it is linked
against.

## Ground-truth data model (exact numbers the manual quotes)

Deterministic: a **seeded** PRNG (xorshift32, fixed seed `0x2463D9A1`), not a
time/entropy source, so the exact 1000-sample sequence -- and therefore the
histogram, mean and standard deviation -- repeats identically every boot.

| Quantity                          | Value                                    |
|------------------------------------|-------------------------------------------|
| Nominal input voltage              | 1.500 V                                    |
| Assumed ADC full-scale             | 2.500 V (matches the real driver's `ADC_ATTEN_DB_12` choice) |
| ADC width                          | 12 bit (codes 0..4095)                     |
| Ideal (error-free) code            | `round(1500/2500 * 4095) = 2457`           |
| Gain error                         | x1.03 (`SIM_ADC_GAIN_ERR`)                 |
| Offset error                       | +25 LSB (`SIM_ADC_OFFSET_ERR_LSB`)         |
| Noise                              | Gaussian, sigma = 8 LSB (`SIM_ADC_NOISE_SIGMA_LSB`), via Box-Muller |
| PRNG                               | xorshift32, seed `0x2463D9A1` (`SIM_ADC_PRNG_SEED`), reseeded in `lab_adc_init()` |

Transfer function (`lab_adc_read_raw()`):

```
raw = round(2457 * 1.03 + 25 + noise), clamped to [0, 4095]
    = round(2555.71 + noise)
```

So a large sample (e.g. this lab's 1000-reading histogram) should show a
sample mean close to **2555.7 raw** with a sample standard deviation close
to **8 raw** -- exactly the known, fixed gain/offset error a two-point
calibration is meant to remove.

Two-point calibration recovers the nominal voltage: correcting with
gain = `1e6 / 1.03 = 970874` ppm and offset = `25` LSB (the
`CONFIG_LAB4_CAL_GAIN_PPM` / `CONFIG_LAB4_CAL_OFFSET` Kconfig defaults in the
lab4 projects, chosen to match these constants exactly) recovers ~2457 raw,
i.e. **~1.500 V**, to within a few raw counts / a few mV.
