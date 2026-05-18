#ifndef GAMELOGIC_H
#define GAMELOGIC_H
#include <vector>

struct PathNode{
    int row;
    int col;
};

struct GraphNode{
    int row;
    int col;
    int dir;
    int turns;
    std::vector<PathNode> pathHistory;
    GraphNode(int r,int c,int d,int t,std::vector<PathNode> path):row(r),col(c),dir(d),turns(t),pathHistory(path){};
};

class GameLogic
{
public:
    GameLogic();
    std::vector<std::vector<int>>* map=NULL;
    void initMap();
    //bool checkPath(int r1,int c1,int r2,int c2);
    bool Nocorner(int r1,int c1,int r2,int c2);
    bool Onecorner(int r1,int c1,int r2,int c2);
    bool Twocorner(int r1,int c1,int r2,int c2);
    std::vector<PathNode> currentPath;
    bool checkPath_Graph(int r1,int c1,int r2,int c2);
};

#endif // GAMELOGIC_H
