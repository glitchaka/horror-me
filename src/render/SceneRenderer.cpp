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
    const Vec2 pos=game.player();
    const Vec2 direction{std::cos(game.heading()),std::sin(game.heading())};
    const Vec2 plane{-direction.y*FieldOfViewScale,direction.x*FieldOfViewScale};
    // World-space perspective projection of procedural floor and ceiling.
    // Textures are stable as the camera moves; no full-screen wallpaper.
    for(int y=0;y<ScreenH;++y) {
        const bool above=y<horizon;
        const int row=std::abs(y-horizon);
        const double worldDepth=(ScreenH*.53)/std::max(1,row);
        for(int x=0;x<ScreenW;++x) {
            const double camera=2.0*(x+.5)/ScreenW-1;
            const Vec2 world{
                pos.x+worldDepth*(direction.x+plane.x*camera),
                pos.y+worldDepth*(direction.y+plane.y*camera)
            };
            Color color=above?
                materials_.ceiling(world,game.completed(),game.elapsed(),game.pressure()):
                materials_.floor(world,game.completed(),game.elapsed(),game.pressure());
            const double fog=std::clamp(std::exp(-worldDepth*.105),.12,1.0);
            out.pixel(x,y,shade(color,fog));
        }
    }
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
        if((side==0&&rx>0)||(side==1&&ry<0))hit=1.0-hit;
        // Identify only fixtures on the wall face that the ray actually hits.
        const int floorX=side==0?mx-stepX:mx;
        const int floorY=side==1?my-stepY:my;
        int panelType=-1;
        bool exitPanel=false;
        for(const auto& terminal:game.terminals()) {
            if(terminal.wallX==mx&&terminal.wallY==my&&
               cell(terminal.position.x)==floorX&&
               cell(terminal.position.y)==floorY) {
                panelType=terminal.id;
                break;
            }
        }
        if(game.completed()>=5) {
            const auto [wx,wy]=game.exitWall();
            if(wx==mx&&wy==my&&
               cell(game.exit().x)==floorX&&cell(game.exit().y)==floorY)
                exitPanel=true;
        }
        const double fade=std::clamp(1.0/(1.0+depth*.12),.20,1.0);
        for(int y=top;y<bottom;++y) {
            const double relativeV=double(y-horizon+wallHeight*.5)/std::max(1,wallHeight);
            Color wallColor=materials_.wall(mx,my,hit,relativeV,side,depth,
                                             game.completed(),game.elapsed(),game.pressure());
            // Functional fixtures cast a visible band of light onto their wall.
            const bool activePanel=exitPanel||(panelType==game.activeTerminal()&&panelType>=0);
            if(activePanel&&std::abs(hit-.5)<.45&&relativeV>.13&&relativeV<.88)
                wallColor=shade(wallColor,1.20);
            const Color decal=(panelType>=0||exitPanel)?
                panelPixel(exitPanel,panelType,hit,relativeV,
                           std::fmod(game.elapsed()*2.0,1.0),game.activeTerminal()):0;
            out.pixel(x,y,decal?shade(decal,std::clamp(fade+.28,.53,1.0)):wallColor);
        }
    }
    drawRevisor(out,game,direction,plane,horizon,depths);
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
    const int lines=int((glow*.6+(imminent?.65:0))*11);
    for(int i=0;i<lines;++i) {
        const int yy=int(noiseHash(i,int(game.elapsed()*22),game.rewrites())%ScreenH);
        const int xx=int(noiseHash(i,yy,game.rewrites())%ScreenW);
        const int width=12+int(noiseHash(xx,i)%95);
        out.rect(xx,yy,width,1,imminent?rgb(112,27,35):rgb(51,42,49));
    }
    atmosphere_.apply(out,game.completed(),game.pressure(),game.elapsed(),game.shake());
}
Color SceneRenderer::panelPixel(bool exitPanel,int type,double u,double v,
                                 double pulse,int active) const {
    // A recessed industrial inspection terminal. Wall decal, never a door
    // or collision object inside the traversal space.
    if(u<.18||u>.82||v<.245||v>.80)return 0;
    const double x=(u-.18)/.64,y=(v-.245)/.555;
    const bool isActive=exitPanel||type==active;
    const Color status=exitPanel?rgb(218,113,91):
                       isActive?TerminalColors[static_cast<std::size_t>(type)]
                               :rgb(49,84,78);
    // Oxidized outer housing, metallic bevel, corner screws.
    if(x<.035||x>.965||y<.035||y>.965)return rgb(13,21,23);
    if(x<.083||x>.917||y<.073||y>.93){
        const bool screw=(x<.12||x>.88)&&(y<.13||y>.87);
        if(screw)return rgb(190,168,109);
        return rgb(89,108,106);
    }
    // Red/amber hazard stripes indicate a containment seal.
    if(y<.175||y>.875) {
        const bool marker=int((x-y*.6)*24)%4<2;
        return marker?rgb(158,129,69):rgb(25,30,34);
    }
    if(x>.15&&x<.85&&y>.23&&y<.66){
        if(!isActive)return rgb(21,39,41);
        const double scan=std::fmod(y*15+pulse*2.9,1.0);
        if(scan<.075)return shade(status,1.55);
        if(int(x*19)%7==0)return shade(status,.34);
        if(int(y*30)%5==0)return shade(status,.58);
        return shade(status,.78+.11*std::sin(pulse*Pi*2));
    }
    // Compact indicator bank and brass mechanical switches.
    if(x>.23&&x<.77&&y>.73&&y<.85){
        const int led=int((x-.23)*22);
        if(led%5<=1)return isActive?rgb(205,219,177):rgb(87,90,79);
        return rgb(32,37,37);
    }
    if(x>.11&&x<.20&&y>.25&&y<.84)return rgb(27,42,41);
    if(x>.81&&x<.88&&y>.25&&y<.84)return rgb(27,42,41);
    return rgb(22,30,34);
}
Color SceneRenderer::revisorPixel(bool finalPhase,double u,double v,
                                   double pulse) const {
    // A bureaucratic apparition: an unnaturally elongated body,
    // a document-covered uniform, and a search-light in place of a face.
    const double x=u-.50;
    const double tremor=.014*std::sin(pulse*6.2831853);
    const double head=std::pow((x-tremor)*9.1,2)+std::pow((v-.225)*10.1,2);
    const bool eyes=v>.185&&v<.25&&
        ((x>-.058&&x<-.022)||(x>.028&&x<.064));
    const Color iris=finalPhase?rgb(252,62,57):rgb(193,219,183);
    if(head<1.0) {
        if(eyes)return iris;
        if(v>.262&&v<.274&&std::abs(x)<.044)
            return rgb(122,57,54);
        const int shadow=noiseHash(int(u*120),int(v*120),13)%21;
        return rgb(25+int(shadow)/2,30+int(shadow)/3,30+int(shadow)/2);
    }
    // Inverted collar and sharp shoulders.
    if(v>.305&&v<.39&&std::abs(x)<(.10+(v-.30)*1.65))
        return rgb(39,44,43);
    // Torso shifts from narrow chest to broad archival robe.
    const double torsoWidth=.105+.19*std::clamp((v-.34)/.55,0.0,1.0);
    if(v>.36&&v<.94&&std::abs(x)<torsoWidth) {
        // A pale mandate stamped vertically across the chest.
        if(std::abs(x)<.052&&v>.47&&v<.75) {
            const int line=int(v*140)%16;
            if(line<2||line==7)return finalPhase?rgb(206,91,75):rgb(173,161,128);
            return rgb(76,77,65);
        }
        if(std::abs(x)<.012)return rgb(102,93,77);
        if(std::abs(x)>torsoWidth-.022)return rgb(13,19,21);
        const int weave=int(noiseHash(int(u*91),int(v*111),23)%18);
        const double f=.72+.28*std::sin(v*14+u*6);
        return rgb(int((25+weave*.4)*f),int((32+weave*.3)*f),int((35+weave*.3)*f));
    }
    // Two separate, impossibly long arms and articulated metal fingers.
    const double leftArm=-.20-.04*std::sin(v*3.2+pulse*.9);
    const double rightArm=.20+.10*std::sin(v*3.7+pulse*1.1);
    if(v>.42&&v<.97) {
        if(std::abs(x-leftArm)<.029||std::abs(x-rightArm)<.020)
            return finalPhase?rgb(62,25,31):rgb(29,41,39);
        if(v>.86&&v<.97&&(std::abs(x-(leftArm-.03))<.010||
                           std::abs(x-(rightArm+.03))<.012))
            return rgb(97,103,82);
    }
    // Legs are narrower than the robe, separated at the bottom.
    if(v>.90&&v<1.0&&((x>-.145&&x<-.065)||(x>.070&&x<.150)))
        return rgb(19,24,25);
    return 0;
}
void SceneRenderer::drawRevisor(Canvas& out,const GameSession& game,
                                 Vec2 direction,Vec2 plane,int horizon,
                                 const std::array<double,ScreenW>& depths) const {
    const Vec2 player=game.player();
    const Vec2 enemy=game.revisor();
    const double inverse=1.0/(plane.x*direction.y-direction.x*plane.y);
    const double relX=enemy.x-player.x,relY=enemy.y-player.y;
    const double transformX=inverse*(direction.y*relX-direction.x*relY);
    const double transformY=inverse*(-plane.y*relX+plane.x*relY);
    if(transformY<=.12)return;
    const int screenX=int(ScreenW*.5*(1+transformX/transformY));
    const int height=std::min(1200,int(ScreenH/transformY*.99));
    const int width=std::max(1,int(height*.64));
    const int left=screenX-width/2,top=horizon-height/2;
    const double pulse=std::fmod(game.elapsed()*5.8,1.0);
    for(int x=std::max(0,left);x<std::min(ScreenW,left+width);++x) {
        if(transformY>depths[x]+.08)continue;
        const double u=double(x-left)/width;
        for(int y=std::max(22,top);y<std::min(ScreenH-28,top+height);++y) {
            const double v=double(y-top)/height;
            const Color color=revisorPixel(game.completed()>=5,u,v,pulse);
            if(color)out.pixel(x,y,shade(color,std::clamp(1.1-transformY*.025,.26,1.0)));
        }
    }
}
}
