#pragma once
#include "../Maze/Maze.h"

using Grid = std::vector<std::vector<bool>>;

class AStarCU
{
public:
    AStarCU(std::vector<const Grid*>& _grid);
    void solve();
    std::vector<Elem> getSolution(size_t i) { return solutions[i]; };
    std::vector<Elem> getPath(size_t i) { return paths[i]; };
private:
    std::vector<const Grid*> grids;
    std::vector<std::vector<Elem>> paths;
    std::vector<std::vector<Elem>> solutions;
};

