#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace horror {
inline constexpr int ScreenW=384;
inline constexpr int ScreenH=216;
inline constexpr int MapW=23;
inline constexpr int MapH=23;
inline constexpr double Pi=3.14159265358979323846;
inline constexpr double FieldOfViewScale=.68;

using Color=std::uint32_t;
struct Vec2 { double x{}, y{}; };

inline double distance(Vec2 a,Vec2 b) { return std::hypot(a.x-b.x,a.y-b.y); }
inline double length(Vec2 v) { return std::hypot(v.x,v.y); }
inline int cell(double v) { return int(std::floor(v)); }
inline double wrapAngle(double a) {
    while(a>Pi) a-=2*Pi;
    while(a<-Pi) a+=2*Pi;
    return a;
}
inline constexpr Color rgb(int r,int g,int b) {
    return (Color(std::clamp(r,0,255))<<16) |
           (Color(std::clamp(g,0,255))<<8) |
           Color(std::clamp(b,0,255));
}
inline Color shade(Color c,double factor) {
    return rgb(int(((c>>16)&255)*factor),int(((c>>8)&255)*factor),int((c&255)*factor));
}
inline std::uint32_t noiseHash(int x,int y,int seed=0) {
    std::uint32_t v=std::uint32_t(x)*374761393u+
                    std::uint32_t(y)*668265263u+std::uint32_t(seed);
    v=(v^(v>>13))*1274126177u;
    return v^(v>>16);
}
}
