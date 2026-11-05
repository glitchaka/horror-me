#include "horror/Maze.hpp"
#include <algorithm>
#include <queue>
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
    for(int i=0;i<78;++i) {
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
int Maze::stepsBetween(Vec2 from,Vec2 to) const {
    const int sx=cell(from.x),sy=cell(from.y),tx=cell(to.x),ty=cell(to.y);
    if(!open(sx,sy)||!open(tx,ty))return -1;
    std::array<std::array<int,MapW>,MapH> steps{};
    for(auto& row:steps)row.fill(-1);
    std::queue<std::pair<int,int>> q;
    q.emplace(sx,sy);
    steps[sy][sx]=0;
    constexpr int dx[4]={1,-1,0,0},dy[4]={0,0,1,-1};
    while(!q.empty()) {
        auto [x,y]=q.front();q.pop();
        if(x==tx&&y==ty)return steps[y][x];
        for(int direction=0;direction<4;++direction) {
            const int xx=x+dx[direction],yy=y+dy[direction];
            if(!open(xx,yy)||steps[yy][xx]!=-1)continue;
            steps[yy][xx]=steps[y][x]+1;
            q.emplace(xx,yy);
        }
    }
    return -1;
}
void Maze::distort(std::mt19937& random,
                   const std::vector<Vec2>& protectedPositions,int changes) {
    // Only alter unoccupied cells, never strand terminals or split the maze.
    const auto connected=[&]() {
        Vec2 origin{};
        int openCount=0;
        for(int y=1;y<MapH-1;++y)for(int x=1;x<MapW-1;++x) {
            if(open(x,y)){++openCount;origin={x+.5,y+.5};}
        }
        if(openCount==0)return false;
        std::array<std::array<bool,MapW>,MapH> seen{};
        std::queue<std::pair<int,int>> queue;
        queue.emplace(cell(origin.x),cell(origin.y));
        seen[cell(origin.y)][cell(origin.x)]=true;
        int reached=0;
        constexpr int dx[4]={1,-1,0,0},dy[4]={0,0,1,-1};
        while(!queue.empty()) {
            const auto [x,y]=queue.front();queue.pop();++reached;
            for(int dir=0;dir<4;++dir) {
                const int xx=x+dx[dir],yy=y+dy[dir];
                if(!open(xx,yy)||seen[yy][xx])continue;
                seen[yy][xx]=true;
                queue.emplace(xx,yy);
            }
        }
        return reached==openCount;
    };
    std::uniform_int_distribution<int> randX(2,MapW-3),randY(2,MapH-3);
    int accepted=0;
    for(int attempt=0;attempt<250&&accepted<changes;++attempt) {
        const int x=randX(random),y=randY(random);
        bool protectedCell=false;
        for(const Vec2 position:protectedPositions) {
            if(distance({x+.5,y+.5},position)<1.45) {
                protectedCell=true;break;
            }
        }
        if(protectedCell)continue;
        const bool wasOpen=open(x,y);
        if(!wasOpen) {
            // Open only a useful connection between existing passages.
            if(!((open(x-1,y)&&open(x+1,y))||
                 (open(x,y-1)&&open(x,y+1))))continue;
            cells_[y][x]='.';
            ++accepted;
        } else {
            // Closing a corridor is allowed only if the entire map
            // remains connected. Revert instantly on failure.
            cells_[y][x]='#';
            if(!connected()){cells_[y][x]='.';continue;}
            ++accepted;
        }
    }
}

}
