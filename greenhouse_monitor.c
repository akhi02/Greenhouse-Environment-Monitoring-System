/*
 * Greenhouse Environment Monitoring System
 * -----------------------------------------
 * Reads three sensors on a BeagleBone board and prints their values once per
 * cycle in a single continuous loop:
 *   - Soil moisture sensor (analog, via ADC)
 *   - CO2 sensor           (analog, via ADC)
 *   - BMP390 sensor        (I2C) for temperature and pressure
 *
 * Build:  make           (or: gcc greenhouse_monitor.c -o greenhouse_monitor)
 * Run:    ./greenhouse_monitor
 */

// Including all necessary libraries
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

// Buffer size, ADC sysfs path, and BMP390 I2C register/address values
#define MAX_BUF              64
#define ADC_PATH             "/sys/bus/iio/devices/iio:device0/in_voltage"
#define ADC_MAX              4095.0f   // 12-bit ADC
#define VREF                 1.8f      // BeagleBone ADC reference voltage (1.8 V)
#define BMP390_ADDR          0x76
#define BMP390_REG_PRESS_MSB 0x04
#define BMP390_REG_TEMP_MSB  0x07
#define I2C_DEVICE           "/dev/i2c-2"
#define LOOP_DELAY_SEC       1

// Global file descriptor for the BMP390 I2C device
int bmp390_fd;

// Initialize the BMP390 sensor over I2C
int bmp390_init(void)
{
    if ((bmp390_fd = open(I2C_DEVICE, O_RDWR)) < 0)
    {
        perror("Failed to open I2C device");
        return -1;
    }
    if (ioctl(bmp390_fd, I2C_SLAVE, BMP390_ADDR) < 0)
    {
        perror("Failed to select BMP390 device");
        return -1;
    }
    return 0;
}

// Read `len` bytes from a BMP390 register into `data`
void bmp390_read(int reg, char *data, int len)
{
    char cmd = (char) reg;
    if (write(bmp390_fd, &cmd, 1) != 1)
    {
        perror("I2C write");
        return;
    }
    if (read(bmp390_fd, data, len) != len)
    {
        perror("I2C read");
    }
}

// Read the raw pressure value from the BMP390
int bmp390_get_pressure(void)
{
    char data[3];
    bmp390_read(BMP390_REG_PRESS_MSB, data, 3);
    return ((data[0] << 16) | (data[1] << 8) | data[2]) >> 4;
}

// Read the raw temperature value from the BMP390
int bmp390_get_temperature(void)
{
    char data[3];
    bmp390_read(BMP390_REG_TEMP_MSB, data, 3);
    return ((data[0] << 16) | (data[1] << 8) | data[2]) >> 8;
}

/*
 * Read a fresh raw value from an ADC sysfs file.
 * The file is opened and closed each call so every read returns a current
 * value (sysfs iio files must be re-opened or rewound between reads).
 * Returns the integer ADC reading, or -1 on error.
 */
int read_adc_raw(const char *path)
{
    char buf[MAX_BUF];
    int fd = open(path, O_RDONLY);
    if (fd < 0)
    {
        perror("open ADC");
        return -1;
    }

    int n = read(fd, buf, sizeof(buf) - 1);
    close(fd);

    if (n < 0)
    {
        perror("read ADC");
        return -1;
    }

    buf[n] = '\0';
    return atoi(buf);
}

// Convert a soil-moisture ADC reading to a percentage (clamped 0-100)
float soil_moisture_percent(int raw)
{
    // Convert ADC value to voltage, then voltage to percentage.
    // Calibration assumption: 0.85 V (dry) .. 1.8 V (wet).
    float voltage = raw * VREF / ADC_MAX;
    float percentage = (voltage - 0.85f) * 100.0f / (1.8f - 0.85f);

    if (percentage < 0.0f)   percentage = 0.0f;
    if (percentage > 100.0f) percentage = 100.0f;
    return percentage;
}

// Convert a CO2 ADC reading to an approximate ppm value
float co2_ppm(int raw)
{
    // Convert ADC value to voltage, then voltage to ppm.
    // Calibration assumption: 0 .. VREF maps to 0 .. 500 ppm.
    float voltage = raw * VREF / ADC_MAX;
    return voltage * 500.0f / VREF;
}

// Main: initialize the BMP390, then continuously poll all three sensors
int main(void)
{
    if (bmp390_init() < 0)
    {
        fprintf(stderr, "BMP390 initialization failed. Exiting.\n");
        return EXIT_FAILURE;
    }

    printf("Greenhouse Environment Monitoring System started.\n");
    printf("Press Ctrl+C to stop.\n\n");

    // Single loop: read each sensor fresh every cycle and print the results
    while (1)
    {
        int soil_raw = read_adc_raw(ADC_PATH "0_raw");
        int co2_raw  = read_adc_raw(ADC_PATH "1_raw");
        int pressure = bmp390_get_pressure();
        int temperature = bmp390_get_temperature();

        if (soil_raw >= 0)
            printf("Soil moisture: %.2f%%\n", soil_moisture_percent(soil_raw));
        if (co2_raw >= 0)
            printf("CO2: %.2f ppm\n", co2_ppm(co2_raw));

        // NOTE: BMP390 raw register values are printed here. For calibrated
        // temperature / pressure, apply Bosch's compensation formulas using
        // the sensor's factory calibration coefficients (NVM registers).
        printf("Pressure: %d (raw), Temperature: %d C (approx)\n",
               pressure, temperature / 100);
        printf("-------------------------------------------\n");

        sleep(LOOP_DELAY_SEC);
    }

    // Close the I2C device (unreachable in this infinite loop, kept for clarity)
    close(bmp390_fd);
    return EXIT_SUCCESS;
}
