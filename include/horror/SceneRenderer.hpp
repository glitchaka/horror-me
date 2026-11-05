#pragma once
#include "horror/Canvas.hpp"
#include "horror/Session.hpp"
#include <array>

namespace horror {
class SceneRenderer final {
public:
    void draw(Canvas&,const GameSession&) const;
private:
    void sprites(Canvas&,const GameSession&,Vec2 direction,Vec2 plane,
                 int horizon,const std::array<double,ScreenW>& depth) const;
    [[nodiscard]] Color spritePixel(int kind,int terminal,double u,double v,
                                    double pulse,int active) const;
};
}
