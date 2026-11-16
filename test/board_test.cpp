#include "stach_board.h"
#include <cassert>
#include <iostream>
int main() {
using namespace stach::board;
assert(width==466 && height==466);
assert(qspi_cs==9 && qspi_clk==10 && qspi_d0==11 && qspi_d3==14);
assert(amoled_reset==21 && amoled_enable==42);
assert(i2c_sda==47 && i2c_scl==48);
assert(battery_adc==4 && imu_interrupt==8);
assert(sd_cs==38 && sd_sclk==41);
assert(validPinMap());
std::cout<<"Board pin map: 8 checks passed\n";
}
