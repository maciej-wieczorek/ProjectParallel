#pragma once
#include <set>
#include "../Maze/Maze.h"

class Node
{
public:
    int x, y;
    int h;

    bool operator<(const Node& other) const
    {
        return h < other.h;
    }
};

class AStarSeq
{
public:
    AStarSeq(const std::vector<std::vector<bool>>& _grid);
    void solve();
    std::vector<Elem> getSolution() { return solution; };
    std::vector<Elem> getPath() { return path; };
private:
    std::vector<std::vector<bool>> grid;
    const int N;
    const int M;
    int startX;
    int startY;
    int targetX;
    int targetY;
    static constexpr int INF = std::numeric_limits<int>::max();
    std::vector<std::vector<int>> metricsOpen;
    std::vector<std::vector<int>> metricsClosed;
    std::vector<std::vector<Elem>> track;
    std::set<Node> open;
	std::set<Node> closed;
    std::vector<Elem> path;
    std::vector<Elem> solution;
};

