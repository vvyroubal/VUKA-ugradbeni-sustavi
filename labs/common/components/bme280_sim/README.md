# bme280_sim

Synthetic implementation of the shared `bme280.h` API (see
`labs/common/components/bme280_defs`) for Labs 5 and 6, whose pipelines need
a temperature/pressure/humidity source but not the BME280 driver itself:
`bme280_init()`/`bme280_read()` return slowly varying synthetic values, in
QEMU and on a board alike. Selected by the Kconfig bool
`CONFIG_LAB_SIM_BME280` (this directory's `Kconfig`, default **y**; the
Lab 5 and Lab 6 `sdkconfig.defaults` set it explicitly), used as a
whole-file guard in `src/bme280_sim.c`.

Lab 3 does **not** use this component: there the students write the
driver (`main/bme280_i2c.c`), its `sdkconfig.defaults` turns
`CONFIG_LAB_SIM_BME280` off, and in QEMU the driver reads the simulated
register map of `bme280_regsim` instead of the I2C bus. A Lab 3
`sdkconfig` that still has `CONFIG_LAB_SIM_BME280=y` (created before the
2026-11 redesign) stops the build with an `#error` asking to delete
`sdkconfig` and `build/`.

## Synthetic data model

Deterministic (no RNG, so logs/captures are reproducible run to run),
driven solely by `esp_timer_get_time()` (microseconds since boot):

| Quantity    | Center      | Amplitude | Period |
|-------------|-------------|-----------|--------|
| Temperature | 22.5 degC   | +/-2.5 degC (20-25 degC) | 90 s |
| Pressure    | 1013 hPa    | +/-1.5 hPa  | 150 s |
| Humidity    | 45 %RH      | +/-3 %RH    | 120 s |
