#include "AStarCU.h"

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
            grid[i * N + j] = gridVec[i][j];
        }
    }
}

void dispachAStarCU(unsigned int N, unsigned int M, bool* grid, Elem* solution, Elem* path);

void AStarCU::solve()
{
    std::vector<Elem> oversizedSolution(N * M, Elem{ -1, - 1 });
    std::vector<Elem> oversizedPath(N * M, Elem{ -1, - 1 });

    dispachAStarCU(N, M, grid, oversizedSolution.data(), oversizedPath.data());

    for (Elem elem : oversizedSolution)
    {
        if (elem.x == -1)
            break;
        else
            solution.push_back(elem);
    }

    for (Elem elem : oversizedPath)
    {
        if (elem.x == -1)
            break;
        else
            path.push_back(elem);
    }
}
