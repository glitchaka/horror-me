#include "horror/SceneRenderer.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

namespace horror {
namespace {
constexpr std::array<Color,4> TerminalColors{
    rgb(225,166,110),rgb(112,196,217),rgb(216,213,196),rgb(196,139,208)
};
}
void SceneRenderer::draw(Canvas& out,const GameSession& game) const {
    const Maze& maze=game.maze();
    const double glow=game.tension();
    const int horizon=ScreenH/2+int(std::sin(game.elapsed()*11)*game.shake()*5);
    for(int y=0;y<ScreenH;++y) for(int x=0;x<ScreenW;++x) {
        if(y<horizon) {
            const double z=double(y)/std::max(1,horizon);
            out.pixel(x,y,rgb(int(6+z*11+glow*10),int(9+z*9),int(15+z*14)));
        } else {
            const double z=double(y-horizon)/std::max(1,ScreenH-horizon);
            const int cracks=noiseHash(x/13,y/8,game.rewrites()*5)%37==0?6:0;
            out.pixel(x,y,rgb(int(9+z*12+glow*17+cracks),
                              int(10+z*9+cracks),int(13+z*9)));
        }
    }
    const Vec2 pos=game.player();
    const Vec2 direction{std::cos(game.heading()),std::sin(game.heading())};
    const Vec2 plane{-direction.y*FieldOfViewScale,direction.x*FieldOfViewScale};
    std::array<double,ScreenW> depths{};
    for(int x=0;x<ScreenW;++x) {
        const double camera=2.0*(x+.5)/ScreenW-1;
        const double rx=direction.x+plane.x*camera;
        const double ry=direction.y+plane.y*camera;
        int mx=cell(pos.x),my=cell(pos.y);
        const double dx=rx==0?1e25:std::abs(1/rx);
        const double dy=ry==0?1e25:std::abs(1/ry);
        const int stepX=rx<0?-1:1,stepY=ry<0?-1:1;
        double sx=(rx<0?pos.x-mx:mx+1.0-pos.x)*dx;
        double sy=(ry<0?pos.y-my:my+1.0-pos.y)*dy;
        int side=0;
        for(int n=0;n<80;++n) {
            if(sx<sy) {sx+=dx;mx+=stepX;side=0;}
            else {sy+=dy;my+=stepY;side=1;}
            if(maze.wall(mx,my))break;
        }
        const double depth=std::max(.08,side==0?sx-dx:sy-dy);
        depths[x]=depth;
        const int wallHeight=int(ScreenH/depth);
        const int top=std::max(0,horizon-wallHeight/2);
        const int bottom=std::min(ScreenH,horizon+wallHeight/2);
        double hit=(side==0?pos.y+depth*ry:pos.x+depth*rx);
        hit-=std::floor(hit);
        int tx=int(hit*32);
        if((side==0&&rx>0)||(side==1&&ry<0))tx=31-tx;
        const int salt=int(noiseHash(mx,my)%17);
        const double fade=std::clamp(1.0/(1.0+depth*.12),.20,1.0);
        for(int y=top;y<bottom;++y) {
            const int ty=int((double(y-horizon+wallHeight*.5)/std::max(1,wallHeight))*32);
            int lit=20+salt+((tx==0||tx==31)?-13:0);
            if(ty%16==0)lit-=8;
            if(noiseHash(mx*3+tx/5,my*5+ty/5)%29==0)lit+=15;
            if(ty>16&&ty<18&&tx>5&&tx<27&&noiseHash(mx,my)%6==0)lit+=24;
            const double v=lit*fade;
            out.pixel(x,y,rgb(int(v*(1+glow*.48)),int(v*1.24),int(v*1.5)));
        }
    }
    sprites(out,game,direction,plane,horizon,depths);
    const Color cross=game.hitSeconds()>0?rgb(255,87,66):rgb(125,157,160);
    out.rect(ScreenW/2-3,ScreenH/2,2,1,cross);
    out.rect(ScreenW/2+2,ScreenH/2,2,1,cross);
    out.rect(ScreenW/2,ScreenH/2-3,1,2,cross);
    out.rect(ScreenW/2,ScreenH/2+2,1,2,cross);
    if(game.hitSeconds()>0) {
        for(int y=0;y<ScreenH;y+=4) {
            out.pixel(1,y,rgb(100,19,21));
            out.pixel(ScreenW-2,y,rgb(100,19,21));
        }
    }
    const bool imminent=game.rewriteSeconds()<3&&game.completed()<5;
    const int lines=int((glow*.7+(imminent?.9:0))*9);
    for(int i=0;i<lines;++i) {
        const int yy=int(noiseHash(i,int(game.elapsed()*22),game.rewrites())%ScreenH);
        const int xx=int(noiseHash(i,yy,game.rewrites())%ScreenW);
        const int width=12+int(noiseHash(xx,i)%75);
        out.rect(xx,yy,width,1,imminent?rgb(139,33,39):rgb(61,38,48));
    }
}
Color SceneRenderer::spritePixel(int kind,int type,double u,double v,
                                  double pulse,int active) const {
    const double horizontal=std::abs(u-.5);
    if(kind==0) {
        if(horizontal>.31||v<.04||v>.97) return 0;
        const Color tone=type==active?TerminalColors[static_cast<std::size_t>(type)]:rgb(49,61,68);
        if(v<.16)return horizontal<.23?shade(tone,.7):rgb(15,21,29);
        if(v<.54) {
            if(horizontal<.20&&v>.22&&v<.48) {
                if((int(u*30)+int(v*38))%8==0)return rgb(223,239,212);
                return shade(tone,.85+.1*pulse);
            }
            return rgb(21,29,39);
        }
        if(horizontal<.1&&v>.68&&v<.78)return tone;
        return int(v*48)%8==0?rgb(49,64,71):rgb(24,31,41);
    }
    if(kind==1||kind==3) {
        if(v<.08||v>.98)return 0;
        if(v<.30) {
            const double radius=std::hypot((u-.5)*1.1,(v-.20)*2.5);
            if(radius>.22)return 0;
            if(v>.168&&v<.215&&((u>.35&&u<.43)||(u>.56&&u<.64)))
                return kind==3?(pulse>.5?rgb(255,220,98):rgb(253,80,51))
                              :(pulse>.5?rgb(255,78,65):rgb(143,27,45));
            return rgb(12,11,18);
        }
        const double width=v<.65?.18+(v-.3)*.43:.33-(v-.65)*.16;
        if(horizontal<width) {
            if(horizontal<.03)return kind==3?rgb(98,39,42):rgb(45,25,34);
            return kind==3?rgb(38+int(19*v),13,25):rgb(15+int(13*v),15,24+int(8*v));
        }
        if(v>.36&&v<.78&&horizontal<.36)
            return kind==3?rgb(73,22,35):rgb(23,19,28);
        return 0;
    }
    if(kind==2) {
        if(v<.04||v>.98||horizontal>.38)return 0;
        if(horizontal>.28||v<.13||v>.88)return rgb(96,212,216);
        if(horizontal>.23)return rgb(27,68,78);
        if(int((u+v)*31)%7==0)return rgb(38,87,96);
        return rgb(9,24,34);
    }
    return 0;
}
void SceneRenderer::sprites(Canvas& out,const GameSession& game,
                            Vec2 direction,Vec2 plane,int horizon,
                            const std::array<double,ScreenW>& depths) const {
    struct Sprite { Vec2 position;int kind;int type;double distance; };
    std::vector<Sprite> sprites;
    const Vec2 player=game.player();
    for(const auto& t:game.terminals())
        sprites.push_back({t.position,0,t.id,distance(player,t.position)});
    sprites.push_back({game.revisor(),game.completed()>=5?3:1,0,
                       distance(player,game.revisor())});
    if(game.completed()>=5)
        sprites.push_back({game.exit(),2,0,distance(player,game.exit())});
    std::sort(sprites.begin(),sprites.end(),
              [](const Sprite& a,const Sprite& b){return a.distance>b.distance;});
    const double determinant=1/(plane.x*direction.y-direction.x*plane.y);
    for(const auto& s:sprites) {
        const double relX=s.position.x-player.x,relY=s.position.y-player.y;
        const double transformX=determinant*(direction.y*relX-direction.x*relY);
        const double transformY=determinant*(-plane.y*relX+plane.x*relY);
        if(transformY<=.12)continue;
        const int screenX=int(ScreenW*.5*(1+transformX/transformY));
        const double scale=s.kind==1?.99:(s.kind==2?.95:.74);
        const int height=std::min(1200,int(ScreenH/transformY*scale));
        const int width=std::max(1,int(height*.64));
        const int left=screenX-width/2,top=horizon-height/2;
        const double pulse=std::fmod(game.elapsed()*5.8,1.0);
        for(int x=std::max(0,left);x<std::min(ScreenW,left+width);++x) {
            if(transformY>depths[x]+.08)continue;
            const double u=double(x-left)/width;
            for(int y=std::max(22,top);y<std::min(ScreenH-28,top+height);++y) {
                const double v=double(y-top)/height;
                const Color color=spritePixel(s.kind,s.type,u,v,pulse,game.activeTerminal());
                if(color)out.pixel(x,y,shade(color,std::clamp(1.1-transformY*.025,.26,1.0)));
            }
        }
    }
}
}
