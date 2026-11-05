#include "horror/HudRenderer.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>

namespace horror {
namespace {
constexpr std::array<Color,4> TerminalColors{
    rgb(225,166,110),rgb(112,196,217),rgb(216,213,196),rgb(196,139,208)
};
}
void HudRenderer::map(Canvas& canvas,const GameSession& game) const {
    constexpr int scale=4;
    constexpr int left=ScreenW-MapW*scale-11;
    constexpr int top=34;
    canvas.rect(left-6,top-17,MapW*scale+12,MapH*scale+54,rgb(6,12,20));
    canvas.text(left,top-12,"PLANO INESTABLE",rgb(164,190,199));
    const Maze& maze=game.maze();
    for(int y=0;y<MapH;++y) for(int x=0;x<MapW;++x)
        canvas.rect(left+x*scale,top+y*scale,scale-1,scale-1,
                    maze.wall(x,y)?rgb(39,52,62):rgb(13,24,31));
    const auto marker=[&](Vec2 position,Color color) {
        canvas.rect(left+cell(position.x)*scale,top+cell(position.y)*scale,3,3,color);
    };
    const auto route=game.objectiveRoute();
    for(std::size_t i=1;i<route.size();++i)
        marker(route[i],rgb(33,103,119));
    for(const auto& terminal:game.terminals())
        marker(terminal.position,terminal.id==game.activeTerminal()&&game.completed()<5?
               TerminalColors[static_cast<std::size_t>(terminal.id)]:rgb(82,85,90));
    if(game.completed()>=5)marker(game.exit(),rgb(113,235,209));
    marker(game.revisor(),rgb(227,39,59));
    marker(game.player(),rgb(242,242,217));
    canvas.text(left,top+MapH*scale+3,"BLANCO: TU",rgb(242,242,217));
    canvas.text(left,top+MapH*scale+13,"ROJO: REVISOR",rgb(227,68,78));
    canvas.text(left,top+MapH*scale+23,"AZUL: RUTA",rgb(114,220,200));
}
void HudRenderer::menu(Canvas& canvas,const GameSession& game) const {
    canvas.clear(rgb(6,10,16));
    for(int y=0;y<ScreenH;++y) {
        const int wave=int(std::sin(y*.17+game.elapsed()*.7)*10);
        for(int x=0;x<ScreenW;++x) {
            if(std::abs(x-ScreenW/2+wave)<1&&y%8<5)
                canvas.pixel(x,y,rgb(34,58,63));
            if(noiseHash(x/5,y/5,33)%89==0)
                canvas.pixel(x,y,rgb(19,32,42));
        }
    }
    canvas.center(27,"PROTOCOLO",rgb(170,199,209),2);
    canvas.center(48,"NO CONFORME",rgb(235,105,103),2);
    canvas.center(76,"HORROR SISTEMICO EN PRIMERA PERSONA",rgb(137,165,173));
    canvas.center(95,"CUMPLE 5 ORDENES CAMBIANTES.",rgb(221,213,190));
    canvas.center(107,"BUSCA EL PANEL ILUMINADO EN LA PARED.",rgb(221,213,190));
    canvas.center(119,"EL PLANO MARCA LA RUTA EN AZUL.",rgb(221,213,190));
    canvas.text(25,146,"WASD: MOVER  RATON / FLECHAS: GIRAR",rgb(145,176,182));
    canvas.text(25,156,"E: SELLAR  F: RASTREAR  Q: ANCLA",rgb(145,176,182));
    canvas.text(25,166,"TAB: RUTA  SHIFT: CORRER  P: PAUSA",rgb(145,176,182));
    canvas.center(190,"ENTER PARA ENTRAR",rgb(239,162,132));
    canvas.center(202,"M: AUDIO",rgb(98,125,136));
}
void HudRenderer::end(Canvas& canvas,const GameSession& game,bool victory) const {
    canvas.clear(rgb(10,9,16));
    if(victory) {
        canvas.center(27,"CONTRATO CERRADO",rgb(144,223,203),2);
        canvas.center(71,"LA ULTIMA REVISION NO ERA OBLIGATORIA.",rgb(210,219,212));
        canvas.center(85,"HAS SALIDO DEL CICLO.",rgb(218,231,220));
        canvas.center(111,"LA RESPUESTA FINAL FUE: NO.",rgb(144,223,203));
    } else {
        canvas.center(27,"NO CONFORME",rgb(224,76,83),2);
        canvas.center(74,game.causeOfDefeat(),rgb(229,146,137));
        canvas.center(95,"EL SISTEMA NO TE DEJO TERMINAR.",rgb(196,193,189));
        canvas.center(110,"PUEDES INTENTARLO CON OTRO CRITERIO.",rgb(154,175,179));
    }
    char stats[100]{};
    std::snprintf(stats,sizeof(stats),"ENTREGAS %d/5  REVISIONES %d  TIEMPO %d S",
                  game.completed(),game.rewrites(),int(game.elapsed()));
    canvas.center(144,stats,rgb(160,185,190));
    canvas.center(182,"ENTER: REINICIAR   ESC: MENU",rgb(225,202,177));
}
void HudRenderer::overlay(Canvas& canvas,const GameSession& game) const {
    canvas.rect(0,0,ScreenW,24,rgb(8,13,21));
    canvas.rect(0,23,ScreenW,1,rgb(51,70,77));
    canvas.text(6,5,"PROTOCOLO / NO CONFORME",rgb(157,185,200));
    canvas.text(6,15,game.actName(),rgb(179,115,114));
    char label[32]{};
    std::snprintf(label,sizeof(label),"%d/5",game.completed());
    canvas.text(167,5,label,rgb(226,225,196));
    canvas.text(222,5,"PRESION",rgb(207,157,152));
    canvas.rect(270,5,107,10,rgb(41,28,38));
    canvas.rect(272,7,int(103*game.tension()),6,
                game.pressure()>72?rgb(232,45,58):rgb(166,80,79));
    if(game.completed()>=5)
        canvas.center(30,"EL REVISOR / FASE FINAL",rgb(252,111,86));
    if(game.completed()<5&&game.rewriteSeconds()<3&&int(game.elapsed()*5)%2==0)
        canvas.center(34,"RECONFIGURACION INMINENTE",rgb(245,83,81));
    if(game.anchorSeconds()>0)
        canvas.text(7,32,"ANCLA ACTIVA",rgb(107,215,214));
    if(game.scanSeconds()>0) {
        const auto route=game.objectiveRoute();
        const Vec2 player=game.player();
        const Vec2 target=route.size()>1?route[1]:game.objective();
        const double angle=wrapAngle(std::atan2(target.y-player.y,target.x-player.x)-game.heading());
        const bool behind=std::cos(angle)<-.50;
        const int x=std::clamp(int(ScreenW/2+std::sin(angle)*115),35,ScreenW-35);
        canvas.text(x-38,39,behind?"GIRA DETRAS":"SIGUE RUTA",rgb(235,202,130));
        canvas.rect(x-2,50,5,5,rgb(235,202,130));
    }
    canvas.rect(0,179,ScreenW,37,rgb(8,12,18));
    canvas.rect(0,179,ScreenW,1,rgb(54,68,75));
    char title[100]{};
    std::snprintf(title,sizeof(title),"ORDEN %d/5: %s  |  RUTA %d PASOS",
                  std::min(game.completed()+1,5),game.objectiveName().c_str(),
                  std::max(0,game.routeSteps()));
    if(game.completed()>=5)
        std::snprintf(title,sizeof(title),"OBJETIVO FINAL: RECHAZAR EN EL NUCLEO | %d PASOS",
                      std::max(0,game.routeSteps()));
    canvas.text(5,184,title,rgb(228,225,200));
    char status[100]{};
    if(game.completed()<5)
        std::snprintf(status,sizeof(status),"TAB: RUTA  F: RASTREAR  E: SELLAR  CAMBIO %02dS  ANCLAS %d",
                      int(std::max(0.0,game.rewriteSeconds())),game.anchors());
    else
        std::snprintf(status,sizeof(status),"SIGUE EL PLANO (TAB) Y PULSA R JUNTO AL NUCLEO");
    canvas.text(5,197,status,rgb(122,158,165));
    if(game.notificationSeconds()>0) {
        canvas.rect(0,161,ScreenW,15,rgb(18,16,22));
        canvas.center(165,game.notification(),rgb(236,156,143));
    }
    const int nearby=game.nearbyTerminal();
    if(game.completed()<5&&nearby>=0&&nearby!=game.activeTerminal()) {
        canvas.rect(59,136,266,20,rgb(40,18,24));
        canvas.center(142,"PANEL INCORRECTO - BUSCA EL ILUMINADO",rgb(240,146,137));
    }
    if(game.objectiveNearby()) {
        canvas.rect(89,139,206,17,rgb(11,28,32));
        if(game.completed()<5) {
            canvas.center(143,"OBJETIVO CORRECTO: MANTEN E",rgb(190,233,218));
            canvas.rect(100,155,int(184*std::clamp(game.interactionProgress()/1.18,0.0,1.0)),
                        2,rgb(112,215,193));
        } else canvas.center(143,"PULSA R PARA DECIR NO",rgb(190,233,218));
    }
}
void HudRenderer::draw(Canvas& canvas,const GameSession& game,const Input& input) const {
    switch(game.phase()) {
    case Phase::Menu: menu(canvas,game);return;
    case Phase::Defeat: end(canvas,game,false);return;
    case Phase::Victory: end(canvas,game,true);return;
    case Phase::Playing:
    case Phase::Paused: break;
    }
    overlay(canvas,game);
    if(input.held(9)) map(canvas,game);
    if(game.phase()==Phase::Paused) {
        canvas.rect(54,69,ScreenW-108,74,rgb(7,12,20));
        canvas.center(79,"PAUSA",rgb(214,223,214),2);
        canvas.center(111,"P / ESC PARA CONTINUAR",rgb(159,190,196));
    }
}
}
