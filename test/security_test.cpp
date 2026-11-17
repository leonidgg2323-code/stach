#include "stach_security.h"
#include <cassert>
#include <iostream>
#include <cstring>
#include <stdint.h>
int main() {
  using namespace stach::security;
  int checked=0;
  auto ck=[&](bool b){assert(b);++checked;};
  ck(allowedHost("192.168.4.1"));ck(allowedHost("192.168.4.1:80"));
  for(const char* h: {"","evil.com","192.168.4.1.evil.com","192.168.4.2","192.168.4.1:81","192.168.4.1@evil.com","[192.168.4.1]"})ck(!allowedHost(h));
  ck(!allowedHost(nullptr));
  ck(allowedOrigin(nullptr));ck(allowedOrigin(""));ck(allowedOrigin("http://192.168.4.1"));ck(allowedOrigin("http://192.168.4.1:80"));
  for(const char* h: {"null","http://evil.com","https://192.168.4.1","http://192.168.4.1.evil.com","http://192.168.4.1:81","file://"})ck(!allowedOrigin(h));
  ck(allowedFetchSite("same-origin"));ck(allowedFetchSite("none"));ck(allowedFetchSite(""));ck(allowedFetchSite(nullptr));ck(!allowedFetchSite("cross-site"));ck(!allowedFetchSite("same-site"));
  uint8_t bytes[16]{};for(int i=0;i<16;++i)bytes[i]=i*15;
  char token[33];encodeToken(bytes,token);ck(strlen(token)==32);ck(!strcmp(token,"000F1E2D3C4B5A69788796A5B4C3D2E1"));
  ck(equalToken(token,token));ck(!equalToken(token,nullptr));ck(!equalToken(nullptr,token));ck(!equalToken(token,""));
  char bad[33];strcpy(bad,token);bad[31]='0'==bad[31]?'1':'0';ck(!equalToken(token,bad));
  for(int i=0;i<32;i++){strcpy(bad,token);bad[i]='0'==bad[i]?'1':'0';ck(!equalToken(token,bad));}
  RateGate limiter;for(int i=0;i<30;i++)ck(limiter.allow(0x01020304,1000));
  ck(!limiter.allow(0x01020304,1000));
  ck(limiter.allow(0x01020304,1040));
  ck(!limiter.allow(0x01020304,1040));
  for(int i=0;i<30;i++)ck(limiter.allow(0x05060708,1040));
  ck(!limiter.allow(0x05060708,1040));
  for(int i=0;i<30;i++)ck(limiter.allow(0x01020304,11000));
  ck(!limiter.allow(0x01020304,11000));
  RateGate overflow;for(int i=0;i<30;i++)ck(overflow.allow(7,0xfffffff0u));
  ck(!overflow.allow(7,0xfffffff0u));
  ck(overflow.allow(7,0x00000018u)); // 40 ms across wrap
  for(int i=0;i<100;i++)ck(!overflow.allow(7,0x00000018u));
  RateGate guests;for(unsigned i=1;i<=12;i++)ck(guests.allow(i,1000+i));
  std::cout<<"Stage-11 security test passed: "<<checked<<" assertions\n";
}
