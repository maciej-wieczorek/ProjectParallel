#pragma once
#include <vector>
#include <random>
#include <stack>
#include "../AStar/json.hpp"

using json = nlohmann::json;

using Grid = std::vector<std::vector<bool>>;
struct Elem 
{
	size_t x;
	size_t y;

	void toJson(json& j) const
	{
		j = json{ {"x", x}, {"y", y} };
	}
};

class Maze
{
public:
	Maze(size_t _N, size_t _M) : N{ _N }, M{ _M }, rd{}, generator(rd())
	{
	}

    void generate();
	Grid getMazeView() const;
	static void printMaze(const Grid& mazeView,
		std::vector<Elem> visited = std::vector<Elem>{}, std::vector<Elem> path = std::vector<Elem>{});

private:
	std::vector<Elem> getNeighbours(Elem pos, const Grid& maze);

	size_t N = 20;
	size_t M = 20;
	std::random_device rd;
	std::mt19937 generator;
	Grid maze;
	std::stack<Elem> path;
};

