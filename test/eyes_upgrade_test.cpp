#include "stach_character.h"
#include <cassert>
#include <cmath>
#include <initializer_list>
struct Surface {int white=0,black=0;void clear(uint16_t c){assert(c==0);}void hline(int x,int y,int w,uint16_t c){assert(x>=0&&x+w<=466&&y>=0&&y<466&&w>0);if(c)++white;else ++black;}};
int main(){using namespace stach;
 auto n=face::evaluate(900,Emotion::Neutral),a1=face::evaluate(900,Emotion::Angry);
 assert(n.browStrength==0 && a1.browStrength>0);
 auto a=face::evaluate(800,Emotion::Neutral),b=face::evaluate(2600,Emotion::Neutral);
 assert(fabs(a.offsetX-b.offsetX)>5 && fabs(a.offsetY-b.offsetY)>2);
 for(auto e:{Emotion::Neutral,Emotion::Angry,Emotion::Sleepy,Emotion::Curious,Emotion::Happy})
 for(uint32_t t=0;t<30000;t+=113){auto st=face::evaluate(t,e);Surface s;face::draw(s,st);assert(s.white>0);}
 character::Controller c;c.set(Emotion::Curious,1200);auto pre=c.sample(1311);c.set(Emotion::Angry,1311);auto post=c.sample(1311);
 assert(fabs(pre.offsetX-post.offsetX)<.0001f && fabs(pre.scale-post.scale)<.0001f && fabs(pre.browLift-post.browLift)<.0001f);
}
