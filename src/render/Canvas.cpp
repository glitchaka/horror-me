#include "horror/Canvas.hpp"
#include "horror/FontAtlas.hpp"
#include <algorithm>

namespace horror {
Canvas::Canvas():pixels_(ScreenW*ScreenH,0){}
void Canvas::clear(Color color){std::fill(pixels_.begin(),pixels_.end(),color);}
void Canvas::pixel(int x,int y,Color color) {
    if(x>=0&&x<ScreenW&&y>=0&&y<ScreenH)
        pixels_[static_cast<std::size_t>(y*ScreenW+x)]=color;
}
void Canvas::rect(int x,int y,int w,int h,Color color) {
    if(w<=0||h<=0)return;
    const int x0=std::clamp(x,0,ScreenW),x1=std::clamp(x+w,0,ScreenW);
    const int y0=std::clamp(y,0,ScreenH),y1=std::clamp(y+h,0,ScreenH);
    for(int yy=y0;yy<y1;++yy){
        auto start=pixels_.begin()+yy*ScreenW+x0;
        std::fill(start,start+(x1-x0),color);
    }
}
void Canvas::text(int x,int y,const std::string& text,Color color,int scale) {
    const auto& font=FontAtlas::instance();
    const bool large=scale>=2;
    const int height=font.height(large);
    for(char letter:text) {
        const int width=font.glyphWidth(letter,large);
        if(x+width<0){x+=width+1;continue;}
        if(x>=ScreenW)break;
        for(int yy=0;yy<height;++yy){
            const int py=y+yy;
            if(py<0||py>=ScreenH)continue;
            for(int xx=0;xx<width;++xx){
                const int px=x+xx;
                if(px<0||px>=ScreenW)continue;
                // Hard coverage at the logical pixel grid: the font
                // remains Annapurna SIL, but no semitransparent halo is
                // enlarged into visibly blurred screen pixels.
                if(font.alpha(letter,large,xx,yy)<2)continue;
                pixels_[static_cast<std::size_t>(py*ScreenW+px)]=color;
            }
        }
        x+=width+1;
    }
}
void Canvas::center(int y,const std::string& value,Color color,int scale) {
    const int width=FontAtlas::instance().textWidth(value,scale>=2);
    text((ScreenW-width)/2,y,value,color,scale);
}
}
