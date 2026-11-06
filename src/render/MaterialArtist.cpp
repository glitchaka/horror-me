#include "horror/MaterialArtist.hpp"
#include <algorithm>
#include <cmath>

namespace horror {
namespace {
double fract(double x){return x-std::floor(x);}
double clamp01(double v){return std::clamp(v,0.0,1.0);}
int sat(double a){return std::clamp(int(a),0,255);}
}
Color MaterialArtist::tint(int r,int g,int b,int act,double light) {
    const double hostility=clamp01(double(act)/5.0);
    const double red=hostility*.19;
    return rgb(sat((r+red*42)*light),
               sat((g*(1-hostility*.13))*light),
               sat((b*(1-hostility*.23))*light));
}
double MaterialArtist::lighting(int x,int y,double time,double pressure,int act) {
    const unsigned seed=noiseHash(x/3,y/3,47);
    const bool emergency=(seed%13u)==0u || (act>=3&&seed%7u==0u);
    double base=.78+(seed%9u)*.025;
    if(emergency) {
        const double pulse=.52+.48*std::sin(time*(3.7+seed%4u)*2.3);
        base=.62+.35*pulse;
    }
    if(pressure>65&&seed%11u==0u)
        base*=.60+.40*std::sin(time*31.0+double(seed%37u));
    return std::max(.18,base);
}
Color MaterialArtist::ceiling(Vec2 world,int act,double time,double pressure) const {
    const int x=cell(world.x),y=cell(world.y);
    const double u=fract(world.x),v=fract(world.y);
    const unsigned seed=noiseHash(x,y,503);
    const bool seam=u<.035||v<.035;
    const bool beam=u>.90&&u<.96;
    const bool vent=seed%17u==0u&&u>.18&&u<.80&&v>.12&&v<.82;
    const bool strip=(x+y*3)%13==0&&u>.18&&u<.82&&v>.38&&v<.52;
    double intensity=lighting(x,y,time,pressure,act);
    if(strip) {
        const double flicker=.86+.14*std::sin(time*4.3+seed%100u);
        return tint(162,194,174,act,intensity*(1.2+flicker*.7));
    }
    if(vent) {
        const bool grille=int(u*38)%5==0||int(v*38)%6==0;
        return tint(grille?60:18,grille?74:27,grille?77:30,act,intensity);
    }
    int base=seam?12:beam?25:42;
    const int variation=int(seed%13u)-6;
    base+=variation;
    if(noiseHash(x*31+int(u*40),y*7+int(v*40),9)%71u<2u)
        base=11;
    return tint(base,base+5,base+8,act,intensity);
}
Color MaterialArtist::floor(Vec2 world,int act,double time,double pressure) const {
    const int x=cell(world.x),y=cell(world.y);
    const double u=fract(world.x),v=fract(world.y);
    const unsigned seed=noiseHash(x,y,83);
    const double fine=double(noiseHash(int(world.x*75),int(world.y*75),91)%37u)/7.0;
    const double seam=std::min({u,1-u,v,1-v});
    const bool strip=(seed%13u)==0u&&v>.46&&v<.51;
    const bool puddle=(seed%19u)==0u&&std::hypot((u-.44)*.9,v-.55)<.31;
    double light=lighting(x,y,time,pressure,act)*.81;
    if(puddle) {
        const double shimmer=2.5*std::sin(time*2.1+u*21+v*13);
        return tint(sat(31+shimmer),sat(51+shimmer),sat(55+shimmer),act,light);
    }
    if(strip) {
        const bool hazard=(int(u*16)+int(v*16))%5<=1;
        return tint(hazard?104:25,hazard?78:26,hazard?24:30,act,light);
    }
    if(seam<.035)
        return tint(11,17,19,act,light);
    if(seed%23u==0u&&(int(u*42)==int(v*42)||int(u*50)==int((1-v)*50)))
        return tint(17,21,23,act,light);
    const int base=26+int(seed%19u)+sat(fine);
    return tint(base+3,base+7,base+8,act,light);
}
Color MaterialArtist::wall(int x,int y,double u,double v,int face,
                           double depth,int act,double time,double pressure) const {
    const unsigned seed=noiseHash(x,y,172);
    const int material=int(seed%4u);
    const double crack=noiseHash(x*41+int(u*97),y*37+int(v*113),31)%67u;
    double lit=lighting(x,y,time,pressure,act);
    const double haze=std::exp(-std::max(0.0,depth)*(.125+.035*act/5.0));
    lit*=haze*(face==0?1.0:.69);
    if(u<.014||u>.988)lit*=.45;
    if(v>.905)lit*=.63;
    if(v<.085)lit*=.69;
    int r=54,g=65,b=68;

    if(material==0) {
        // Institutional ceramic cladding; fractured grout and stains.
        const double groutU=fract(u*3),groutV=fract(v*4);
        const bool grout=groutU<.026||groutV<.025;
        r=grout?19:79;g=grout?25:90;b=grout?31:88;
        const double water=double(noiseHash(x*9+int(u*79),y*21+int(v*77),44)%100u);
        if(v>.55&&water<17.0){r-=24;g-=29;b-=27;}
        if(crack<2){r=16;g=20;b=24;}
    } else if(material==1) {
        // Oxidized, bolted steel partitions.
        r=63;g=67;b=63;
        if(fract(u*2)<.035||fract(v*3)<.024){r=19;g=24;b=25;}
        if((std::abs(fract(u*2)-.06)<.018||std::abs(fract(u*2)-.94)<.018)&&
           (std::abs(fract(v*3)-.06)<.019||std::abs(fract(v*3)-.93)<.019)) {
            r=143;g=144;b=128;
        }
        if(noiseHash(x*19+int(u*59),y*31+int(v*68),99)%37u<4u) {
            r+=32;g-=10;b-=19;
        }
        if(v>.83&&v<.89&&u>.04&&u<.96){r=113;g=91;b=38;}
    } else if(material==2) {
        // Wet concrete with exposed reinforcement and maintenance markings.
        const int stain=int(noiseHash(x*9+int(u*34),y*11+int(v*47),94)%41u);
        r=47+stain/2;g=54+stain/2;b=57+stain/2;
        if(noiseHash(x*11+int(u*83),y*23+int(v*87),5)%89u<2u)
            r=g=b=12;
        if(u>.10&&u<.16&&v>.10&&v<.86&&seed%3u==0u){r=92;g=39;b=35;}
        if(fract(v*6)<.012){r-=16;g-=19;b-=18;}
    } else {
        // Archive cabinets, vents and numbered panels.
        const bool cabinet=u>.10&&u<.88&&v>.14&&v<.90;
        const bool frame=cabinet&&(u<.14||u>.85||v<.18||v>.84);
        if(cabinet){r=36;g=48;b=54;}
        if(frame){r=100;g=109;b=107;}
        if(cabinet&&v>.36&&v<.43&&u>.20&&u<.80){r=93;g=88;b=65;}
        if(cabinet&&v>.60&&v<.67&&u>.20&&u<.80){r=102;g=96;b=69;}
        if(cabinet&&u>.45&&u<.55&&v>.48&&v<.55){r=158;g=163;b=132;}
    }

    // Emergency lamps embedded into the architecture; not random full-screen red.
    if(seed%11u==0u&&u>.39&&u<.61&&v>.07&&v<.125) {
        const bool alarm=(act>=3||pressure>70);
        const double power=1.0+.55*std::sin(time*(alarm?8.0:2.4));
        return tint(alarm?sat(193*power):sat(117*power),
                    alarm?28:sat(187*power),alarm?31:sat(153*power),act,lit*1.6);
    }
    // Number plate and caution bands on some sectors.
    if(seed%7u==0u&&u>.16&&u<.66&&v>.30&&v<.39) {
        const bool stripe=int((u+v)*36)%4<=1;
        r=stripe?131:24;g=stripe?111:27;b=stripe?45:27;
    }
    if(seed%5u==0u&&u>.77&&u<.83&&v>.17&&v<.84) {
        r=18;g=25;b=24;
        if(int(v*50)%7==0){r=119;g=94;b=56;}
    }
    return tint(r,g,b,act,lit);
}
}
