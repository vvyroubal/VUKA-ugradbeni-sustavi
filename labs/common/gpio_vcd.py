#!/usr/bin/env python3
"""gpio_vcd.py -- turn a QEMU esp32c3 trace into one VCD file per GPIO pin.

Usage: gpio_vcd.py <trace.log> <out_dir>

<trace.log> is a QEMU log written with
    -msg timestamp=on -trace memory_region_ops_write -D <trace.log>
(run_qemu.sh / run_qemu.ps1 add these options with --gpio-vcd / -GpioVcd).
Every write to the GPIO block (QEMU region 'esp32.gpio', base 0x60004000)
updates a model of the GPIO_OUT and GPIO_ENABLE registers, whether the
program wrote them directly or through gpio_set_level(). Each pin that was
ever an output gets <out_dir>/gpioN.vcd with its LOW/HIGH changes; time is
in microseconds since QEMU's first traced write, i.e. roughly since boot.
Open the files in PulseView (Import -> Value Change Dump) or GTKWave.

Limits: only levels the program sets through the GPIO output registers are
seen -- not inputs (QEMU does not drive them) and not signals a peripheral
routes through the GPIO matrix (LEDC PWM, I2C, UART). Times are the host's
clock while QEMU runs: good to about a millisecond, not to microseconds.
"""
import pathlib
import re
import sys

BASE = 0x60004000
OUT, OUT_W1TS, OUT_W1TC = 0x04, 0x08, 0x0C
EN, EN_W1TS, EN_W1TC = 0x20, 0x24, 0x28
PINS = 22                                  # ESP32-C3: GPIO0..GPIO21
STAMP = re.compile(r"@(\d+)\.(\d+):memory_region_ops_write ")
WRITE = re.compile(r"addr (0x[0-9a-fA-F]+) value (0x[0-9a-fA-F]+) "
                   r".*name 'esp32\.gpio'")


def changes(trace):
    """Yield (time_us, pin, level) for every output level change."""
    out = en = 0
    t0 = None
    last = {}
    with open(trace, errors="replace") as f:
        for line in f:
            s = STAMP.search(line)
            if not s:
                continue
            t = int(s[1]) * 1_000_000 + int(s[2])
            if t0 is None:
                t0 = t
            w = WRITE.search(line)
            if not w:
                continue
            off, val = int(w[1], 16) - BASE, int(w[2], 16)
            if off == OUT:
                out = val
            elif off == OUT_W1TS:
                out |= val
            elif off == OUT_W1TC:
                out &= ~val
            elif off == EN:
                en = val
            elif off == EN_W1TS:
                en |= val
            elif off == EN_W1TC:
                en &= ~val
            for pin in range(PINS):
                if en >> pin & 1:
                    level = out >> pin & 1
                    if last.get(pin) != level:
                        last[pin] = level
                        yield t - t0, pin, level


def main(argv):
    if len(argv) != 3:
        sys.stderr.write("Usage: gpio_vcd.py <trace.log> <out_dir>\n")
        return 1
    events = {}
    for t, pin, level in changes(argv[1]):
        events.setdefault(pin, []).append((t, level))
    out_dir = pathlib.Path(argv[2])
    out_dir.mkdir(parents=True, exist_ok=True)
    for old in out_dir.glob("gpio*.vcd"):
        old.unlink()
    if not events:
        print("gpio_vcd.py: no GPIO pin was used as an output")
    for pin, ev in sorted(events.items()):
        path = out_dir / ("gpio%d.vcd" % pin)
        with open(path, "w", newline="\n") as f:
            f.write("$timescale 1us $end\n")
            f.write("$scope module esp32c3 $end\n")
            f.write("$var wire 1 ! GPIO%d $end\n" % pin)
            f.write("$upscope $end\n$enddefinitions $end\n")
            f.write("#0\nx!\n")             # unknown until it is an output
            for t, level in ev:
                f.write("#%d\n%d!\n" % (t, level))
        print("gpio_vcd.py: %s (%d changes)" % (path, len(ev)))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
