#include "horror/Maze.hpp"
#include "horror/Session.hpp"
#include "horror/RevisorAI.hpp"
#include <cmath>
#include <exception>
#include <iostream>
#include <random>
#include <stdexcept>
#include <vector>

using namespace horror;

namespace {
void require(bool condition,const char* message) {
    if(!condition)throw std::runtime_error(message);
}
void testMazeConnectivity() {
    for(std::uint32_t seed=0;seed<32;++seed) {
        std::mt19937 rng(seed*337u+11u);
        Maze maze;
        maze.regenerate(rng);
        const auto floor=maze.floors();
        require(floor.size()>125,"La generacion contiene pocas casillas");
        require(maze.open(1,1),"El punto de partida esta bloqueado");
        for(Vec2 p:floor)
            require(maze.stepsBetween({1.5,1.5},p)>=0,
                    "Existe una zona aislada desde el comienzo");
        std::vector<Vec2> protectedPositions{
            {1.5,1.5},maze.farthest({1.5,1.5}),maze.nearest({11.5,11.5})
        };
        for(int revision=0;revision<6;++revision) {
            maze.distort(rng,protectedPositions,7);
            for(Vec2 p:protectedPositions)
                require(maze.open(cell(p.x),cell(p.y)),
                        "La revision bloqueo una posicion protegida");
            for(Vec2 p:maze.floors())
                require(maze.stepsBetween({1.5,1.5},p)>=0,
                        "La revision corto la conectividad del laberinto");
        }
    }
}
void testRevisor() {
    std::mt19937 rng(777);
    Maze maze;
    maze.regenerate(rng);
    const Vec2 player{1.5,1.5};
    RevisorAI ai;
    ai.relocate(maze.farthest(player));
    require(maze.open(cell(ai.position().x),cell(ai.position().y)),
            "El perseguidor aparecio dentro de una pared");
    const int original=maze.stepsBetween(ai.position(),player);
    for(int i=0;i<180;++i)ai.update(maze,player,45,2,.05);
    require(maze.open(cell(ai.position().x),cell(ai.position().y)),
            "El perseguidor atraveso el laberinto");
    require(maze.stepsBetween(ai.position(),player)<original,
            "El perseguidor no se acerco al jugador");
}
void testSession() {
    GameSession session;
    Input input{};
    require(session.phase()==Phase::Menu,"La sesion no inicia en el menu");
    input.set(13,true);
    session.update(.016,input);
    require(session.phase()==Phase::Playing,"Enter no comienza la partida");
    input.endFrame();
    input.set(13,false);
    input.endFrame();
    require(session.rewriteSeconds()>=32,"Plazo inicial demasiado breve");
    for(const auto& terminal:session.terminals())
        require(session.maze().stepsBetween(session.player(),terminal.position)>=0,
                "Existe una terminal inaccesible");
    for(int i=0;i<2200&&session.phase()==Phase::Playing;++i) {
        session.update(.05,input);
        require(std::isfinite(session.pressure()),"Presion no finita");
        require(session.maze().open(cell(session.player().x),cell(session.player().y)),
                "Jugador desplazado a una pared");
        require(session.maze().stepsBetween(session.player(),session.objective())>=0,
                "Objetivo inaccesible tras una revision");
    }
    if(session.phase()==Phase::Defeat) {
        input.set(13,true);
        session.update(.016,input);
        require(session.phase()==Phase::Playing,"Enter no reinicia la sesion");
    }
}
}
int main() {
    try {
        testMazeConnectivity();
        testRevisor();
        testSession();
        std::cout<<"CoreTests: OK (topologia, perseguidor y estados)\n";
        return 0;
    } catch(const std::exception& error) {
        std::cerr<<"CoreTests: FALLO - "<<error.what()<<'\n';
        return 1;
    }
}
