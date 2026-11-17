#!/usr/bin/env python3
"""Ensure firmware page = offline demo page; validate DOM controls & JS syntax."""
from pathlib import Path
from html.parser import HTMLParser
import subprocess, tempfile, re, sys
root = Path(__file__).resolve().parents[1]
html = (root / 'web/stach-panel.html').read_text(encoding='utf-8')
header = (root / 'include/stach_web.h').read_text(encoding='utf-8')
assert 'R"STACHWEB(' + html + ')STACHWEB";' in header, 'Embedded HTML differs from preview; regenerate header'
assert (root / 'preview/stach-panel.html').read_text(encoding='utf-8') == html, 'Offline preview differs from device page'
class Elements(HTMLParser):
    def __init__(self): super().__init__();self.ids=[];self.attrs=[]
    def handle_starttag(self, name, attrs):
        d=dict(attrs);self.attrs.append((name,d));
        if 'id' in d:self.ids.append(d['id'])
e=Elements();e.feed(html)
assert len(e.ids) == len(set(e.ids)), 'Duplicate DOM ids'
for item in ['app','connection','connectionText','screen','eyes','leftEye','rightEye','leftPupil','rightPupil',
             'joystick','knob','centerGaze','idleMotion','tiltEnabled','ssid','ip','battery','charging','touch',
             'imu','gesture','clients','toast','pairingLabel','emotionButtons','faceTag','activeEmotion',
             'mousePanel','mouseModeBtn','mouseStatus','mouseLed','mousePad','mousePointer','mouseCalibrateBtn',
             'mouseSensitivity','mouseSensitivityValue','mouseInvertX','mouseInvertY','mouseAxisYX',
             'mouseLeftBtn','mouseRightBtn','mouseScrollUp','mouseScrollDown',
             'modesPanel','modePreview','blinkEnabled','eyeBrightness','eyeBrightnessValue',
             'gazeActivity','gazeActivityValue']:
    assert item in e.ids, f'Missing control: {item}'
assert set(d.get('data-emotion') for _,d in e.attrs if 'data-emotion' in d)=={'neutral','angry','sleepy','curious','happy'}
assert set(d.get('data-pairing') for _,d in e.attrs if 'data-pairing' in d)=={'face','wifi','portal'}
assert set(d.get('data-mode') for _,d in e.attrs if 'data-mode' in d)=={'face','mouse','wifi','portal'}
assert '<script>' in html and '</script>' in html
js = html.split('<script>',1)[1].split('</script>',1)[0]
with tempfile.NamedTemporaryFile(suffix='.js',mode='w',encoding='utf-8') as f:
    f.write(js);f.flush()
    subprocess.run(['node','--check',f.name],check=True)
main=(root/'src/main.cpp').read_text(encoding='utf-8')
for endpoint in ['/api/status','/api/expression','/api/pairing','/api/gaze','/api/settings',
                 '/api/mouse/mode','/api/mouse/calibrate','/api/mouse/config','/api/mouse/click',
                 '/api/mouse/scroll','/api/mouse/move','/api/mode','/api/appearance']:
    assert endpoint in html and endpoint in main, f'API contract mismatch: {endpoint}'
assert 'stach::web::page' in main
assert 'remoteGaze.apply(frame,now)' in main
assert 'WiFi.softAPgetStationNum()' in main
assert 'chargingKnown' in html and 'batteryKnown' in html
assert 'surprised' not in html.lower()
assert 'glow' not in html.lower() and 'filter:blur' not in html.lower()
assert 'https://' not in html and 'http://' not in html, 'Panel must not load external resources'
print(f'Stage-10 web checks: {len(e.ids)} unique IDs; {len(js)} JS chars, no syntax/API mismatch')
