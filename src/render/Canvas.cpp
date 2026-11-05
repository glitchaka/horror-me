#include "horror/Canvas.hpp"
#include <algorithm>
#include <array>
#include <cctype>
#include <cstdint>

namespace horror {
namespace {
using Glyph=std::array<std::uint8_t,7>;
const Glyph& glyph(char character) {
    static constexpr Glyph blank{};
    static constexpr Glyph A{14,17,17,31,17,17,17},B{30,17,17,30,17,17,30};
    static constexpr Glyph C{15,16,16,16,16,16,15},D{30,17,17,17,17,17,30};
    static constexpr Glyph E{31,16,16,30,16,16,31},F{31,16,16,30,16,16,16};
    static constexpr Glyph G{15,16,16,23,17,17,15},H{17,17,17,31,17,17,17};
    static constexpr Glyph I{31,4,4,4,4,4,31},J{7,2,2,2,18,18,12};
    static constexpr Glyph K{17,18,20,24,20,18,17},L{16,16,16,16,16,16,31};
    static constexpr Glyph M{17,27,21,21,17,17,17},N{17,25,25,21,19,19,17};
    static constexpr Glyph O{14,17,17,17,17,17,14},P{30,17,17,30,16,16,16};
    static constexpr Glyph Q{14,17,17,17,21,18,13},R{30,17,17,30,20,18,17};
    static constexpr Glyph S{15,16,16,14,1,1,30},T{31,4,4,4,4,4,4};
    static constexpr Glyph U{17,17,17,17,17,17,14},V{17,17,17,17,17,10,4};
    static constexpr Glyph W{17,17,17,21,21,21,10},X{17,17,10,4,10,17,17};
    static constexpr Glyph Y{17,17,10,4,4,4,4},Z{31,1,2,4,8,16,31};
    static constexpr Glyph N0{14,17,19,21,25,17,14},N1{4,12,4,4,4,4,14};
    static constexpr Glyph N2{14,17,1,2,4,8,31},N3{30,1,1,14,1,1,30};
    static constexpr Glyph N4{2,6,10,18,31,2,2},N5{31,16,16,30,1,1,30};
    static constexpr Glyph N6{14,16,16,30,17,17,14},N7{31,1,2,4,8,8,8};
    static constexpr Glyph N8{14,17,17,14,17,17,14},N9{14,17,17,15,1,1,14};
    static constexpr Glyph Dot{0,0,0,0,0,12,12},Colon{0,12,12,0,12,12,0};
    static constexpr Glyph Dash{0,0,0,31,0,0,0},Slash{1,1,2,4,8,16,16};
    static constexpr Glyph Bang{4,4,4,4,4,0,4},Question{14,17,1,2,4,0,4};
    static constexpr Glyph Plus{0,4,4,31,4,4,0},Equal{0,31,0,31,0,0,0};
    static constexpr Glyph Left{14,8,8,8,8,8,14},Right{14,2,2,2,2,2,14};
    static constexpr Glyph Percent{17,2,4,8,17,0,0},Comma{0,0,0,0,12,12,8};
    static constexpr Glyph Bar{4,4,4,4,4,4,4},Under{0,0,0,0,0,0,31};
    if(character>='a'&&character<='z') character=char(character-32);
    switch(character) {
    case 'A':return A;case 'B':return B;case 'C':return C;case 'D':return D;
    case 'E':return E;case 'F':return F;case 'G':return G;case 'H':return H;
    case 'I':return I;case 'J':return J;case 'K':return K;case 'L':return L;
    case 'M':return M;case 'N':return N;case 'O':return O;case 'P':return P;
    case 'Q':return Q;case 'R':return R;case 'S':return S;case 'T':return T;
    case 'U':return U;case 'V':return V;case 'W':return W;case 'X':return X;
    case 'Y':return Y;case 'Z':return Z;
    case '0':return N0;case '1':return N1;case '2':return N2;case '3':return N3;
    case '4':return N4;case '5':return N5;case '6':return N6;case '7':return N7;
    case '8':return N8;case '9':return N9;
    case '.':return Dot;case ':':return Colon;case '-':return Dash;
    case '/':return Slash;case '!':return Bang;case '?':return Question;
    case '+':return Plus;case '=':return Equal;case '[':return Left;
    case ']':return Right;case '%':return Percent;case ',':return Comma;
    case '|':return Bar;case '_':return Under;
    default:return blank;
    }
}
}
Canvas::Canvas():pixels_(ScreenW*ScreenH,0){}
void Canvas::clear(Color color) { std::fill(pixels_.begin(),pixels_.end(),color); }
void Canvas::pixel(int x,int y,Color color) {
    if(x>=0&&x<ScreenW&&y>=0&&y<ScreenH)
        pixels_[static_cast<std::size_t>(y*ScreenW+x)]=color;
}
void Canvas::rect(int x,int y,int width,int height,Color color) {
    if(width<=0||height<=0) return;
    const int x0=std::clamp(x,0,ScreenW),x1=std::clamp(x+width,0,ScreenW);
    const int y0=std::clamp(y,0,ScreenH),y1=std::clamp(y+height,0,ScreenH);
    for(int yy=y0;yy<y1;++yy) {
        auto first=pixels_.begin()+yy*ScreenW+x0;
        std::fill(first,first+(x1-x0),color);
    }
}
void Canvas::text(int x,int y,const std::string& value,Color color,int scale) {
    if(scale<1)return;
    for(char character:value) {
        const Glyph& bits=glyph(character);
        for(int row=0;row<7;++row)
            for(int col=0;col<5;++col)
                if(bits[row]&(1<<(4-col)))
                    rect(x+col*scale,y+row*scale,scale,scale,color);
        x+=6*scale;
    }
}
void Canvas::center(int y,const std::string& value,Color color,int scale) {
    text((ScreenW-int(value.size())*6*scale)/2,y,value,color,scale);
}
}
