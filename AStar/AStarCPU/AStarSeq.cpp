#include "AStarSeq.h"

AStarSeq::AStarSeq(const std::vector<std::vector<bool>>& _grid) :
    grid{ _grid },
    N{ static_cast<int>(grid.size()) },
    M{ static_cast<int>(grid[0].size()) },
    startX{ 0 },
    startY{ 1 },
    targetX{ M - 1 },
    targetY{ N - 2 }
{

}

int h(int x1, int y1, int x2, int y2)
{
    return abs(x2 - x1) + abs(y2 - y1);
}


void AStarSeq::solve()
{
    path.clear();
    solution.clear();

    track = std::vector<std::vector<Elem>>(N, std::vector<Elem>(M, Elem{ 0, 0 }));
    closed = std::vector<std::vector<bool>>(N, std::vector<bool>(M, false));

	Node start{ startX, startY, h(startX, startY, targetX, targetY) };
    open.insert(start);

    while (!open.empty())
    {
        Node X = *open.begin();
        open.erase(open.begin());
        path.push_back(Elem{ X.x, X.y });

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
                children.push_back(Node{ x, y, h(x, y, targetX, targetY)});
            }
        }
        if (X.x < M-1)
        {
            int x = X.x + 1;
            int y = X.y;
            if (grid[y][x] == 0)
            {
                children.push_back(Node{ x, y, h(x, y, targetX, targetY)});
            }
        }
        if (X.y > 0)
        {
            int x = X.x;
            int y = X.y - 1;
            if (grid[y][x] == 0)
            {
                children.push_back(Node{ x, y, h(x, y, targetX, targetY)});
            }
        }
        if (X.y < N-1)
        {
            int x = X.x;
            int y = X.y + 1;
            if (grid[y][x] == 0)
            {
                children.push_back(Node{ x, y, h(x, y, targetX, targetY)});
            }
        }

        for (const Node& child : children)
        {
            if (!closed[child.y][child.x])
            {
                track[child.y][child.x] = Elem{ X.x, X.y };
                open.insert(child);
            }
        }

        closed[X.y][X.x] = true;
    }
}

