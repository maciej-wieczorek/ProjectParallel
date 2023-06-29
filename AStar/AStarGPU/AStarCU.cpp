#include "AStarCU.h"
#include <iostream>
AStarCU::AStarCU(const std::vector<std::vector<bool>>& _grid) :
	gridVec{ _grid },
	N{ static_cast<int>(gridVec.size()) },
	M{ static_cast<int>(gridVec[0].size()) }
{
	grid = new bool[N * M];
    for (size_t i = 0; i < N; ++i)
    {
        for (size_t j = 0; j < M; ++j)
        {
            grid[i * M + j] = gridVec[i][j];
        }
    }
}

void dispachAStarCU(unsigned int N, unsigned int M, bool* grid, Elem* solution, Elem* path);

void AStarCU::solve()
{
    std::vector<Elem> oversizedSolution(N * M, Elem{ 0, 0 });
    std::vector<Elem> oversizedPath(N * M, Elem{ 0, 0 });

    dispachAStarCU(N, M, grid, oversizedSolution.data(), oversizedPath.data());

    for (Elem elem : oversizedSolution)
    {
        if (elem.x == 0 && elem.y == 0)
            break;
        else
            solution.push_back(elem);
    }

    for (Elem elem : oversizedPath)
    {
        if (elem.x == 0 && elem.y == 0)
            break;
        else
            path.push_back(elem);
    }
}
