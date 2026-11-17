#!/usr/bin/env python3
"""Real Chromium JS flow using a mocked fetch transport (not real ESP32 HTTP)."""
from pathlib import Path
from playwright.sync_api import sync_playwright
root=Path(__file__).resolve().parents[1]
html=(root/'web/stach-panel.html').read_text(encoding='utf-8')
mock_fetch=r"""
window.__stachRequests=[];
window.fetch=async (url, options={})=>{
  const rec={url:String(url),method:options.method||'GET',headers:options.headers||{}};
  window.__stachRequests.push(rec);
  const headers={'Content-Type':'application/json'};
  if(rec.url==='/api/session')return new Response(JSON.stringify({token:'A1'.repeat(16)}),{status:200,headers});
  if(rec.method==='POST'){
    const valid=rec.headers['X-Stach-Token']==='A1'.repeat(16);
    return new Response(JSON.stringify({ok:valid}),{status:valid?200:403,headers});
  }
  if(rec.url==='/api/status')return new Response(JSON.stringify({name:'Stach',expression:'neutral',deviceMode:'face'}),{status:200,headers});
  return new Response('{}',{status:200,headers});
};
"""
with sync_playwright() as p:
    browser=p.chromium.launch(executable_path='/usr/bin/chromium',headless=True,args=['--no-sandbox'])
    for width,height in [(390,844),(1280,900)]:
        page=browser.new_page(viewport={'width':width,'height':height})
        errors=[];page.on('pageerror',lambda e:errors.append(str(e)))
        page.evaluate(mock_fetch)
        page.set_content(html,wait_until='load')
        page.locator('[data-emotion="angry"]').click()
        page.wait_for_function("document.getElementById('activeEmotion').textContent.includes('Злой')")
        page.locator('[data-mode="portal"]').click()
        page.wait_for_function("document.getElementById('pairingLabel').textContent.includes('QR панели')")
        requests=page.evaluate('window.__stachRequests')
        posts=[r for r in requests if r['method']=='POST']
        assert len(posts)>=2,posts
        assert len([r for r in requests if r['url']=='/api/session'])==1, requests
        assert all(r['headers'].get('X-Stach-Token')=='A1'*16 for r in posts),posts
        assert not errors,errors
        print(f'Chromium mocked-transport test OK {width}×{height}: {len(posts)} signed POSTs, one session fetch')
        page.close()
    browser.close()
