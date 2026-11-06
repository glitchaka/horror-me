#include "horror/Atmosphere.hpp"
#include "horror/Canvas.hpp"
#include "horror/FontAtlas.hpp"
#include "horror/HudRenderer.hpp"
#include "horror/Input.hpp"
#include "horror/MaterialArtist.hpp"
#include "horror/PixelPresenter.hpp"
#include "horror/SceneRenderer.hpp"
#include "horror/Session.hpp"
#include <exception>
#include <iostream>
#include <stdexcept>

using namespace horror;
namespace {
void require(bool good,const char* description){
    if(!good)throw std::runtime_error(description);
}
void pixelPerfectTest() {
    PixelPresenter presenter;
    std::vector<Color> source(static_cast<std::size_t>(ScreenW)*ScreenH,rgb(12,19,23));
    source[0]=rgb(246,37,73);
    source[1]=rgb(15,211,97);
    const auto& output=presenter.compose(source.data(),1366,768);
    const PixelViewport viewport=presenter.viewport();
    require(viewport.scale==3,"Fractional scale used at 1366x768");
    require(viewport.x==107&&viewport.y==60,"Letterbox is not centered");
    require(viewport.width==1152&&viewport.height==648,"Incorrect 3x pixel dimensions");
    const auto at=[&](int x,int y){return output[static_cast<std::size_t>(y)*1366+x];};
    require(at(0,0)==0&&at(106,60)==0&&at(107,59)==0,"Letterbox pixels are not black");
    for(int y=60;y<63;++y)for(int x=107;x<110;++x)
        require(at(x,y)==rgb(246,37,73),"A source pixel was filtered or blended");
    for(int y=60;y<63;++y)for(int x=110;x<113;++x)
        require(at(x,y)==rgb(15,211,97),"Adjacent source pixels were blended");
    const auto& full=presenter.compose(source.data(),1920,1080);
    const auto five=presenter.viewport();
    require(five.scale==5&&five.width==1920&&five.height==1080,
            "Fullscreen is not pixel-perfect at 1080p");
    require(full[0]==rgb(246,37,73)&&full[4]==rgb(246,37,73)&&
            full[5]==rgb(15,211,97),"A 5x native pixel block was not retained");
    presenter.compose(source.data(),300,160);
    require(presenter.viewport().scale==1,"Small viewport must never interpolate");
}
void renderTest(){
    Canvas canvas;
    const auto& font=FontAtlas::instance();
    require(font.textWidth("PROTOCOLO",false)>35,"Annapurna regular glyph widths missing");
    require(font.textWidth("PROTOCOLO",true)>60,"Annapurna bold glyph widths missing");
    int sampleCount=0;
    for(int y=0;y<font.height(false);++y)
        for(int x=0;x<font.glyphWidth('A',false);++x)
            if(font.alpha('A',false,x,y)>0)++sampleCount;
    require(sampleCount>15,"Annapurna glyph alpha missing");
    canvas.clear(rgb(2,4,5));
    canvas.text(8,8,"NO CONFORME",rgb(197,207,188));
    canvas.center(40,"PROTOCOLO",rgb(226,92,89),2);
    int changed=0;
    for(int i=0;i<ScreenW*ScreenH;++i)
        if(canvas.data()[i]!=rgb(2,4,5))++changed;
    require(changed>170,"Font atlas output is invisible");
    const MaterialArtist materials;
    const Color wall=materials.wall(2,4,.4,.5,0,2.5,2,12,45);
    const Color floor=materials.floor({4.2,8.3},2,12,45);
    const Color ceiling=materials.ceiling({4.2,8.3},2,12,45);
    require(wall!=floor||floor!=ceiling,"Procedural materials collapsed to one color");
    Atmosphere effects;
    effects.apply(canvas,3,83,22,.1);
    GameSession game;
    Input input;
    input.set(13,true);
    game.update(.016,input);
    require(game.phase()==Phase::Playing,"Game could not start");
    SceneRenderer scene;
    HudRenderer hud;
    scene.draw(canvas,game);
    hud.draw(canvas,game,input);
    require(canvas.data()[ScreenW*20+20]!=canvas.data()[ScreenW*140+20],
            "3D world and UI have no contrast");
    constexpr int mapX=ScreenW-MapW*3-9;
    require(canvas.data()[static_cast<std::size_t>(38)*ScreenW+mapX-6]==rgb(106,178,159),
            "The always-on minimap border was not rendered");
    const Vec2 goal=game.objective();
    const int targetX=mapX+cell(goal.x)*3;
    const int targetY=57+cell(goal.y)*3;
    require(canvas.data()[static_cast<std::size_t>(targetY)*ScreenW+targetX]==rgb(248,206,94),
            "The minimap does not mark the active objective in gold");
}
}
int main(){
    try{
        pixelPerfectTest();
        renderTest();
        std::cout<<"RenderTests: OK (pixel perfect, minimap, objective, font, 3D+HUD)\n";
        return 0;
    }catch(const std::exception& error){
        std::cerr<<"RenderTests: FALLO: "<<error.what()<<"\n";
        return 1;
    }
}
