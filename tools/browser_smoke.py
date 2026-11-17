#!/usr/bin/env python3
"""Optional local real-Chromium interaction checks, not needed by GitHub Actions."""
from pathlib import Path
from playwright.sync_api import sync_playwright
root=Path(__file__).resolve().parents[1]
html=(root/'web/stach-panel.html').read_text(encoding='utf-8').replace("const demo = location.protocol === 'file:';","const demo = true;")
with sync_playwright() as p:
    browser=p.chromium.launch(headless=True,executable_path='/usr/bin/chromium',args=['--no-sandbox'])
    for size in [(390,844),(1280,900)]:
        page=browser.new_page(viewport={'width':size[0],'height':size[1]},device_scale_factor=1)
        failures=[]
        page.on('pageerror',lambda exc: failures.append(str(exc)))
        page.set_content(html, wait_until="load")
        page.wait_for_timeout(300)
        assert 'Демо без устройства' in page.locator('#connectionText').inner_text()
        assert not page.locator('#screen .brow.left').evaluate('(el) => getComputedStyle(el).opacity')=='1'
        page.locator('[data-emotion="angry"]').click()
        page.wait_for_function("document.getElementById('screen').classList.contains('angry')")
        page.locator('[data-pairing="wifi"]').click()
        page.wait_for_function("document.getElementById('pairingLabel').textContent.includes('Wi-Fi QR')")
        page.locator('#idleMotion').evaluate('(el) => el.click()')
        assert not page.locator('#idleMotion').is_checked()
        assert page.locator('#batteryPower').inner_text() == 'Не определена'
        page.locator('#powerSaver').evaluate('(el) => el.click()')
        assert not page.locator('#powerSaver').is_checked()
        page.locator('#wakeScreen').click()
        assert page.locator('#screenMode').inner_text() == 'Активный'
        page.locator('#powerPanel details summary').click()
        page.locator('#calibrationMv').fill('3900')
        page.locator('#calibrateBattery').click()
        assert page.locator('#batteryPower').inner_text() == '63%'
        page.locator('[data-demo="charging"]').click()
        page.wait_for_function("document.getElementById('screen').classList.contains('charging')")
        joy=page.locator('#joystick');joy.scroll_into_view_if_needed();r=joy.bounding_box()
        page.mouse.move(r['x']+r['width']/2,r['y']+r['height']/2)
        page.mouse.down();page.mouse.move(r['x']+r['width']*.80,r['y']+r['height']*.3)
        assert page.locator('#knob').evaluate('(el) => parseFloat(el.style.left)')>50
        page.mouse.up();assert page.locator('#knob').evaluate('(el) => parseFloat(el.style.left)')==50
        if size[0]<500:
            assert page.evaluate('document.documentElement.scrollWidth <= innerWidth+1'), 'Horizontal overflow mobile'
        path=root/'preview'/f'panel-{size[0]}px.png'
        page.screenshot(path=str(path),full_page=True)
        assert not failures,failures
        print(f'Chromium smoke OK for {size[0]}×{size[1]}, screenshot {path.name}')
        page.close()
    browser.close()
