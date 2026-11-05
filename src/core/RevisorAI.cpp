#include "horror/RevisorAI.hpp"
#include <algorithm>
#include <array>
#include <queue>
#include <utility>

namespace horror {
void RevisorAI::update(const Maze& maze,Vec2 player,double pressure,
                        int completed,double dt) {
    clock_-=std::clamp(dt,0.0,.05);
    if(clock_>0)return;
    clock_=std::max(0.0,clock_+.10);
    const int sx=cell(position_.x),sy=cell(position_.y);
    const int gx=cell(player.x),gy=cell(player.y);
    if(!maze.open(sx,sy)||!maze.open(gx,gy)) {
        relocate(maze.nearest(position_));
        return;
    }
    std::array<std::array<int,MapW>,MapH> distances{};
    for(auto& row:distances)row.fill(-1);
    std::queue<std::pair<int,int>> queue;
    queue.emplace(gx,gy);
    distances[gy][gx]=0;
    constexpr int dx[4]={1,-1,0,0};
    constexpr int dy[4]={0,0,1,-1};
    while(!queue.empty()) {
        const auto [x,y]=queue.front();
        queue.pop();
        for(int direction=0;direction<4;++direction) {
            const int xx=x+dx[direction],yy=y+dy[direction];
            if(!maze.open(xx,yy)||distances[yy][xx]!=-1)continue;
            distances[yy][xx]=distances[y][x]+1;
            queue.emplace(xx,yy);
        }
    }
    const int here=distances[sy][sx];
    if(here<0) {
        relocate(maze.nearest(position_));
        return;
    }
    Vec2 target=here==0?player:Vec2{sx+.5,sy+.5};
    int best=here;
    for(int direction=0;direction<4;++direction) {
        const int x=sx+dx[direction],y=sy+dy[direction];
        if(maze.open(x,y)&&distances[y][x]>=0&&distances[y][x]<best) {
            best=distances[y][x];
            target={x+.5,y+.5};
        }
    }
    const Vec2 difference{target.x-position_.x,target.y-position_.y};
    const double magnitude=length(difference);
    if(magnitude<=1e-6)return;
    // Become faster when seen, pressure is high, or the last act begins.
    const double sight=maze.visible(position_,player)?1.17:1.0;
    const double velocity=(.85+completed*.17+pressure*.0035)*sight;
    const double step=std::min(velocity*.1,magnitude);
    const Vec2 next{position_.x+difference.x*step/magnitude,
                    position_.y+difference.y*step/magnitude};
    if(maze.circleFree(next.x,next.y,.12))position_=next;
}
}
