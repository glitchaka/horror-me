#include "horror/Atmosphere.hpp"
#include "horror/Canvas.hpp"
#include "horror/FontAtlas.hpp"
#include "horror/HudRenderer.hpp"
#include "horror/Input.hpp"
#include "horror/MaterialArtist.hpp"
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
}
}
int main(){
    try{
        renderTest();
        std::cout<<"RenderTests: OK (font, materials, effects, 3D+HUD)\n";
        return 0;
    }catch(const std::exception& error){
        std::cerr<<"RenderTests: FALLO: "<<error.what()<<"\n";
        return 1;
    }
}
