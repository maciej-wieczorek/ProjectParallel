#pragma once
#include <string>
#include <vector>

#include "../Maze/Maze.h"

extern std::string projectPath;

struct Test
{
	std::string filename;
	std::vector<std::vector<bool>> grid;
	int solutionSize;
};

class Tester
{
public:
	void generateTests(int startSize = 5, int endSize = 105, int numOfTests = 2000, bool purge = true);
	void loadTests();
	std::vector<std::vector<bool>> loadGrid(std::string filename);
	Test loadTest(std::string filename);
	double runTestsSeq(bool silent = false);
	double runTestsOMP(bool silent = false);
	double runTestsCU(bool silent = false);
	void runIncTestsSizeTest(int numTestsStart = 100, int numTestsEnd = 2200, int sizeInc = 300, int repeat = 5);
	void seqTestWithGrid(const std::vector<std::vector<bool>>& grid);
	void cudaTestWithGrid(const std::vector<std::vector<bool>>& grid);
	void seqTestRandomGrid(int rows, int cols);
	void cudaTestRandomGrid(int rows, int cols);
	void webDump(const std::vector<std::vector<bool>>& grid, const std::vector<Elem>& solution, const std::vector<Elem>& path);

private:
	std::vector<Test> tests;
};

