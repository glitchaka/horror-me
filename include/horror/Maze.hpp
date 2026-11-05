#pragma once
#include "horror/Types.hpp"
#include <array>
#include <random>
#include <vector>

namespace horror {
class Maze final {
public:
    void regenerate(std::mt19937& random);
    [[nodiscard]] bool wall(int x,int y) const;
    [[nodiscard]] bool open(int x,int y) const { return !wall(x,y); }
    [[nodiscard]] bool circleFree(double x,double y,double radius=.22) const;
    [[nodiscard]] bool visible(Vec2 a,Vec2 b) const;
    [[nodiscard]] std::vector<Vec2> floors() const;
    [[nodiscard]] Vec2 nearest(Vec2 p) const;
    [[nodiscard]] Vec2 farthest(Vec2 p) const;
private:
    std::array<std::array<char,MapW>,MapH> cells_{};
};
}
