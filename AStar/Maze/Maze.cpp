#include "Maze.h"
#include <Windows.h>
#include <iostream>

void Maze::generate()
{
	maze = Grid(N, std::vector<bool>(M, false));
	Elem pos{ 0,0 };
	path = std::stack<Elem>{};
	std::stack<Elem> track;
	path.push(pos);
	track.push(pos);
	maze[0][0] = true;
	size_t visited = 1;

	while (visited != N * M)
	{

		std::vector<Elem> neighbours = getNeighbours(pos, maze);
		if (neighbours.size() > 0)
		{
			std::uniform_int_distribution<int> distribution(0, static_cast<int>(neighbours.size()) - 1);
			pos = neighbours[distribution(generator)];

			track.push(pos);
			maze[pos.y][pos.x] = true;
			++visited;
		}
		else
		{
			// backtrack
			track.pop();
			pos = track.top();
		}
		path.push(pos);
	}

}

Grid Maze::getMazeView() const
{
	Grid mazeView(2 * N + 1, std::vector<bool>(2 * M + 1, true));
	std::vector<Elem> pathView(path.size());
	std::stack<Elem> pathCopy{ path };
	while (!pathCopy.empty())
	{
		pathView[pathCopy.size() - 1] = pathCopy.top();
		pathCopy.pop();
	}

	mazeView[1][0] = 0;
	mazeView[mazeView.size() - 2][mazeView[0].size() - 1] = 0;
	for (size_t i = 0; i < pathView.size() - 1; ++i)
	{
		Elem pos = pathView[i];
		Elem nextPos = pathView[i + 1];

		mazeView[pos.y * 2 + 1][pos.x * 2 + 1] = 0;
		mazeView[nextPos.y * 2 + 1][nextPos.x * 2 + 1] = 0;

		int dx = nextPos.x - pos.x;
		int dy = nextPos.y - pos.y;

		if (dx == 1)
			mazeView[pos.y * 2 + 1][pos.x * 2 + 2] = 0;
		if (dx == -1)
			mazeView[pos.y * 2 + 1][pos.x * 2] = 0;
		if (dy == 1)
			mazeView[pos.y * 2 + 2][pos.x * 2 + 1] = 0;
		if (dy == -1)
			mazeView[pos.y * 2][pos.x * 2 + 1] = 0;
	}

	return mazeView;
}

std::vector<Elem> Maze::getNeighbours(Elem pos, const Grid& maze)
{
	std::vector<Elem> n;
	if (pos.x > 0 && maze[pos.y][pos.x - 1] == false)
		n.emplace_back(Elem{ pos.x - 1, pos.y });
	if (pos.x < M-1 && maze[pos.y][pos.x + 1] == false)
		n.emplace_back(Elem{ pos.x + 1, pos.y });
	if (pos.y > 0 && maze[pos.y - 1][pos.x] == false)
		n.emplace_back(Elem{ pos.x, pos.y - 1 });
	if (pos.y < N-1 && maze[pos.y + 1][pos.x] == false)
		n.emplace_back(Elem{ pos.x, pos.y + 1 });

	return n;
}

bool isOnPath(const std::vector<Elem>& path, size_t x, size_t y)
{
	for (const auto& elem : path)
	{
		if (elem.x == x && elem.y == y)
			return true;
	}

	return false;
}

void Maze::printMaze(const Grid& mazeView, std::vector<Elem> visited, std::vector<Elem> path)
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleOutputCP(CP_UTF8);
	CONSOLE_FONT_INFOEX fontInfo;
	fontInfo.cbSize = sizeof(CONSOLE_FONT_INFOEX);
	GetCurrentConsoleFontEx(hConsole, FALSE, &fontInfo);
	fontInfo.dwFontSize.X = 7;  // Set the desired width of the character
	fontInfo.dwFontSize.Y = 7;
	SetCurrentConsoleFontEx(hConsole, FALSE, &fontInfo);
	char charBlock[] = u8"\u2588";

	for (size_t i = 0; i < mazeView.size(); ++i)
	{
		for (size_t j = 0; j < mazeView[0].size(); ++j)
		{
			if (isOnPath(path, j, i))
			{
				SetConsoleTextAttribute(hConsole, 6);
			}
			else if (isOnPath(visited, j, i))
			{
				SetConsoleTextAttribute(hConsole, 2);
			}
			else if (mazeView[i][j])
			{
				SetConsoleTextAttribute(hConsole, 1);
			}
			else
			{
				SetConsoleTextAttribute(hConsole, 15);
			}

			std::cout << charBlock << charBlock;
		}
		std::cout << '\n';
	}
}

