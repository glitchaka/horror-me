#include "horror/Maze.hpp"
#include <algorithm>
#include <utility>

namespace horror {
void Maze::regenerate(std::mt19937& random) {
    for(auto& row:cells_) row.fill('#');
    std::vector<std::pair<int,int>> stack{{1,1}};
    cells_[1][1]='.';
    constexpr int dx[4]={2,-2,0,0};
    constexpr int dy[4]={0,0,2,-2};
    while(!stack.empty()) {
        const auto [x,y]=stack.back();
        std::array<int,4> options{0,1,2,3};
        std::shuffle(options.begin(),options.end(),random);
        bool extended=false;
        for(int direction:options) {
            const int nx=x+dx[direction],ny=y+dy[direction];
            if(nx<=0||nx>=MapW-1||ny<=0||ny>=MapH-1||cells_[ny][nx]=='.')
                continue;
            cells_[y+dy[direction]/2][x+dx[direction]/2]='.';
            cells_[ny][nx]='.';
            stack.emplace_back(nx,ny);
            extended=true;
            break;
        }
        if(!extended) stack.pop_back();
    }
    std::uniform_int_distribution<int> rx(2,MapW-3),ry(2,MapH-3);
    for(int i=0;i<25;++i) {
        const int x=rx(random),y=ry(random);
        if(cells_[y][x]!='#') continue;
        if((open(x-1,y)&&open(x+1,y)) || (open(x,y-1)&&open(x,y+1)))
            cells_[y][x]='.';
    }
}
bool Maze::wall(int x,int y) const {
    return x<0||x>=MapW||y<0||y>=MapH||cells_[y][x]=='#';
}
bool Maze::circleFree(double x,double y,double radius) const {
    return !wall(cell(x-radius),cell(y-radius))
        && !wall(cell(x+radius),cell(y-radius))
        && !wall(cell(x-radius),cell(y+radius))
        && !wall(cell(x+radius),cell(y+radius));
}
bool Maze::visible(Vec2 a,Vec2 b) const {
    const double d=distance(a,b);
    const int steps=std::max(2,int(d*14));
    for(int i=1;i<steps;++i) {
        const double t=double(i)/steps;
        if(wall(cell(a.x+(b.x-a.x)*t),cell(a.y+(b.y-a.y)*t)))
            return false;
    }
    return true;
}
std::vector<Vec2> Maze::floors() const {
    std::vector<Vec2> result;
    result.reserve(MapW*MapH/2);
    for(int y=1;y<MapH-1;++y)
        for(int x=1;x<MapW-1;++x)
            if(open(x,y)) result.push_back({x+.5,y+.5});
    return result;
}
Vec2 Maze::nearest(Vec2 target) const {
    const auto cells=floors();
    if(cells.empty()) return {1.5,1.5};
    return *std::min_element(cells.begin(),cells.end(),[&](Vec2 a,Vec2 b){
        return distance(a,target)<distance(b,target);
    });
}
Vec2 Maze::farthest(Vec2 target) const {
    const auto cells=floors();
    if(cells.empty()) return {1.5,1.5};
    return *std::max_element(cells.begin(),cells.end(),[&](Vec2 a,Vec2 b){
        return distance(a,target)<distance(b,target);
    });
}
}
