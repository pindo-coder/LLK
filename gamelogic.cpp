#include "gamelogic.h"
#include <algorithm>
#include <queue>
using namespace std;


GameLogic::GameLogic() {}
void GameLogic::initMap(){

}
//bool GameLogic::checkPath(int r1,int c1,int r2,int c2){
//    if(Nocorner(r1,c1,r2,c2))return true;
//    else if(Onecorner(r1,c1,r2,c2))return true;
//    else if(Twocorner(r1,c1,r2,c2))return true;
//    else return false;
//}
bool GameLogic::Nocorner(int r1,int c1,int r2,int c2){
    if(r1==r2){
        int minC=min(c1,c2);
        int maxC=max(c1,c2);
        for(int i=minC+1;i<maxC;i++){
            if((*map)[r1][i]!=0)return false;
        }
        currentPath.clear();
        currentPath.push_back({r1,c1});
        currentPath.push_back({r2,c2});
        return true;
    }
    if(c1==c2){
        int minR=min(r1,r2);
        int maxR=max(r1,r2);
        for(int i=minR+1;i<maxR;i++){
            if((*map)[i][c1]!=0)return false;
        }
        currentPath.clear();
        currentPath.push_back({r1,c1});
        currentPath.push_back({r2,c2});
        return true;
    }
    return false;
}
bool GameLogic::Onecorner(int r1,int c1,int r2,int c2){
    if((*map)[r1][c2]==0&&Nocorner(r1,c1,r1,c2)&&Nocorner(r1,c2,r2,c2)){
        currentPath.clear();
        currentPath.push_back({r1,c1});
        currentPath.push_back({r1,c2});
        currentPath.push_back({r2,c2});
        return true;
    }
    else if((*map)[r2][c1]==0&&Nocorner(r1,c1,r2,c1)&&Nocorner(r2,c1,r2,c2)){
        currentPath.clear();
        currentPath.push_back({r1,c1});
        currentPath.push_back({r2,c1});
        currentPath.push_back({r2,c2});
        return true;
    }
    else{
        return false;
    }
}
bool GameLogic::Twocorner(int r1,int c1,int r2,int c2){
    for(int i=0;i<(*map).size();i++){
        if(i==r1)continue;
        if((*map)[i][c1]!=0)continue;
        if(Nocorner(r1,c1,i,c1)==false)continue;
        if(Onecorner(i,c1,r2,c2)){
            currentPath.insert(currentPath.begin(),{r1,c1});
            return true;
        }
    }
    for(int i=0;i<(*map).size();i++){
        if(i==c1)continue;
        if((*map)[r1][i]!=0)continue;
        if(Nocorner(r1,c1,r1,i)==false)continue;
        if(Onecorner(r1,i,r2,c2)){
            currentPath.insert(currentPath.begin(),{r1,c1});
            return true;
        }
    }
    return false;
}
bool GameLogic::checkPath_Graph(int r1,int c1,int r2,int c2){
    if (r1 == r2 && c1 == c2) return false;
    // 1. 准备 BFS 队列
    std::queue<GraphNode> q;
    // 2. 访问标记数组：记录到达该点时的“最小拐弯次数”了）
    std::vector<std::vector<int>> minTurns(map->size(), std::vector<int>((*map)[0].size(), 3));
    std::vector<PathNode> startPath;
    startPath.push_back(PathNode{r1,c1});
    // 初始点入队（初始方向设为 -1，拐弯设为 0）
    q.push(GraphNode(r1, c1, -1, 0,startPath));
    minTurns[r1][c1] = 0;
    // 方向向量：上下左右
    int dr[] = {-1, 1, 0, 0};
    int dc[] = {0, 0, -1, 1};

    while (!q.empty()) {
        GraphNode curr = q.front();
        q.pop();
        for (int i = 0; i < 4; ++i) {
            int nr = curr.row + dr[i];
            int nc = curr.col + dc[i];
            // 越界检查
            if (nr < 0 || nr >= map->size() || nc < 0 || nc >= (*map)[0].size()) continue;
            // 计算如果往这个方向走，拐弯次数是多少
            int nextTurns = curr.turns;
            bool isCornor=false;
            if (curr.dir != -1 && curr.dir != i) {
                nextTurns++;
                isCornor=true;
            }
            // 核心约束：拐弯不能超过 2 次
            if (nextTurns > 2) continue;
            std::vector<PathNode> nextPath=curr.pathHistory;
            if(isCornor){
                nextPath.push_back(PathNode{curr.row,curr.col});
            }
            // 碰到目标点
            if (nr == r2 && nc == c2) {
                nextPath.push_back(PathNode{r2,c2});
                currentPath.clear();
                for(auto p:nextPath){
                    currentPath.push_back(PathNode{p.row,p.col});
                }
                return true;
            }
            // 碰到障碍物（非目标点的其他方块）
            if ((*map)[nr][nc] != 0) continue;
            // 图优化：如果到达当前点的拐弯次数，比之前记录的还要少，才继续搜索
            if (nextTurns <= minTurns[nr][nc]) {
                minTurns[nr][nc] = nextTurns;
                q.push(GraphNode(nr, nc, i, nextTurns,nextPath));
            }
        }
    }
    return false;
}




















