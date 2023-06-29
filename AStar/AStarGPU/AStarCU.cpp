#include "AStarCU.h"
#include <iostream>
AStarCU::AStarCU(std::vector<const Grid*>& _grids) :
	grids{ _grids },
	paths(grids.size(), std::vector<Elem>{}),
	solutions(grids.size(), std::vector<Elem>{})
{
	for (size_t i = 0; i < grids.size(); ++i)
	{
		const Grid* grid = grids[i];
		unsigned int size = grid->size() * grid->at(0).size();
		paths[i] = std::vector<Elem>(size, Elem{ 0, 0 });
		solutions[i] = std::vector<Elem>(size, Elem{ 0, 0 });
	}
}

void dispachAStarCU(const std::vector<const Grid*>& grids, std::vector<std::vector<Elem>>& paths, std::vector<std::vector<Elem>>& solutions);

void AStarCU::solve()
{
    dispachAStarCU(grids, paths, solutions);

	for (size_t i = 0; i < solutions.size(); ++i)
	{
		std::vector<Elem> trimedSolution;
		for (size_t j = 0; j < solutions[i].size(); ++j)
		{
			if (solutions[i][j].x == 0 && solutions[i][j].y == 0)
			{
				break;
			}
			else
			{
				trimedSolution.push_back(solutions[i][j]);
			}
		}

		solutions[i] = trimedSolution;
	}
	
	for (size_t i = 0; i < paths.size(); ++i)
	{
		std::vector<Elem> trimedPath;
		for (size_t j = 0; j < paths[i].size(); ++j)
		{
			if (paths[i][j].x == 0 && paths[i][j].y == 0)
			{
				break;
			}
			else
			{
				trimedPath.push_back(paths[i][j]);
			}
		}

		paths[i] = trimedPath;
	}
}
