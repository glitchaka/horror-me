#pragma once
#include "horror/Canvas.hpp"
#include "horror/Input.hpp"
#include "horror/Session.hpp"

namespace horror {
class HudRenderer final {
public:
    void draw(Canvas&,const GameSession&,const Input&) const;
private:
    void map(Canvas&,const GameSession&) const;
    void miniMap(Canvas&,const GameSession&) const;
    void menu(Canvas&,const GameSession&) const;
    void end(Canvas&,const GameSession&,bool victory) const;
    void overlay(Canvas&,const GameSession&) const;
};
}
