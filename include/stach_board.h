#pragma once
// Waveshare ESP32-S3-Touch-AMOLED-1.43 (standard/B) hardware map.
// Not suitable for 1.43C without a separate schematic and BSP.
#include <stdint.h>
namespace stach { namespace board {
constexpr int width=466, height=466;
#ifdef STACH_EYES_175
constexpr int qspi_cs=12, qspi_clk=38, qspi_d0=4, qspi_d1=5, qspi_d2=6, qspi_d3=7;
constexpr int amoled_reset=39, amoled_enable=-1;
constexpr int i2c_sda=15, i2c_scl=14;
constexpr int battery_adc=-1;
constexpr uint8_t touch_address=0x5a;
#else
constexpr int qspi_cs=9, qspi_clk=10, qspi_d0=11, qspi_d1=12, qspi_d2=13, qspi_d3=14;
constexpr int amoled_reset=21, amoled_enable=42;
constexpr int i2c_sda=47, i2c_scl=48, imu_interrupt=8;
constexpr int battery_adc=4;
constexpr int sd_cs=38, sd_mosi=39, sd_miso=40, sd_sclk=41;
constexpr uint8_t touch_address=0x38;
#endif
constexpr uint8_t qmi8658_address_a=0x6A, qmi8658_address_b=0x6B;
constexpr uint8_t pcf85063_address=0x51;
constexpr uint8_t ft3168_address=0x38;
constexpr bool validPinMap() {return qspi_cs!=qspi_clk && i2c_sda!=i2c_scl && amoled_reset!=amoled_enable;}
static_assert(validPinMap(),"Conflicting board pins");
static_assert(width==466 && height==466,"Unexpected display dimensions");
}}
