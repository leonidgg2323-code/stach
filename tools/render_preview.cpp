#include "stach_face.h"
#include <cstdint>
#include <cstdio>
#include <vector>
#include <algorithm>
#include <cstdlib>
struct Surface {
 std::vector<uint16_t> pix;
 Surface():pix(466*466,0){}
 void clear(uint16_t c){std::fill(pix.begin(),pix.end(),c);}
 void hline(int x,int y,int w,uint16_t c){
   if(x<0||y<0||x+w>466||y>=466||w<=0)abort();
   std::fill(pix.begin()+y*466+x,pix.begin()+y*466+x+w,c);
 }
};
int main(){
 Surface s;
 // 8 sec directional motion, then 2 sec angry, 2 sec sleepy, then 4 sec charging.
 const int frames=16*18;
 for(int frame=0;frame<frames;++frame){
  int ms=frame*1000/18;
  stach::Emotion e=stach::Emotion::Neutral;
  if(ms>=8000 && ms<10000)e=stach::Emotion::Angry;
  if(ms>=10000&&ms<12000)e=stach::Emotion::Sleepy;
  if(ms>=12000)e=stach::Emotion::Happy;
  auto state=stach::face::evaluate(uint32_t(ms),e);
  if(e==stach::Emotion::Happy)state.chargeStrength=1;
  stach::face::draw(s,state);
  printf("P6\n466 466\n255\n");
  for(uint16_t c:s.pix){
    unsigned char rgb[]={static_cast<unsigned char>((c>>11&31)*255/31),static_cast<unsigned char>((c>>5&63)*255/63),static_cast<unsigned char>((c&31)*255/31)};
    fwrite(rgb,1,3,stdout);
  }
 }
}
