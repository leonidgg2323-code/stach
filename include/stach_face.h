#pragma once
#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include "stach_core.h"

// Flat, non-glowing, anti-aliased face. Internal resolution 466x466.
// White eyes; black pupils. Grey shades appear on SHAPE EDGES only for AA.
namespace stach { namespace face {
constexpr int WIDTH = 466, HEIGHT = 466;
// Eye PAIR never leaves the central visual area of the circular 466px display.
// Individual pupils still shift and the eyes change relative size for perspective.
constexpr float MAX_EYE_SHIFT_X = 20.f;
constexpr float MAX_EYE_SHIFT_Y = 12.f;

struct State {
  float gazeX=0, gazeY=0, openness=1;
  float offsetX=0, offsetY=0;
  float scale=1;
  float browLift=0, browTilt=0, browStrength=0;
  float browArc=0, browAsymmetry=0;
  float chargeStrength=0, chargePhase=0;
};
inline float clamp(float x,float a,float b){return x<a?a:(x>b?b:x);}
inline float ease(float x){x=clamp(x,0,1);return x*x*(3-2*x);}
inline float interpolate(float t,float t0,float t1,float from,float to){
  return from+(to-from)*ease((t-t0)/(t1-t0));
}

// Motion has a reason: idle -> brief anticipation -> glance -> hold -> return.
// This does NOT change emotion spontaneously.
inline State evaluate(uint32_t ms, Emotion emotion){
  State s;
  const float t=fmodf(float(ms)/1000.0f,8.0f);
  float direction=0, position=0, anticipation=0;
  if(t>=1.50f && t<1.74f) anticipation=interpolate(t,1.50f,1.74f,0,1);
  else if(t>=1.74f && t<2.20f) {direction=-1;position=interpolate(t,1.74f,2.20f,0,1);}
  else if(t>=2.20f && t<3.20f) {direction=-1;position=1;}
  else if(t>=3.20f && t<3.70f) {direction=-1;position=interpolate(t,3.20f,3.70f,1,0);}
  else if(t>=4.50f && t<4.74f) anticipation=-interpolate(t,4.50f,4.74f,0,1);
  else if(t>=4.74f && t<5.20f) {direction=1;position=interpolate(t,4.74f,5.20f,0,1);}
  else if(t>=5.20f && t<6.20f) {direction=1;position=1;}
  else if(t>=6.20f && t<6.70f) {direction=1;position=interpolate(t,6.20f,6.70f,1,0);}

  s.gazeX=direction*position*.88f;
  s.offsetX=direction*position*18.0f + anticipation*2.0f;
  s.offsetY=position*3.0f;
  s.scale=1-0.04f*fabsf(anticipation);

  // A small purposeful blink after returning to centre.
  float blink=fabsf(t-6.92f);
  if(blink<.095f) s.openness=clamp(blink/.095f,.12f,1);
  switch(emotion){
    case Emotion::Neutral: break;
    case Emotion::Angry:
      s.openness*=.75f;s.browStrength=1;s.browLift=-9;s.browTilt=.96f;s.browArc=-2;break;
    case Emotion::Sleepy:
      s.openness*=.43f;s.browStrength=.70f;s.browLift=-5;s.browArc=-1;break;
    case Emotion::Curious:
      s.browStrength=1;s.browLift=8;s.browAsymmetry=15;s.browArc=4;break;
    case Emotion::Happy:
      s.openness*=.72f;s.browStrength=.90f;s.browLift=10;s.browArc=8;break;
    default: break;
  }
  s.chargePhase=fmodf(float(ms)/1900.0f,1.0f);
  return s;
}

// RGB565 greyscale: antialias only along the contour, NO screen glow/halo.
inline uint16_t grey(float brightness){
  int v=int(clamp(brightness,0,1)*255.0f+.5f);
  return uint16_t(((v>>3)<<11)|((v>>2)<<5)|(v>>3));
}
// Each scanline has a solid center run and just a few blended contour pixels.
// Four vertically supersampled slices avoid staircase-shaped curves at native size.
template<class Surface> void ellipseAA(Surface& surface,float cx,float cy,float rx,float ry,bool white){
  if(rx<=0||ry<=0)return;
  const int ymin=int(floorf(cy-ry-1)),ymax=int(ceilf(cy+ry+1));
  for(int y=ymin;y<=ymax;++y){
    if(y<0||y>=HEIGHT)continue;
    float L[4],R[4],minL=1e6f,maxL=-1e6f,maxEdgeL=-1e6f,minEdgeR=1e6f;
    bool any=false,all=true;
    for(int k=0;k<4;++k){
      float yy=(y+(k+.5f)/4.0f-cy)/ry;
      if(fabsf(yy)>=1) {L[k]=1e6f;R[k]=-1e6f;all=false;continue;}
      float dx=rx*sqrtf(1-yy*yy);L[k]=cx-dx;R[k]=cx+dx;
      if(L[k]<minL)minL=L[k];
      if(R[k]>maxL)maxL=R[k];
      if(L[k]>maxEdgeL)maxEdgeL=L[k];
      if(R[k]<minEdgeR)minEdgeR=R[k];
      any=true;
    }
    if(!any)continue;
    const int first=int(floorf(minL-.5f));
    const int last=int(ceilf(maxL+.5f));
    // Pixels definitely fully covered in every vertical sample.
    int fullFirst=all?int(ceilf(maxEdgeL+.5f)):last+1;
    int fullLast=all?int(floorf(minEdgeR-.5f)):first-1;
    if(fullFirst<=fullLast){
      int x=fullFirst,xend=fullLast;
      if(x<0)x=0;
      if(xend>=WIDTH)xend=WIDTH-1;
      if(x<=xend)surface.hline(x,y,xend-x+1,white?0xffff:0);
    }
    // Supersampling only contour pixels, never fill whole frame pixel-by-pixel.
    for(int x=first;x<=last;++x){
      if(x<0||x>=WIDTH||(x>=fullFirst && x<=fullLast))continue;
      float coverage=0;
      for(int k=0;k<4;++k){
        if(L[k]>R[k])continue;
        coverage+=clamp(fminf(x+.5f,R[k])-fmaxf(x-.5f,L[k]),0,1)*.25f;
      }
      if(coverage<=0)continue;
      // white-on-black or black-on-white with correct antialiased edge.
      uint16_t color=grey(white?coverage:1.0f-coverage);
      surface.hline(x,y,1,color);
    }
  }
}

template<class Surface> void stroke(Surface& surface,int x1,int y1,int x2,int y2,int thick,uint16_t color){
  const int dx=x2-x1,dy=y2-y1,steps=abs(dx)>abs(dy)?abs(dx):abs(dy);
  for(int i=0;i<=steps;++i){
    const int x=x1+(steps?dx*i/steps:0),y=y1+(steps?dy*i/steps:0);
    for(int yy=y-thick/2;yy<=y+thick/2;++yy)
      if(x>=0&&x<WIDTH&&yy>=0&&yy<HEIGHT)surface.hline(x,yy,1,color);
  }
}

template<class Surface> void draw(Surface& surface,const State& st){
  surface.clear(0);
  const float sc=clamp(st.scale,.83f,1.08f);
  // Final safety clamp: applies even to malformed sensor values / future modules.
  const float eyeOffsetX=clamp(st.offsetX,-MAX_EYE_SHIFT_X,MAX_EYE_SHIFT_X);
  const float eyeOffsetY=clamp(st.offsetY,-MAX_EYE_SHIFT_Y,MAX_EYE_SHIFT_Y);
  const float gaze=clamp(st.gazeX,-1,1);
  for(int i=0;i<2;++i){
    // Perspective: frontal eyes have the SAME proportions.
    // Left glance => LEFT smaller; right glance => RIGHT smaller.
    float perspective=i==0?(1+.17f*gaze):(1-.17f*gaze);
    const float rx=55*sc*perspective;
    const float ry=73*sc*perspective*clamp(st.openness,.025f,1);
    const float cx=(i?303:163)+eyeOffsetX,cy=229+eyeOffsetY;
    ellipseAA(surface,cx,cy,rx,ry,true);
    if(ry>10){
      float px=cx+clamp(gaze*15*sc,-rx*.30f,rx*.30f);
      float py=cy+clamp(st.gazeY,-1,1)*11*sc;
      float prx=23*sc*perspective;
      float pry=fminf(ry*.72f,39*sc*perspective);
      ellipseAA(surface,px,py,prx,pry,false);
    }
    if(st.browStrength>.01f){
      int mx=int(cx),by=int(cy-ry-29-st.browLift + (i?-1:1)*st.browAsymmetry*.5f);
      int half=int(37*sc*perspective),tilt=int(st.browTilt*(i?-1:1)*14);
      int arc=int(st.browArc),thick=2+int(3*clamp(st.browStrength,0,1));
      stroke(surface,mx-half,by-tilt,mx,by-arc,thick,0xffff);
      stroke(surface,mx,by-arc,mx+half,by+tilt,thick,0xffff);
    }
  }
  // Charging: thin, FLAT green sweep around the inner perimeter + battery.
  // Nothing glows; charging indicator is absent except on a real charging event.
  if(st.chargeStrength>.01f){
    const float p=clamp(st.chargeStrength,0,1);
    const uint16_t green=0x07E0;
    // Continuous, thin charge-progress sweep. No particles, bloom or glow.
    const float start=-2.0f+st.chargePhase*6.2831853f;
    int oldX=0,oldY=0;
    for(int n=0;n<=100;++n){
      float angle=start+1.12f*float(n)/100.0f;
      int x=int(233+197*cosf(angle)),y=int(233+197*sinf(angle));
      if(n>0)stroke(surface,oldX,oldY,x,y,2,green);
      oldX=x;oldY=y;
    }
    // Top battery pictogram. Outline white, fill green; show only while charging.
    const int x0=210,y0=35,w=45,h=18;
    for(int y=y0;y<y0+h;++y){
      surface.hline(x0,y,w,(y==y0||y==y0+h-1)?0xffff:0);
      surface.hline(x0,y,1,0xffff);surface.hline(x0+w-1,y,1,0xffff);
    }
    surface.hline(x0+w,y0+6,3,0xffff);surface.hline(x0+w,y0+7,3,0xffff);
    int fill=int((w-6)*(.32f+.62f*st.chargePhase)*p);
    for(int y=y0+3;y<y0+h-3;++y)if(fill>0)surface.hline(x0+3,y,fill,green);
  }
}
}} // namespace stach::face
