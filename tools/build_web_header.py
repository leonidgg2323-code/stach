#!/usr/bin/env python3
"""Generate an Arduino PROGMEM header from the exact offline preview HTML."""
from pathlib import Path
root = Path(__file__).resolve().parents[1]
html = (root / 'web/stach-panel.html').read_text(encoding='utf-8')
assert ')STACHWEB"' not in html
out = root / 'include/stach_web.h'
out.write_text('#pragma once\n// Generated from web/stach-panel.html. Run python3 tools/build_web_header.py after edits.\n'
               'namespace stach { namespace web {\nstatic const char page[] PROGMEM = R"STACHWEB('
               + html + ')STACHWEB";\n}} // namespace stach::web\n', encoding='utf-8')
print(f'STACH panel: {len(html.encode("utf-8"))} UTF-8 bytes; embedded header written')
