#include <iostream>
#include <vector>
#include <set>
#include <cassert>
#include <fstream>

#include "../Maze/Maze.h"
#include "json.hpp"

using json = nlohmann::json;

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

void to_json(json& j, const std::vector<Elem>& vec)
{
    for (const auto& obj : vec)
    {
        json jsonObj;
        obj.toJson(jsonObj);
        j.push_back(jsonObj);
    }
}

int main(char* argc, char** argv) {
    Maze maze{ (size_t)atoi(argv[1]), (size_t)atoi(argv[2])};
    maze.generate();
    std::vector<std::vector<bool>> grid = maze.getMazeView();

    //Maze::printMaze(grid);
    //std::cout << "\n\n";

    const int N = grid.size();
    const int M = grid[0].size();
    constexpr int INF = std::numeric_limits<int>::max();
    
    std::vector<std::vector<int>> metricsOpen(N, std::vector<int>(M, INF));
    std::vector<std::vector<int>> metricsClosed{ metricsOpen };
    std::vector<std::vector<Elem>> track(N, std::vector<Elem>(M, Elem{0, 0}));


    int startX = 0;
    int startY = 1;
    int targetX = M-1;
    int targetY = N-2;

    std::set<Node> open;
    std::set<Node> closed;

    Node start{ startX, startY, 0, h(startX, startY, targetX, targetY) };
    open.insert(start);
    metricsOpen[start.y][start.x] = start.g + start.h;
    std::vector<Elem> path;
    std::vector<Elem> solution;

    while (!open.empty())
    {
        Node X = *open.begin();
        open.erase(open.begin());
        metricsOpen[X.y][X.x] = INF;
        path.push_back(Elem{ (size_t)X.x, (size_t)X.y });

        if (X.x == targetX && X.y == targetY)
        {
            std::stack<Elem> buildSol;
            Elem elem{ X.x, X.y };
            while (!(elem.x == startX && elem.y == startY))
            {
                buildSol.push(elem);
                elem = track[elem.y][elem.x];
            }
            solution.push_back(elem);
            while (!buildSol.empty())
            {
                solution.push_back(buildSol.top());
                buildSol.pop();
            }

            break;
        }

        std::vector<Node> children;
        if (X.x > 0)
        {
            int x = X.x - 1;
            int y = X.y;
            if (grid[y][x] == 0)
            {
                children.push_back(Node{ x, y, X.g + 1, h(x, y, targetX, targetY)});
            }
        }
        if (X.x < M-1)
        {
            int x = X.x + 1;
            int y = X.y;
            if (grid[y][x] == 0)
            {
                children.push_back(Node{ x, y, X.g + 1, h(x, y, targetX, targetY)});
            }
        }
        if (X.y > 0)
        {
            int x = X.x;
            int y = X.y - 1;
            if (grid[y][x] == 0)
            {
                children.push_back(Node{ x, y, X.g + 1, h(x, y, targetX, targetY)});
            }
        }
        if (X.y < N-1)
        {
            int x = X.x;
            int y = X.y + 1;
            if (grid[y][x] == 0)
            {
                children.push_back(Node{ x, y, X.g + 1, h(x, y, targetX, targetY)});
            }
        }

        for (const Node& child : children)
        {
            bool isOnOpen = metricsOpen[child.y][child.x] == INF ? false : true;
            bool isOnClosed = metricsClosed[child.y][child.x] == INF ? false : true;
            if (!isOnOpen && !isOnClosed)
            {
                track[child.y][child.x] = Elem{ (size_t)X.x, (size_t)X.y };
                metricsOpen[child.y][child.x] = child.g + child.h;
                open.insert(child);
            }
            else if (isOnOpen)
			{
                int f = metricsOpen[child.y][child.x];
                if (child.g + child.h < f)
                {
					track[child.y][child.x] = Elem{ (size_t)X.x, (size_t)X.y };
                    metricsOpen[child.y][child.x] = child.g + child.h;

                    auto iter = open.find(Node{ child.x, child.y, f });
                    open.erase(iter);
                    open.insert(Node{ child.x, child.y, child.g + child.h });
                }
			}
            else
			{
                int f = metricsClosed[child.y][child.x];
                if (child.g + child.h < f)
                {
					track[child.y][child.x] = Elem{ (size_t)X.x, (size_t)X.y };
                    metricsClosed[child.y][child.x] = INF;
                    metricsOpen[child.y][child.x] = child.g + child.h;

                    auto iter = closed.find(Node{ child.x, child.y, f });
                    closed.erase(iter);
                    open.insert(Node{ child.x, child.y, child.g + child.h });
                }
			}
        }

        metricsClosed[X.y][X.x] = X.g + X.h;
        closed.insert(X);
    }

    //Maze::printMaze(grid, path, solution);

    json jsonData;
    json jsonPathData;
    to_json(jsonPathData, path);
    json jsonSolutionData;
    to_json(jsonSolutionData, solution);

    jsonData = json{ {"grid", grid}, {"path", jsonPathData}, {"solution", jsonSolutionData} };

    std::ofstream outputFile("maze.json");
    outputFile << jsonData.dump();
    outputFile.close();

    return 0;
}
