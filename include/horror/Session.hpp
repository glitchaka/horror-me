#pragma once
#include "horror/Input.hpp"
#include "horror/Maze.hpp"
#include <array>
#include <random>
#include <string>
#include <vector>

namespace horror {
enum class Phase { Menu, Playing, Paused, Defeat, Victory };
enum class SoundEvent { Anchor, Scan, Seal, Damage, Rewrite, Defeat, Victory };
struct Terminal { Vec2 position{}; int id{}; };

class GameSession final {
public:
    GameSession();
    void update(double deltaSeconds,const Input& input);
    void pauseOnFocusLoss();
    [[nodiscard]] const Maze& maze() const { return maze_; }
    [[nodiscard]] Phase phase() const { return phase_; }
    [[nodiscard]] Vec2 player() const { return player_; }
    [[nodiscard]] Vec2 revisor() const { return revisor_; }
    [[nodiscard]] Vec2 exit() const { return exit_; }
    [[nodiscard]] const std::array<Terminal,4>& terminals() const { return terminals_; }
    [[nodiscard]] Vec2 objective() const;
    [[nodiscard]] std::string objectiveName() const;
    [[nodiscard]] int activeTerminal() const { return activeTerminal_; }
    [[nodiscard]] int completed() const { return completed_; }
    [[nodiscard]] int anchors() const { return anchors_; }
    [[nodiscard]] int rewrites() const { return rewrites_; }
    [[nodiscard]] double heading() const { return heading_; }
    [[nodiscard]] double pressure() const { return pressure_; }
    [[nodiscard]] double rewriteSeconds() const { return rewriteSeconds_; }
    [[nodiscard]] double scanSeconds() const { return scanSeconds_; }
    [[nodiscard]] double anchorSeconds() const { return anchorSeconds_; }
    [[nodiscard]] double interactionProgress() const { return interaction_; }
    [[nodiscard]] double hitSeconds() const { return hitSeconds_; }
    [[nodiscard]] double shake() const { return shake_; }
    [[nodiscard]] double elapsed() const { return elapsed_; }
    [[nodiscard]] bool objectiveNearby() const;
    [[nodiscard]] const std::string& notification() const { return notification_; }
    [[nodiscard]] double notificationSeconds() const { return notificationSeconds_; }
    [[nodiscard]] const std::string& causeOfDefeat() const { return defeatCause_; }
    std::vector<SoundEvent> drainSounds();
    [[nodiscard]] double tension() const { return std::clamp(pressure_/100.0,0.0,1.0); }
private:
    void start();
    void placeObjects();
    void resetDeadline();
    void rewrite();
    void move(const Input&,double);
    void chase(double);
    void complete();
    void lose(std::string reason);
    void win();
    void inform(std::string text,double seconds=3.4);
    void sound(SoundEvent);
    [[nodiscard]] int randomInt(int low,int high);

    Maze maze_;
    std::mt19937 random_{0x5EA1C0DEu};
    std::array<Terminal,4> terminals_{};
    std::vector<SoundEvent> pendingSounds_{};
    Vec2 player_{1.5,1.5},revisor_{20.5,20.5},exit_{11.5,11.5};
    Phase phase_{Phase::Menu};
    double heading_{.25},pressure_{9},rewriteSeconds_{33},elapsed_{0};
    double scanSeconds_{0},scanCooldown_{0},anchorSeconds_{0};
    double interaction_{0},wrongCooldown_{0},chaseClock_{0};
    double hitSeconds_{0},shake_{0},notificationSeconds_{0};
    int completed_{0},anchors_{2},rewrites_{0},activeTerminal_{2};
    std::string notification_,defeatCause_;
};
}
