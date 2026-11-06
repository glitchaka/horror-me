#pragma once
#include "horror/Types.hpp"
#include <string>
#include <vector>

namespace horror {
class Canvas final {
public:
    Canvas();
    void clear(Color color);
    void pixel(int x,int y,Color color);
    void rect(int x,int y,int w,int h,Color color);
    void text(int x,int y,const std::string& value,Color color,int scale=1);
    void center(int y,const std::string& value,Color color,int scale=1);
    [[nodiscard]] const Color* data() const { return pixels_.data(); }
    [[nodiscard]] Color* mutableData() { return pixels_.data(); }
private:
    std::vector<Color> pixels_;
};
}
