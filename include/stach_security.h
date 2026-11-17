#pragma once
#include <stdint.h>
#include <stddef.h>
#include <string.h>

// No Arduino dependencies: security rules are portable and host-testable.
// The per-BOOT token stops browser CSRF; it is not a password for other
// devices that already know the Wi-Fi WPA2 passphrase.
namespace stach { namespace security {
inline bool allowedHost(const char* host) {
  return host && (!strcmp(host, "192.168.4.1") || !strcmp(host, "192.168.4.1:80"));
}
inline bool allowedOrigin(const char* origin) {
  return !origin || !*origin || !strcmp(origin, "http://192.168.4.1") ||
         !strcmp(origin, "http://192.168.4.1:80");
}
inline bool allowedFetchSite(const char* site) {
  return !site || !*site || !strcmp(site, "same-origin") || !strcmp(site, "none");
}
inline bool equalToken(const char* expected, const char* supplied) {
  if (!expected || !supplied || strlen(expected)!=32 || strlen(supplied)!=32) return false;
  unsigned mismatch=0;
  for (size_t i=0;i<32;i++) mismatch |= unsigned(uint8_t(expected[i])^uint8_t(supplied[i]));
  return mismatch==0;
}
inline void encodeToken(const uint8_t (&bytes)[16], char (&out)[33]) {
  static const char* hex="0123456789ABCDEF";
  for (unsigned i=0;i<16;i++){out[i*2]=hex[bytes[i]>>4];out[i*2+1]=hex[bytes[i]&15];}
  out[32]='\0';
}

// Fixed memory; handles tick wrap. Upper limit 4 AP clients, one slot/client.
struct RateGate {
  struct Slot { uint32_t ip=0,last=0,seen=0;uint8_t tokens=30;bool used=false;};
  Slot slots[4]{};
  bool allow(uint32_t ip,uint32_t now){
    Slot* chosen=nullptr;
    for(auto &s: slots) if(s.used&&s.ip==ip){chosen=&s;break;}
    if(!chosen){
      for(auto &s: slots)if(!s.used){chosen=&s;break;}
      if(!chosen){chosen=&slots[0];for(auto &s: slots)
        if(uint32_t(now-s.seen)>uint32_t(now-chosen->seen))chosen=&s;}
      *chosen=Slot{}; chosen->used=true;chosen->ip=ip;chosen->last=now;
    }
    uint32_t elapsed=uint32_t(now-chosen->last);
    // 25 tokens per second, burst 30. Bound refill calculation.
    if(elapsed>=40){
      const uint32_t extra=elapsed/40;
      chosen->tokens=uint8_t(extra>=30 || extra+chosen->tokens>=30 ? 30 : chosen->tokens+extra);
      chosen->last=now-(elapsed%40);
    }
    chosen->seen=now;
    if(!chosen->tokens)return false;
    --chosen->tokens;
    return true;
  }
};
}} // namespace stach::security
