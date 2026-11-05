#include "horror/Session.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <queue>
#include <utility>

namespace horror {
namespace {
constexpr int KeyEnter=13, KeyEscape=27, KeyTab=9, KeyShift=16;
constexpr int KeyLeft=37, KeyUp=38, KeyRight=39, KeyDown=40;
constexpr std::array<int,5> Sequence{2,0,3,1,2};
constexpr std::array<const char*,4> Names{"DEPURAR","COMPILAR","VERIFICAR","ENTREGAR"};
}
GameSession::GameSession()=default;
int GameSession::randomInt(int low,int high) {
    return std::uniform_int_distribution<int>(low,high)(random_);
}
void GameSession::inform(std::string text,double seconds) {
    notification_=std::move(text);
    notificationSeconds_=seconds;
}
void GameSession::sound(SoundEvent event) { pendingSounds_.push_back(event); }
std::vector<SoundEvent> GameSession::drainSounds() {
    std::vector<SoundEvent> current;
    current.swap(pendingSounds_);
    return current;
}
Vec2 GameSession::objective() const {
    return completed_<5?terminals_[activeTerminal_].position:exit_;
}
std::string GameSession::objectiveName() const {
    if(completed_>=5) return "RECHAZAR LA REVISION FINAL";
    return std::string("SELLAR: ")+Names[activeTerminal_];
}
bool GameSession::objectiveNearby() const {
    return distance(player_,objective())<1.4&&maze_.visible(player_,objective());
}
void GameSession::placeObjects() {
    // Place terminals by route length, not geometric distance.
    // The player must be able to make meaningful progress before a revision.
    const auto floors=maze_.floors();
    std::vector<Vec2> chosen;
    constexpr std::array<int,4> desiredSteps{9,12,15,18};
    for(int i=0;i<4;++i) {
        double best=-1e20;
        Vec2 selected=player_;
        for(const Vec2 p:floors) {
            const int steps=maze_.stepsBetween(player_,p);
            if(steps<5)continue;
            double nearest=distance(p,player_);
            for(const Vec2 other:chosen)
                nearest=std::min(nearest,distance(p,other));
            if(nearest<3.0)continue;
            const double jitter=double(noiseHash(cell(p.x),cell(p.y),i)%100)/100.0;
            const double score=-std::abs(steps-desiredSteps[static_cast<std::size_t>(i)])*2.0
                               +std::min(nearest,7.0)*.6+jitter;
            if(score>best){best=score;selected=p;}
        }
        chosen.push_back(selected);
        terminals_[static_cast<std::size_t>(i)]={selected,i};
    }
    // The central exit is chosen before revisions and remains stable.
    exit_=maze_.nearest({MapW/2.0,MapH/2.0});
    double exitScore=-1e20;
    for(const Vec2 p:floors) {
        const int steps=maze_.stepsBetween(player_,p);
        if(steps<7)continue;
        bool overlaps=false;
        for(const auto& terminal:terminals_)
            if(distance(p,terminal.position)<2.5)overlaps=true;
        if(overlaps)continue;
        const double score=-std::abs(steps-14)*1.5
                           -distance(p,{MapW/2.0,MapH/2.0})*.12;
        if(score>exitScore){exitScore=score;exit_=p;}
    }
    // The Revisor begins outside the player's immediate field of vision.
    revisorAI_.relocate(maze_.farthest(player_));
    double enemyScore=-1e20;
    for(const Vec2 p:floors) {
        const int steps=maze_.stepsBetween(player_,p);
        if(steps<11||steps>27||maze_.visible(player_,p))continue;
        const double score=-std::abs(steps-18)*1.4+distance(player_,p)*.16;
        if(score>enemyScore){enemyScore=score;revisorAI_.relocate(p);}
    }
}
void GameSession::resetDeadline() {
    if(completed_>=5){rewriteSeconds_=1e6;return;}
    const int steps=maze_.stepsBetween(player_,objective());
    // 2.55 tiles/second is the walking speed. Extra time covers
    // exploration, reaction, the Revisor, and imperfect routing.
    rewriteSeconds_=std::clamp(25.0+std::max(0,steps)*1.15,32.0,64.0);
}
void GameSession::start() {
    completed_=0;anchors_=2;rewrites_=0;
    activeTerminal_=Sequence[0];defeatCause_.clear();
    pressure_=9;rewriteSeconds_=33;elapsed_=0;
    scanSeconds_=scanCooldown_=anchorSeconds_=0;
    interaction_=wrongCooldown_=hitSeconds_=shake_=0;
    player_={1.5,1.5};heading_=.25;
    maze_.regenerate(random_);
    placeObjects();
    resetDeadline();
    phase_=Phase::Playing;
    inform("CONTRATO ABIERTO. CUMPLE 5 ENTREGAS.",5);
    sound(SoundEvent::Rewrite);
}
void GameSession::rewrite() {
    if(anchorSeconds_>0) {
        rewriteSeconds_=12;
        inform("CRITERIO ANCLADO. CAMBIO RECHAZADO.");
        return;
    }
    // Preserve player position and current objective while physically
    // altering corridors. Every floor tile remains reachable.
    std::vector<Vec2> safe{player_,revisorAI_.position(),exit_};
    for(const auto& terminal:terminals_)safe.push_back(terminal.position);
    maze_.distort(random_,safe,7);
    interaction_=0;
    scanSeconds_=0;
    ++rewrites_;
    activeTerminal_=(activeTerminal_+randomInt(1,3))%4;
    resetDeadline();
    wrongCooldown_=2.5;
    shake_=1.0;
    pressure_=std::min(100.0,pressure_+3.0);
    inform(std::string("CRITERIO MODIFICADO: ")+Names[activeTerminal_],5);
    sound(SoundEvent::Rewrite);
}
void GameSession::complete() {
    ++completed_;
    if(completed_<5) activeTerminal_=Sequence[completed_];
    interaction_=0;
    pressure_=std::max(0.0,pressure_-17.0);
    anchors_=std::min(3,anchors_+1);
    resetDeadline();
    shake_=.16;
    if(completed_==5)
        inform("LA ULTIMA ORDEN ES IMPOSIBLE. BUSCA EL NUCLEO.",6);
    else
        inform("ACEPTADO... HASTA LA SIGUIENTE REVISION.",4);
    sound(SoundEvent::Seal);
}
void GameSession::lose(std::string cause) {
    phase_=Phase::Defeat;
    defeatCause_=std::move(cause);
    sound(SoundEvent::Defeat);
}
void GameSession::win() {
    phase_=Phase::Victory;
    sound(SoundEvent::Victory);
}
void GameSession::move(const Input& input,double dt) {
    if(input.held(KeyLeft)) heading_=wrapAngle(heading_-2.1*dt);
    if(input.held(KeyRight)) heading_=wrapAngle(heading_+2.1*dt);
    heading_=wrapAngle(heading_+std::clamp(input.mouseDeltaX,-100.0,100.0)*.0032);
    const Vec2 forward{std::cos(heading_),std::sin(heading_)};
    const Vec2 sideways{-forward.y,forward.x};
    Vec2 velocity{};
    if(input.held('W')||input.held(KeyUp)) {
        velocity.x+=forward.x;velocity.y+=forward.y;
    }
    if(input.held('S')||input.held(KeyDown)) {
        velocity.x-=forward.x;velocity.y-=forward.y;
    }
    if(input.held('D')) {
        velocity.x+=sideways.x;velocity.y+=sideways.y;
    }
    if(input.held('A')) {
        velocity.x-=sideways.x;velocity.y-=sideways.y;
    }
    const double norm=length(velocity);
    if(norm>1e-5) {
        const double step=(input.held(KeyShift)?3.75:2.55)*dt/norm;
        if(maze_.circleFree(player_.x+velocity.x*step,player_.y))
            player_.x+=velocity.x*step;
        if(maze_.circleFree(player_.x,player_.y+velocity.y*step))
            player_.y+=velocity.y*step;
        if(input.held(KeyShift)) pressure_+=dt*.16;
    }
}
void GameSession::pauseOnFocusLoss() {
    if(phase_==Phase::Playing) phase_=Phase::Paused;
}
void GameSession::update(double dt,const Input& input) {
    dt=std::clamp(dt,0.0,.05);
    if(phase_==Phase::Menu) {
        elapsed_+=dt;
        if(input.pressed(KeyEnter)) start();
        return;
    }
    if(phase_==Phase::Defeat||phase_==Phase::Victory) {
        if(input.pressed(KeyEnter)) start();
        else if(input.pressed(KeyEscape)) phase_=Phase::Menu;
        return;
    }
    if(phase_==Phase::Paused) {
        if(input.pressed('P')||input.pressed(KeyEscape))
            phase_=Phase::Playing;
        return;
    }
    if(input.pressed('P')||input.pressed(KeyEscape)) {
        phase_=Phase::Paused;
        return;
    }
    elapsed_+=dt;
    notificationSeconds_=std::max(0.0,notificationSeconds_-dt);
    hitSeconds_=std::max(0.0,hitSeconds_-dt);
    scanSeconds_=std::max(0.0,scanSeconds_-dt);
    scanCooldown_=std::max(0.0,scanCooldown_-dt);
    anchorSeconds_=std::max(0.0,anchorSeconds_-dt);
    wrongCooldown_=std::max(0.0,wrongCooldown_-dt);
    shake_=std::max(0.0,shake_-dt*1.75);
    move(input,dt);
    if(input.pressed('F')&&scanCooldown_<=0) {
        scanSeconds_=3.6;
        scanCooldown_=7;
        pressure_=std::min(100.0,pressure_+1.0);
        inform("ESCANEO: DIRECCION DEL OBJETIVO EXPUESTA.",2.7);
        sound(SoundEvent::Scan);
    }
    if(input.pressed('Q')) {
        if(anchors_>0&&anchorSeconds_<=0) {
            --anchors_;
            anchorSeconds_=10;
            rewriteSeconds_=std::max(rewriteSeconds_,10.0);
            revisorAI_.relocate(maze_.farthest(player_));
            pressure_=std::max(0.0,pressure_-7.0);
            inform("ANCLA ACTIVA: DIEZ SEGUNDOS DE ESTABILIDAD.",3.2);
            sound(SoundEvent::Anchor);
        } else inform("SIN ANCLAS DISPONIBLES.",2);
    }
    const bool nearby=objectiveNearby();
    if(completed_<5) {
        if(nearby&&input.held('E')) {
            interaction_+=dt;
            if(interaction_>=1.18) complete();
        } else interaction_=std::max(0.0,interaction_-dt*2.2);
        if(!nearby&&input.held('E')&&wrongCooldown_<=0) {
            for(const auto& terminal:terminals_) {
                if(terminal.id==activeTerminal_) continue;
                if(distance(player_,terminal.position)<1.3&&
                   maze_.visible(player_,terminal.position)) {
                    wrongCooldown_=3.5;
                    pressure_+=8;
                    inform("TERMINAL INCORRECTA. COSTE DE RETRABAJO.",3);
                    sound(SoundEvent::Damage);
                    break;
                }
            }
        }
    } else if(nearby&&input.pressed('R')) {
        win();
        return;
    }
    rewriteSeconds_-=dt;
    if(rewriteSeconds_<=0&&completed_<5) rewrite();
    revisorAI_.update(maze_,player_,pressure_,completed_,dt);
    const double near=distance(player_,revisorAI_.position());
    if(near<4.5) pressure_+=dt*(4.5-near)*.55;
    pressure_+=dt*(.23+completed_*.06);
    if(completed_>=5&&near<8&&maze_.visible(player_,revisorAI_.position()))
        pressure_+=dt*1.7;
    if(near<.55&&hitSeconds_<=0) {
        pressure_+=23;
        hitSeconds_=1.5;
        shake_=1.0;
        revisorAI_.relocate(maze_.farthest(player_));
        inform("EL REVISOR TE ENCONTRO. RECUPERA EL CONTROL.",4);
        sound(SoundEvent::Damage);
        if(pressure_>=100) defeatCause_="EL REVISOR TE ALCANZO";
    }
    if(pressure_>=100)
        lose(defeatCause_.empty()?"LA PRESION DEL SISTEMA TE CONSUMIO":defeatCause_);
}
}
