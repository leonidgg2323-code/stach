from pathlib import Path
root=Path(__file__).resolve().parent.parent
s=(root/'src/stach_bringup.cpp').read_text()
p=(root/'platformio.ini').read_text()
b=(root/'tools/build_firmware.py').read_text()
assert '[env:stach-waveshare-bringup]' in p
assert 'build_src_filter = -<*> +<stach_bringup.cpp>' in p
assert 'STACH_BRINGUP' in p and 'stach-waveshare-bringup' in b
assert 'void setup()' in s and 'void loop()' in s
assert s.index('#ifdef STACH_BRINGUP') < s.index('#include <Arduino.h>')
assert s.rstrip().endswith('#endif // STACH_BRINGUP: compiled out of all production firmware profiles.')
for needle in ('0xDA','LCD_EN,HIGH','I2C_SDA=47','I2C_SCL=48','Arduino_CO5300','Arduino_SH8601','10000000'):
    assert needle in s,needle
assert 'void setup()' in (root/'src/main.cpp').read_text()
print('STACH bring-up static integration: PASS')
