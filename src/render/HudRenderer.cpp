#include "horror/HudRenderer.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <string>

namespace horror {
namespace {
constexpr std::array<Color,4> TerminalColors{
    rgb(225,166,110),rgb(112,196,217),rgb(216,213,196),rgb(196,139,208)
};
Color signal(const GameSession& game) {
    if(game.pressure()>75)return rgb(223,66,70);
    if(game.completed()>=4)return rgb(183,123,79);
    return rgb(123,196,176);
}
void stripe(Canvas& canvas,int y,Color color) {
    for(int x=4;x<ScreenW-4;x+=9)canvas.rect(x,y,5,1,color);
}
}
void HudRenderer::map(Canvas& canvas,const GameSession& game) const {
    constexpr int scale=4;
    constexpr int left=ScreenW-MapW*scale-12;
    constexpr int top=52;
    const int width=MapW*scale;
    canvas.rect(left-7,top-26,width+14,MapH*scale+54,rgb(6,13,17));
    canvas.rect(left-7,top-26,width+14,2,rgb(106,162,149));
    canvas.text(left-3,top-22,"PLANO / RUTA",rgb(174,217,204));
    const Maze& maze=game.maze();
    for(int y=0;y<MapH;++y)for(int x=0;x<MapW;++x)
        canvas.rect(left+x*scale,top+y*scale,scale-1,scale-1,
                    maze.wall(x,y)?rgb(46,60,61):rgb(14,25,29));
    const auto marker=[&](Vec2 pos,Color color) {
        canvas.rect(left+cell(pos.x)*scale,top+cell(pos.y)*scale,3,3,color);
    };
    for(Vec2 position:game.objectiveRoute())marker(position,rgb(61,152,158));
    for(const auto& terminal:game.terminals())
        marker(terminal.position,terminal.id==game.activeTerminal()&&game.completed()<5?
               TerminalColors[static_cast<std::size_t>(terminal.id)]:rgb(83,90,94));
    if(game.completed()>=5)marker(game.exit(),rgb(103,222,178));
    marker(game.revisor(),rgb(230,71,71));
    marker(game.player(),rgb(245,247,220));
    canvas.text(left,top+MapH*scale+3,"BLANCO: TU",rgb(221,235,211));
    canvas.text(left,top+MapH*scale+17,"AZUL: RUTA",rgb(119,209,209));
    canvas.text(left,top+MapH*scale+30,"ROJO: REVISOR",rgb(225,100,104));
}
void HudRenderer::miniMap(Canvas& canvas,const GameSession& game) const {
    // Always-visible, north-up minimap. The gold/green target is the
    // objective, while the Revisor is RED and the player is WHITE.
    constexpr int scale=3;
    constexpr int left=ScreenW-MapW*scale-9;
    constexpr int top=57;
    constexpr int width=MapW*scale;
    canvas.rect(left-6,top-19,width+12,width+33,rgb(7,19,22));
    canvas.rect(left-6,top-19,width+12,2,rgb(106,178,159));
    canvas.text(left-1,top-18,"OBJ / RUTA",rgb(190,227,202));
    const Maze& maze=game.maze();
    for(int y=0;y<MapH;++y)for(int x=0;x<MapW;++x)
        canvas.rect(left+x*scale,top+y*scale,scale,scale,
                    maze.wall(x,y)?rgb(47,61,65):rgb(12,26,30));
    for(const Vec2 tile:game.objectiveRoute())
        canvas.rect(left+cell(tile.x)*scale,top+cell(tile.y)*scale,
                    scale,scale,rgb(45,111,122));
    const Vec2 objective=game.objective();
    const int goalX=left+cell(objective.x)*scale;
    const int goalY=top+cell(objective.y)*scale;
    canvas.rect(goalX-1,goalY-1,scale+2,scale+2,
                game.completed()>=5?rgb(90,233,171):rgb(248,206,94));
    const Vec2 revisor=game.revisor();
    const int enemyX=left+cell(revisor.x)*scale;
    const int enemyY=top+cell(revisor.y)*scale;
    canvas.rect(enemyX,enemyY,scale,scale,rgb(234,49,60));
    const Vec2 player=game.player();
    const int playerX=left+cell(player.x)*scale;
    const int playerY=top+cell(player.y)*scale;
    canvas.rect(playerX-1,playerY-1,scale+2,scale+2,rgb(247,247,219));
    // Heading makes the player's orientation readable even at 3 px/tile.
    const int vx=int(std::round(std::cos(game.heading())*4));
    const int vy=int(std::round(std::sin(game.heading())*4));
    canvas.rect(playerX+1+vx,playerY+1+vy,2,2,rgb(247,247,219));
    canvas.rect(left,top+width+5,4,4,rgb(248,206,94));
    canvas.text(left+6,top+width,"OBJ",rgb(223,226,197));
    canvas.rect(left+40,top+width+5,4,4,rgb(234,49,60));
    canvas.text(left+46,top+width,"REV",rgb(223,180,172));
}
void HudRenderer::menu(Canvas& canvas,const GameSession& game) const {
    canvas.clear(rgb(8,13,17));
    for(int y=0;y<ScreenH;++y)for(int x=0;x<ScreenW;++x){
        const unsigned seed=noiseHash(x/3,y/3,77);
        if(seed%137u==0u)canvas.pixel(x,y,rgb(25,42,45));
        const int j=int(std::sin(game.elapsed()*.3+y*.05)*5);
        if(std::abs(x-319+j)<2&&y%6<4)
            canvas.pixel(x,y,rgb(34,59,55));
    }
    canvas.rect(8,7,ScreenW-16,2,rgb(107,158,139));
    canvas.rect(8,12,2,ScreenH-24,rgb(52,85,82));
    canvas.rect(ScreenW-10,12,2,ScreenH-24,rgb(52,85,82));
    canvas.rect(8,ScreenH-9,ScreenW-16,2,rgb(107,158,139));
    canvas.text(20,19,"DEPARTAMENTO DE VERIFICACION",rgb(145,188,173));
    canvas.text(20,33,"EXPEDIENTE  004 / SIN RESOLVER",rgb(101,127,129));
    canvas.rect(17,50,350,1,rgb(54,79,76));
    canvas.center(54,"PROTOCOLO",rgb(223,225,193),2);
    canvas.center(78,"NO CONFORME",rgb(220,111,94),2);
    canvas.center(107,"CINCO ENTREGAS. UN SOLO CRITERIO:",rgb(172,200,188));
    canvas.center(121,"EL QUE DECIDA EL SISTEMA.",rgb(188,199,193));
    canvas.center(136,"EL REVISOR NUNCA TERMINA SU TURNO.",rgb(126,152,148));
    canvas.rect(39,160,306,31,rgb(26,47,49));
    canvas.rect(39,160,306,1,rgb(156,191,151));
    canvas.center(163,"ENTER / INGRESAR",rgb(217,220,192),2);
    canvas.center(194,"WASD MOVER  F ESCAN  TAB RUTA  E SELLAR",rgb(114,152,153));
}
void HudRenderer::end(Canvas& canvas,const GameSession& game,bool victory) const {
    canvas.clear(rgb(10,13,17));
    for(int i=0;i<50;++i){
        const unsigned hash=noiseHash(i,int(game.elapsed()),26);
        canvas.rect(int(hash%ScreenW),int((hash>>8)%ScreenH),3,1,rgb(36,59,54));
    }
    const Color accent=victory?rgb(105,216,184):rgb(214,68,71);
    canvas.rect(12,16,ScreenW-24,2,accent);
    canvas.center(28,victory?"CONTRATO CERRADO":"NO CONFORME",accent,2);
    if(victory) {
        canvas.center(73,"EL SISTEMA NO PUEDE EXIGIR MAS.",rgb(200,212,191));
        canvas.center(89,"HAS RECHAZADO LA ULTIMA ORDEN.",rgb(216,220,203));
        canvas.center(111,"EL SILENCIO POR FIN ES TUYO.",accent);
    }else{
        canvas.center(73,game.causeOfDefeat(),rgb(238,151,139));
        canvas.center(90,"EL REVISOR HA CERRADO EL EXPEDIENTE.",rgb(174,184,180));
        canvas.center(111,"UNA ORDEN MAS. UN NUEVO INTENTO.",rgb(189,176,157));
    }
    char stats[96]{};
    std::snprintf(stats,sizeof(stats),"ENTREGAS %d/5  REVISIONES %d  TIEMPO %d S",
                  game.completed(),game.rewrites(),int(game.elapsed()));
    canvas.center(141,stats,rgb(147,174,168));
    canvas.rect(35,171,314,1,accent);
    canvas.center(177,"ENTER: REINICIAR  ESC: MENU",rgb(232,220,193));
}
void HudRenderer::overlay(Canvas& canvas,const GameSession& game) const {
    const Color accent=signal(game);
    canvas.rect(0,0,ScreenW,37,rgb(9,17,20));
    canvas.rect(0,35,ScreenW,2,rgb(55,84,77));
    canvas.rect(5,3,3,29,accent);
    canvas.text(13,1,"ARCHIVO / NO CONFORME",rgb(195,212,188));
    canvas.text(13,17,game.actName(),rgb(144,181,167));
    canvas.text(254,1,"PRESION",rgb(210,169,149));
    canvas.rect(310,7,68,11,rgb(45,31,35));
    canvas.rect(312,9,int(64*game.tension()),7,accent);
    canvas.rect(310,19,68,2,rgb(64,74,70));
    if(game.completed()<5&&game.rewriteSeconds()<4&&int(game.elapsed()*4)%2==0){
        canvas.rect(0,38,ScreenW,16,rgb(47,17,21));
        canvas.center(40,"REVISION DEL PROTOCOLO INMINENTE",rgb(244,123,117));
    }
    if(game.anchorSeconds()>0) {
        canvas.rect(5,40,89,16,rgb(19,61,57));
        canvas.text(8,42,"ANCLA ACTIVA",rgb(118,225,187));
    }
    // The guide follows the next traversable corridor of the objective
    // route. The enemy is never used for objective guidance.
    const auto route=game.objectiveRoute();
    const Vec2 player=game.player();
    const Vec2 waypoint=route.size()>1?route[1]:game.objective();
    const double bearing=wrapAngle(
        std::atan2(waypoint.y-player.y,waypoint.x-player.x)-game.heading());
    const char* direction=std::abs(bearing)<.40?"FRENTE":
                          std::abs(bearing)>2.75?"DETRAS":
                          bearing<0?"IZQUIERDA":"DERECHA";
    canvas.rect(7,40,209,19,rgb(10,31,32));
    canvas.rect(7,40,3,19,rgb(233,196,103));
    char nav[72]{};
    std::snprintf(nav,sizeof(nav),"RUTA AL OBJ: %s",direction);
    canvas.text(13,41,nav,rgb(241,218,145));
    if(game.scanSeconds()>0) {
        canvas.rect(7,61,238,20,rgb(9,39,41));
        canvas.text(12,62,"ESCAN: SIGUE LA RUTA AZUL",rgb(130,226,201));
    }
    canvas.rect(0,171,ScreenW,45,rgb(8,17,20));
    canvas.rect(0,170,ScreenW,1,rgb(72,102,94));
    const int route=std::max(0,game.routeSteps());
    char primary[90]{};
    if(game.completed()<5)
        std::snprintf(primary,sizeof(primary),"ORDEN %d/5: %s  |  RUTA %d PASOS",
                      game.completed()+1,game.objectiveName().c_str(),route);
    else
        std::snprintf(primary,sizeof(primary),"ORDEN FINAL: ENCUENTRA EL NUCLEO  |  %d PASOS",route);
    canvas.text(8,172,primary,rgb(231,224,188));
    char secondary[96]{};
    if(game.completed()<5)
        std::snprintf(secondary,sizeof(secondary),
                      "TAB: MAPA  F: RASTREO  E: SELLAR  CAMBIO %02d S",
                      int(std::max(0.0,game.rewriteSeconds())));
    else
        std::snprintf(secondary,sizeof(secondary),"TAB: MAPA  R: RECHAZAR EN EL NUCLEO");
    canvas.text(8,191,secondary,rgb(131,179,164));
    stripe(canvas,214,rgb(45,70,63));
    if(game.notificationSeconds()>0) {
        canvas.rect(0,149,ScreenW,19,rgb(20,29,31));
        canvas.rect(0,149,3,19,rgb(182,104,91));
        canvas.center(151,game.notification(),rgb(229,179,152));
    }
    const int nearby=game.nearbyTerminal();
    if(game.completed()<5&&nearby>=0&&nearby!=game.activeTerminal()){
        canvas.rect(54,129,ScreenW-108,19,rgb(65,20,27));
        canvas.center(130,"PANEL INCORRECTO / BUSCA EL ILUMINADO",rgb(239,148,132));
    }
    if(game.objectiveNearby()){
        canvas.rect(54,126,ScreenW-108,21,rgb(16,61,55));
        if(game.completed()<5){
            canvas.center(128,"PANEL CORRECTO: MANTEN E",rgb(213,236,196));
            canvas.rect(64,147,int(254*std::clamp(game.interactionProgress()/1.18,0.0,1.0)),
                        2,rgb(119,226,185));
        }else
            canvas.center(128,"NIEGATE: PULSA R",rgb(225,231,187));
    }
}
void HudRenderer::draw(Canvas& canvas,const GameSession& game,const Input& input) const {
    switch(game.phase()){
    case Phase::Menu: menu(canvas,game);return;
    case Phase::Defeat:end(canvas,game,false);return;
    case Phase::Victory:end(canvas,game,true);return;
    case Phase::Playing:case Phase::Paused:break;
    }
    overlay(canvas,game);
    miniMap(canvas,game);
    if(input.held(9))map(canvas,game);
    if(game.phase()==Phase::Paused){
        canvas.rect(46,67,ScreenW-92,77,rgb(6,18,21));
        canvas.rect(46,67,ScreenW-92,2,rgb(141,184,163));
        canvas.center(78,"SISTEMA SUSPENDIDO",rgb(211,213,189),2);
        canvas.center(110,"P / ESC PARA CONTINUAR",rgb(157,192,176));
    }
}
}
