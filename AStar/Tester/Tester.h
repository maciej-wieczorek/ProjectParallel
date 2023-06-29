#pragma once
#include <string>
#include <vector>

#include "../Maze/Maze.h"

extern std::string projectPath;

struct Test
{
	std::vector<std::vector<bool>> grid;
	int solutionSize;
};

class Tester
{
public:
	void generateTests();
	void loadTests();
	void runTestsSeq();
	void runTestsOMP();
	void runTestsCU();
	void seqTestWithGrid(const std::vector<std::vector<bool>>& grid);
	void cudaTestWithGrid(const std::vector<std::vector<bool>>& grid);
	void seqTestRandomGrid(int rows, int cols);
	void cudaTestRandomGrid(int rows, int cols);
	void webDump(const std::vector<std::vector<bool>>& grid, const std::vector<Elem>& solution, const std::vector<Elem>& path);

private:
	std::vector<Test> tests;
};

