#pragma once
#include "horror/Types.hpp"
#include <algorithm>

namespace horror {
struct PixelViewport {
    int x{};
    int y{};
    int width{};
    int height{};
    int scale{1};
};
// Every source pixel must occupy a square of integer physical pixels.
// Unused client area is letterboxed, never blurred by fractional scaling.
[[nodiscard]] inline PixelViewport calculatePixelViewport(int clientWidth,int clientHeight) {
    const int w=std::max(1,clientWidth),h=std::max(1,clientHeight);
    const int scale=std::max(1,std::min(w/ScreenW,h/ScreenH));
    const int width=ScreenW*scale,height=ScreenH*scale;
    return {(w-width)/2,(h-height)/2,width,height,scale};
}
}
