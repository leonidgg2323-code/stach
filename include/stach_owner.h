#pragma once
#include <Preferences.h>
#include <WebServer.h>
#include <esp_random.h>
#include <esp_timer.h>
#include <mbedtls/pkcs5.h>
#include <mbedtls/platform_util.h>
#include "stach_security.h"

namespace stach { namespace owner {
struct Credential {
  uint8_t version;
  char login[33];
  uint8_t salt[16], hash[32];
};
static Credential stored{};
static bool claimed=false, storageOK=false;
static char invitation[33]{}, session[33]{};
static int64_t expires=0, nextAttempt=0;
static unsigned failures=0;
static String deviceName="Stach";

inline void randomToken(char (&out)[33]) {
  uint8_t bytes[16];
  esp_fill_random(bytes,sizeof(bytes));
  security::encodeToken(bytes,out);
}
inline void begin(Preferences& prefs) {
  claimed=prefs.isKey("ownerV1");
  storageOK=!claimed || (prefs.getBytesLength("ownerV1")==sizeof(stored) &&
    prefs.getBytes("ownerV1",&stored,sizeof(stored))==sizeof(stored) &&
    stored.version==1 && memchr(stored.login,0,sizeof(stored.login)));
  if(!claimed) randomToken(invitation);
  deviceName=prefs.getString("displayName","Stach");
}
inline bool authorized(WebServer& web) {
  if(!claimed || !storageOK || !session[0] || esp_timer_get_time()>=expires) return false;
  String cookies=web.header("Cookie");
  int pos=0;
  while(pos<(int)cookies.length()) {
    int end=cookies.indexOf(';',pos);
    if(end<0) end=cookies.length();
    String item=cookies.substring(pos,end); item.trim();
    if(item.startsWith("stach_owner="))
      return security::equalToken(session,item.c_str()+12);
    pos=end+1;
  }
  return false;
}
inline void openSession(WebServer& web) {
  randomToken(session);
  expires=esp_timer_get_time()+12LL*60*60*1000000;
  web.sendHeader("Set-Cookie",String("stach_owner=")+session+"; Path=/; HttpOnly; SameSite=Strict; Max-Age=43200");
}
inline void logout(WebServer& web) {
  memset(session,0,sizeof(session));
  web.sendHeader("Set-Cookie","stach_owner=; Path=/; HttpOnly; SameSite=Strict; Max-Age=0");
}
inline bool validLogin(const String& value) {
  if(value.length()<3 || value.length()>32) return false;
  for(unsigned i=0;i<value.length();++i) {
    char c=value[i];
    if(!((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||c=='_'||c=='-')) return false;
  }
  return true;
}
inline bool validName(const String& name) {
  unsigned count=0;
  for(unsigned i=0;i<name.length();++i) {
    uint8_t c=name[i];
    if(c==0xd0 || c==0xd1) {
      if(++i>=name.length())return false;
      uint8_t next=name[i];
      if(next<0x80||next>0xbf)return false;
    } else if(!(isalnum(c) || c==' ' || c=='-' || c=='_')) return false;
    if(++count>24)return false;
  }
  return count>0;
}
inline bool derive(const String& pass,const uint8_t* salt,uint8_t* hash) {
  return mbedtls_pkcs5_pbkdf2_hmac_ext(MBEDTLS_MD_SHA256,
    reinterpret_cast<const uint8_t*>(pass.c_str()),pass.length(),salt,16,60000,32,hash)==0;
}
inline void authenticate(WebServer& web,Preferences& prefs,bool signup) {
  if(!storageOK) {web.send(503,"text/plain","Account storage unavailable");return;}
  if(signup==claimed) {web.send(409,"text/plain","Account state changed");return;}
  int64_t now=esp_timer_get_time();
  if(now<nextAttempt){web.send(429,"text/plain","Try again later");return;}
  String login=web.arg("login"), pass=web.arg("password");
  if(!validLogin(login)||pass.length()<12||pass.length()>96) {
    web.send(400,"text/plain","Invalid login or password length");return;
  }
  bool ok=false;
  if(signup) {
    if(security::equalToken(invitation,web.arg("claim").c_str())) {
      Credential record{};
      record.version=1;
      login.toCharArray(record.login,sizeof(record.login));
      esp_fill_random(record.salt,sizeof(record.salt));
      if(!derive(pass,record.salt,record.hash) ||
         prefs.putBytes("ownerV1",&record,sizeof(record))!=sizeof(record)) {
        web.send(503,"text/plain","Could not save account");return;
      }
      stored=record;claimed=true;
      memset(invitation,0,sizeof(invitation));
      ok=true;
    }
  } else {
    uint8_t result[32]{};
    if(derive(pass,stored.salt,result)) {
      unsigned diff=0;
      for(size_t i=0;i<sizeof(result);++i) diff|=result[i]^stored.hash[i];
      ok=diff==0 && login==stored.login;
    }
    mbedtls_platform_zeroize(result,sizeof(result));
  }
  if(!ok) {
    failures=min(5u,failures+1);
    nextAttempt=esp_timer_get_time()+(1LL<<failures)*1000000;
    web.send(403,"text/plain","Credentials or pairing code incorrect");return;
  }
  failures=0; nextAttempt=0;
  openSession(web);
  web.send(200,"application/json","{\"ok\":true}");
}
}}
