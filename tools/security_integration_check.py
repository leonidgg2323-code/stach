#!/usr/bin/env python3
"""CI static contract tests: all WebServer handlers must explicitly use the guard."""
from pathlib import Path
import re
root=Path(__file__).resolve().parents[1]
main=(root/'src/main.cpp').read_text()
web=(root/'web/stach-panel.html').read_text()
handlers=list(re.finditer(r'server\.on\("([^"]+)",\s*HTTP_(GET|POST),\s*\[\]\(\)\s*\{',main))
assert len(handlers)>=18, f'unexpectedly few routes: {len(handlers)}'
for handler in handlers:
    path,method=handler.group(1),handler.group(2)
    rest=main[handler.end():handler.end()+180]
    expected='if(!guardRequest(true))return;' if method=='POST' else 'if(!guardRequest(false))return;'
    assert expected in rest, f'unprotected route: {method} {path}'
assert 'server.collectHeaders(collectedHeaders,sizeof(collectedHeaders)/sizeof(collectedHeaders[0]))' in main
assert 'if(!owner::authorized(server))' in main
assert 'if(!guardEnvelope())return;' in main
assert '"X-Stach-Token"' in main and '"Origin"' in main and '"Sec-Fetch-Site"' in main
assert 'stach::security::equalToken(sessionToken,server.header("X-Stach-Token").c_str())' in main
assert 'stach::security::allowedHost(server.hostHeader().c_str())' in main
assert 'stach::security::RateGate requestRate' in main
assert 'server.send(429,' in main and 'server.send(413,' in main and 'server.send(403,' in main
assert 'Content-Security-Policy' in main and 'X-Content-Type-Options' in main and 'Referrer-Policy' in main
assert 'WiFi.softAPgetStationNum()' in main and 'if(startAccessPoint())' in main
assert '"/api/session"' in main and '"/api/health"' in main
assert "getControlToken()" in web and "'X-Stach-Token':await getControlToken()" in web
assert "securityToken" not in web # token is never persisted to localStorage
for forbidden in ['localStorage.setItem(', 'document.cookie=', 'Serial.println("Stach Wi-Fi password', 'Serial.println("WiFi password']:
    assert forbidden not in main+web, forbidden
print(f'Stage-11 integration contracts passed: {len(handlers)} guarded API/web routes')
