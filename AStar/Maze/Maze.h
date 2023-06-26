#pragma once
#include <vector>
#include <random>
#include <stack>
#include "../thirdparty/json.hpp"

using json = nlohmann::json;

using Grid = std::vector<std::vector<bool>>;
struct Elem 
{
	int x;
	int y;

	void toJson(json& j) const
	{
		j = json{ {"x", x}, {"y", y} };
	}
};

class Maze
{
public:
	Maze(int _N, int _M) : N{ _N }, M{ _M }, rd{}, generator(rd())
	{
	}

    void generate();
	Grid getMazeView() const;
	static void printMaze(const Grid& mazeView,
		std::vector<Elem> visited = std::vector<Elem>{}, std::vector<Elem> path = std::vector<Elem>{});

private:
	std::vector<Elem> getNeighbours(Elem pos, const Grid& maze);

	int N;
	int M;
	std::random_device rd;
	std::mt19937 generator;
	Grid maze;
	std::stack<Elem> path;
};

