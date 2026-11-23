#!/usr/bin/env python3
"""Static CI assertions for the dedicated Waveshare diagnostics route."""
from pathlib import Path
root=Path(__file__).resolve().parents[1]
main=(root/'src/main.cpp').read_text(encoding='utf-8')
report=(root/'include/stach_hw_report.h').read_text(encoding='utf-8')
assert 'server.on("/api/hardware", HTTP_GET, [](){\n    if(!guardRequest(false))return;' in main
assert main.count("hardwareValidated") >= 2
assert 'evidence.flashBytes=ESP.getFlashChipSize()' in main
assert 'evidence.psramBytes=ESP.getPsramSize()' in main
assert 'bootHardwareReport=stach::hwreport::evaluate(evidence);' in main
assert 'stach::display::diagnosticPattern();' in main
assert 'hardwareTestUntilMs=tickMs+5000' in main
assert 'wasScreenDark=false; // ensure a previously sleeping screen can be cleaned up' in main
assert 'if(int32_t(hardwareTestUntilMs-now)>0 && !airMouse.active())' in main
assert 'FLASH_EXPECTED_BYTES = 16u * 1024u * 1024u' in report
assert 'r.basicDriversReady = (r.warnings & CRITICAL) == 0;' in report
print('Stage-2 hardware diagnostics routes and guards: OK')
