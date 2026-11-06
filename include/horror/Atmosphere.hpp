#pragma once
#include "horror/Canvas.hpp"

namespace horror {
// Framebuffer effects applied to the 3D world before the readable HUD.
class Atmosphere final {
public:
    void apply(Canvas& canvas,int act,double pressure,double time,double shake) const;
};
}
