# Greenhouse Environment Monitoring System

An embedded C application for a **BeagleBone** board that monitors key environmental parameters in a greenhouse to help maintain optimal conditions for plant growth. It continuously reads three sensors and prints their values in real time:

- **Soil moisture** (analog, via ADC)
- **CO2 concentration** (analog, via ADC)
- **Temperature & pressure** (Bosch **BMP390** barometric sensor over I2C)

> Academic embedded-systems project (Master of Science in Computer Science).

---

## Overview

Maintaining a healthy greenhouse requires keeping environmental conditions within an ideal range. This system samples soil moisture, CO2 level, temperature, and barometric pressure once per second and reports them, giving a continuous picture of the greenhouse environment that can be used to drive irrigation, ventilation, or alerts.

## Hardware

- **BeagleBone** board (Linux, with sysfs IIO ADC and I2C support)
- **Soil moisture sensor** — analog output read via the on-board ADC (`in_voltage0_raw`)
- **CO2 sensor** — analog output read via the on-board ADC (`in_voltage1_raw`)
- **BMP390** — barometric pressure & temperature sensor on the I2C bus (`/dev/i2c-2`, address `0x76`)

## How It Works

1. The BMP390 is initialized on the I2C bus (`bmp390_init`).
2. A single control loop runs once per second and, on every cycle:
   - reads a fresh soil-moisture ADC value and converts it to a percentage,
   - reads a fresh CO2 ADC value and converts it to an approximate ppm,
   - reads raw temperature and pressure from the BMP390,
   - prints all readings.

ADC values are read from sysfs IIO files. Each read re-opens the file so every cycle reports a **current** value. Sensor conversions use the BeagleBone's 1.8 V ADC reference and a 12-bit (0–4095) range.

```
Soil moisture: 42.13%
CO2: 318.50 ppm
Pressure: 100872 (raw), Temperature: 24 C (approx)
-------------------------------------------
```

## Build & Run

On the BeagleBone (or any Linux host with the I2C/IIO interfaces wired up):

```bash
# Build
make

# Run
./greenhouse_monitor
```

Or compile directly:

```bash
gcc greenhouse_monitor.c -o greenhouse_monitor
```

Press **Ctrl+C** to stop. See [DEPENDENCIES.md](DEPENDENCIES.md) for the required system packages and kernel modules.

## Project Structure

```
.
├── greenhouse_monitor.c   # Main application (sensor reads + monitoring loop)
├── Makefile               # Build configuration
├── DEPENDENCIES.md        # System packages, headers, and I2C setup
├── .gitignore
├── LICENSE
└── README.md
```

## Notes & Limitations

- **BMP390 calibration:** The program prints the BMP390's *raw* temperature and pressure registers. For true °C and Pa, apply Bosch's compensation formulas using the sensor's factory calibration coefficients (read from its NVM registers). This is a known simplification.
- **Sensor calibration:** Soil-moisture and CO2 conversions use assumed voltage ranges (documented inline in `greenhouse_monitor.c`). Calibrate against your specific sensors for accurate absolute values.
- **ADC reference:** Conversions assume the BeagleBone's 1.8 V ADC reference; do not exceed 1.8 V on the analog pins.

## Possible Enhancements

- Apply full BMP390 calibration compensation for accurate temperature/pressure.
- Log readings to a CSV file or push to a cloud/IoT dashboard.
- Add threshold-based alerts and actuator control (irrigation pump, fan).
- Use POSIX threads or `select()` for independent per-sensor sampling rates.

## License

This project is licensed under the terms in the [LICENSE](LICENSE) file.
