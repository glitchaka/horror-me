#pragma once
#include "horror/Types.hpp"

namespace horror {
// Stateless procedural material and light sampler. Independent of the renderer,
// platform and game rules; all patterns are deterministic in world coordinates.
class MaterialArtist final {
public:
    [[nodiscard]] Color ceiling(Vec2 world,int act,double time,double pressure) const;
    [[nodiscard]] Color floor(Vec2 world,int act,double time,double pressure) const;
    [[nodiscard]] Color wall(int tileX,int tileY,double u,double v,int face,
                              double depth,int act,double time,double pressure) const;
private:
    [[nodiscard]] static double lighting(int x,int y,double time,double pressure,int act);
    [[nodiscard]] static Color tint(int r,int g,int b,int act,double light);
};
}
