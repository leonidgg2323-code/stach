from pathlib import Path
import sys

Import("env")
sys.path.insert(0, str(Path(env.subst("$PROJECT_DIR")) / "tools"))
from patch_ble_mouse import patch_file

patch_file(Path(env.subst("$PROJECT_LIBDEPS_DIR/$PIOENV")) / "ESP32 BLE Mouse" / "BleMouse.cpp")
