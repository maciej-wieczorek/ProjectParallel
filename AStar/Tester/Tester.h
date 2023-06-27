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
	void seqTest(int rows, int cols);
	void seqTestWebDump(int rows, int cols);
	void cudaTestWebDump(int rows, int cols);
	void webDump(const std::vector<std::vector<bool>>& grid, const std::vector<Elem>& solution, const std::vector<Elem>& path);

private:
	std::vector<Test> tests;
};

