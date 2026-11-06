#pragma once
#include "horror/PixelViewport.hpp"
#include <vector>

namespace horror {
// Builds a native-resolution window buffer with strict nearest-neighbor
// integer enlargement and black letterboxing; no Win32 or GPU dependencies.
class PixelPresenter final {
public:
    [[nodiscard]] const std::vector<Color>& compose(const Color* source,
                                                     int clientWidth,int clientHeight);
    [[nodiscard]] PixelViewport viewport() const { return viewport_; }
private:
    PixelViewport viewport_{};
    std::vector<Color> output_;
};
}
