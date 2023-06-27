#pragma once
#include "../Maze/Maze.h"

class AStarCU
{
public:
    AStarCU(const std::vector<std::vector<bool>>& _grid);
    ~AStarCU() { delete grid; }
    void solve();
    std::vector<Elem> getSolution() { return solution; };
    std::vector<Elem> getPath() { return path; };
private:
    std::vector<std::vector<bool>> gridVec;
    bool* grid = nullptr;
    const int N;
    const int M;
    std::vector<Elem> path;
    std::vector<Elem> solution;
};

