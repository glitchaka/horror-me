#pragma once
#include "horror/Maze.hpp"

namespace horror {
class RevisorAI final {
public:
    void relocate(Vec2 position) {
        position_=position;
        clock_=0.0;
    }
    void update(const Maze& maze,Vec2 player,double pressure,
                int completed,double deltaSeconds);
    [[nodiscard]] Vec2 position() const { return position_; }
private:
    Vec2 position_{20.5,20.5};
    double clock_=0.0;
};
}
