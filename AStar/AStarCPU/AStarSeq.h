#pragma once
#include <set>
#include "../Maze/Maze.h"

int h(int x1, int y1, int x2, int y2)
{
    return abs(x2 - x1) + abs(y2 - y1);
}

class Node
{
public:
    int x, y;
    int g;
    int h;

    bool operator==(const Node& other) const
    {
        return x == other.x && y == other.y;
    }

    bool operator<(const Node& other) const
    {
        if (g + h < other.g + other.h)
        {
            return true;
        }
        else if (g + h == other.g + other.h)
        {
            if (h < other.h)
            {
                return true;
            }
            else if (other.h == h)
            {
                if (x != other.x)
                    return x < other.x;
                else
                    return y < other.y;

                // not complete
            }
            else
            {
                return false;
            }
        }
        else
        {
            return false;
        }
        
    }
};

class AStarSeq
{
public:
    AStarSeq(const std::vector<std::vector<bool>>& _grid);
    std::vector<Elem> solve();
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
};

