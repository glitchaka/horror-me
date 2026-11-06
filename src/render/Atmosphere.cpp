#include "horror/Atmosphere.hpp"
#include <algorithm>
#include <cmath>

namespace horror {
namespace {
int clampColor(double v){return std::clamp(int(v),0,255);}
}
void Atmosphere::apply(Canvas& canvas,int act,double pressure,double time,double shake) const {
    Color* pixels=canvas.mutableData();
    const double stress=std::clamp(pressure/100.0,0.0,1.0);
    const int tick=int(time*37);
    // Frame vignette, camera noise, local interference and pressure-induced color shift.
    const bool emergency=act>=3;
    const int glitchY=int(noiseHash(tick/2,act,87)%ScreenH);
    for(int y=0;y<ScreenH;++y) {
        const double vertical=std::abs((y+0.5)-ScreenH*.50)/(ScreenH*.50);
        const bool glitch=stress>.55&&std::abs(y-glitchY)<int(2+stress*5);
        for(int x=0;x<ScreenW;++x) {
            const int i=y*ScreenW+x;
            Color c=pixels[i];
            const double horizontal=std::abs((x+.5)-ScreenW*.5)/(ScreenW*.5);
            const double edge=std::pow(std::min(1.0,std::hypot(horizontal*.85,vertical*.86)),1.6);
            double fade=1.0-.48*edge-.13*stress*edge;
            const unsigned hash=noiseHash(x/2+tick*3,y/2+tick,27);
            const int grain=int(hash%9u)-4;
            const int scan=(y%3==0)?-2:0;
            int red=int((c>>16)&255),green=int((c>>8)&255),blue=int(c&255);
            const int addition=int(grain*(.3+stress*1.3))+scan;
            red=clampColor(red*fade+addition+(emergency?stress*8:0));
            green=clampColor(green*fade+addition);
            blue=clampColor(blue*fade+addition*.7);
            if(glitch&&((x/13+tick)%3==0)){
                red=clampColor(red+24*stress+shake*16);
                green=clampColor(green*.64);
                blue=clampColor(blue*.73);
            }
            if(stress>.8&&hash%200u<2u){
                red=clampColor(red+48*stress);
                green=clampColor(green+15);
            }
            pixels[i]=rgb(red,green,blue);
        }
    }
}
}
