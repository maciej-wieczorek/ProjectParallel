#pragma once
#include <string>
#include <vector>

extern std::string projectPath;

struct Test
{
	std::vector<std::vector<bool>> grid;
	int solutionSize;
};

class Tester
{
public:
	void loadBin() {};
	void loadJson() {};
	void dumpBin() {};
	void dumpJson() {};
	void generateTests();
	void loadTests();
	void runTests();
	void singleTest(int rows, int cols);
	void singleTestWebDump(int rows, int cols);

private:
	std::vector<Test> tests;
};

