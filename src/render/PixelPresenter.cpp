#include "horror/PixelPresenter.hpp"
#include <algorithm>
#include <cstring>

namespace horror {
const std::vector<Color>& PixelPresenter::compose(const Color* source,
                                                   int clientWidth,int clientHeight) {
    const int width=std::max(1,clientWidth),height=std::max(1,clientHeight);
    viewport_=calculatePixelViewport(width,height);
    output_.resize(static_cast<std::size_t>(width)*height);
    std::fill(output_.begin(),output_.end(),Color{0});
    const int size=viewport_.scale;
    for(int sourceY=0;sourceY<ScreenH;++sourceY) {
        const int baseY=viewport_.y+sourceY*size;
        // Off-screen cropping is only needed on displays smaller than
        // the logical 384x216 framebuffer.
        if(baseY>=height||baseY+size<=0)continue;
        const int srcOffset=sourceY*ScreenW;
        for(int sourceX=0;sourceX<ScreenW;++sourceX) {
            const int baseX=viewport_.x+sourceX*size;
            if(baseX>=width||baseX+size<=0)continue;
            const int left=std::max(0,baseX);
            const int right=std::min(width,baseX+size);
            const int top=std::max(0,baseY);
            const int bottom=std::min(height,baseY+size);
            const Color color=source[srcOffset+sourceX];
            for(int y=top;y<bottom;++y)
                std::fill_n(output_.data()+static_cast<std::size_t>(y)*width+left,
                            right-left,color);
        }
    }
    return output_;
}
}
