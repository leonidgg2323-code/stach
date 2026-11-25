#pragma once
namespace stach { namespace owner {
static const char loginPage[] PROGMEM=R"OWNER(<!doctype html>
<html lang="ru"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>STACH · Аккаунт</title>
<style>*{box-sizing:border-box}body{margin:0;background:#11151c;color:#eef3fc;font:16px system-ui;min-height:100svh;display:grid;place-items:center;padding:20px}main{width:min(100%,390px)}h1{font-size:30px}p{color:#b5c1cf;line-height:1.5}label{display:block;margin:16px 0}input,button{display:block;width:100%;font:inherit;border-radius:10px;padding:12px;margin-top:5px}input{background:#1b2029;color:white;border:1px solid #627181}button{border:none;background:#9ef2d0;color:#122820;font-weight:650}small{color:#9be6c8}#msg{min-height:1.5em}</style>
<main><small>STACH</small><h1 id="title"></h1><p id="help"></p>
<form id="account"><label>Логин<input id="user" required autocomplete="username" minlength="3" maxlength="32" pattern="[A-Za-z0-9_\-]{3,32}"></label>
<label>Пароль<input id="pass" type="password" required minlength="12" maxlength="96"></label>
<button id="action"></button></form><p id="msg" role="status"></p></main>
<script>
const signup=__SIGNUP__, $=id=>document.getElementById(id);
const claim=new URLSearchParams(location.hash.slice(1)).get('claim')||'';
if(location.hash)history.replaceState(null,'',location.pathname);
$('title').textContent=signup?'Аккаунт владельца':'Вход в панель';
$('help').textContent=signup?'Придумай логин из 3–32 латинских букв, цифр, дефисов или подчёркиваний и пароль от 12 символов. Сохрани их для следующего входа.':'Введи логин и пароль владельца этого STACH.';
$('pass').autocomplete=signup?'new-password':'current-password';
$('action').textContent=signup?'Создать аккаунт':'Войти';
if(signup && !claim){$('account').hidden=true;$('help').textContent='Подключись к Wi-Fi STACH. Отсканируй код «2/2 REGISTER» на экране устройства. Если виден первый код, коснись дисплея один раз.';}
$('account').onsubmit=async e=>{
 e.preventDefault();$('action').disabled=true;$('msg').textContent='Подождите…';
 try{
  const body=new URLSearchParams({login:$('user').value,password:$('pass').value,claim});
  const r=await fetch(signup?'/account/register':'/account/login',{method:'POST',headers:{'X-Stach-Account':'1'},body,credentials:'same-origin'});
  if(r.ok){location.replace('/');return}
  $('msg').textContent=({400:'Проверь формат логина и длину пароля (12–96 байт).',403:'Проверь логин и пароль. Для регистрации отсканируй актуальный QR-код.',409:'Состояние аккаунта изменилось. Открой панель заново.',429:'Слишком много попыток. Подожди полминуты.'})[r.status]||'Не удалось сохранить или открыть аккаунт.';
 }catch{ $('msg').textContent='Нет связи. Проверь подключение к Wi-Fi STACH.' }
 $('action').disabled=false;
};
</script></html>)OWNER";
}}
