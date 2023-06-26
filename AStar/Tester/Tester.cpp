#include "Tester.h"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <omp.h>

#include "../Maze/Maze.h"
#include "../AStarCPU/AStarSeq.h"
#include "Timer.h"

#define FIXED_FLOAT(x) std::fixed << std::setprecision(3) << (x)

void to_json(json& j, const std::vector<Elem>& vec)
{
    for (const auto& obj : vec)
    {
        json jsonObj;
        obj.toJson(jsonObj);
        j.push_back(jsonObj);
    }
}

void Tester::generateTests()
{
	std::cout << "generating tests... ";
	std::filesystem::path path{ projectPath + "/tests/" };
	constexpr int startSize = 5;
	constexpr int endSize = 1005;
	constexpr int numOfTests = 100;
	constexpr int sizeInc = (endSize - startSize) / numOfTests;

	int size = startSize;
	for (int i = 0; i < numOfTests; ++i)
	{
		int rows = size;
		int cols = size;
		if (i % 3 == 0)
		{
			rows /= 2;
			cols *= 2;
		}
		else if (i % 3 == 1)
		{
			rows *= 2;
			cols /= 2;
		}
		Maze maze{ rows, cols };
		maze.generate();
		std::vector<std::vector<bool>> grid = maze.getMazeView();

		AStarSeq solver{ grid };
		solver.solve();
		std::vector<Elem> solution = solver.getSolution();

		int gridRows = grid.size();
		int gridCols = grid[0].size();
		std::stringstream filename;
		filename << "test" << gridRows << "x" << gridCols << ".bin";
		std::ofstream outputFile{ path.string() + filename.str(), std::ios::binary };

		outputFile.write(reinterpret_cast<const char*>(&gridRows), sizeof(gridRows));
		outputFile.write(reinterpret_cast<const char*>(&gridCols), sizeof(gridCols));
		for (int i = 0; i < gridRows; ++i)
		{
			for (int j = 0; j < gridCols; ++j)
			{
				bool field = grid[i][j];
				outputFile.write(reinterpret_cast<const char*>(&field), sizeof(field));
			}
		}
		int solutionSize = solution.size();
		outputFile.write(reinterpret_cast<const char*>(&solutionSize), sizeof(solutionSize));

		outputFile.close();
		size += sizeInc;
	}

	std::cout << "finished generating\n";
}

void Tester::loadTests()
{
	std::cout << "loading tests... ";
	std::filesystem::path path{ projectPath + "/tests/" };
	tests.clear();
	for (const auto& entry : std::filesystem::directory_iterator(path))
    {
		std::ifstream inputFile{ entry.path() };
		int gridRows = 0, gridCols = 0;
		inputFile.read(reinterpret_cast<char*>(&gridRows), sizeof(gridRows));
		inputFile.read(reinterpret_cast<char*>(&gridCols), sizeof(gridCols));

		Test test;
		test.grid = std::vector<std::vector<bool>>(gridRows, std::vector<bool>(gridCols, false));

		for (int i = 0; i < gridRows; ++i)
		{
			for (int j = 0; j < gridCols; ++j)
			{
				bool field;
				inputFile.read(reinterpret_cast<char*>(&field), sizeof(field));
				test.grid[i][j] = field;
			}
		}
		int solutionSize = 0;
		inputFile.read(reinterpret_cast<char*>(&solutionSize), sizeof(solutionSize));
		test.solutionSize = solutionSize;

		char dummy;
		inputFile.read(reinterpret_cast<char*>(&dummy), sizeof(dummy));

		if (!inputFile.eof())
			throw std::exception{};


		inputFile.close();
		tests.push_back(std::move(test));

		//break;
    }

	std::cout << "finished loading\n";
}

std::string testSolution(const Test& test, const std::vector<Elem>& solution)
{
	if (solution.size() > test.solutionSize)
	{
		return "to long";
	}
	else if (solution.size() < test.solutionSize)
	{
		return "to short";
	}
	else
	{
		for (int i = 0; i < solution.size(); ++i)
		{
			int x = solution[i].x;
			int y = solution[i].y;
			if (test.grid[y][x] != 0)
				return "has incorrect move";
		}
	}

	return "correct";
}

void Tester::runTestsSeq()
{
	if (tests.empty())
		loadTests();

	double fullTimer = 0.0;
	for (const Test& test : tests)
	{
		AStarSeq aStarSeq{ test.grid };
		Timer timer;
		aStarSeq.solve();
		auto elapsed = timer.elapsed();
		fullTimer += elapsed;
		std::cout << "Test " << test.grid.size() << "x" << test.grid[0].size()
			<< "\ttime:\t" << FIXED_FLOAT(elapsed) << "\tsolution:\t" << testSolution(test, aStarSeq.getSolution()) << '\n';
	}
	std::cout << "Total time: " << fullTimer;
}

void Tester::runTestsOMP()
{
	if (tests.empty())
		loadTests();

	double fullTimer = 0.0;
	int numThreads = 1;
#pragma omp parallel for schedule(dynamic, 1)
	for (int i = 0; i < tests.size(); ++i)
	{
		numThreads = omp_get_num_threads();
		AStarSeq aStarSeq{ tests[i].grid };
		Timer timer;
		aStarSeq.solve();
		double elapsed = timer.elapsed();
#pragma omp atomic
		fullTimer += elapsed;
#pragma omp critical
		{
			std::cout << "Test " << tests[i].grid.size() << "x" << tests[i].grid[0].size()
				<< "\ttime:\t" << FIXED_FLOAT(elapsed) << "\tsolution:\t" << testSolution(tests[i], aStarSeq.getSolution()) << '\n';
		}
	}
	std::cout << "Number of threads: " << numThreads << '\n';
	std::cout << "Total time: " << fullTimer / numThreads << '\n';
	std::cout << "Total CPU time: " << fullTimer << '\n';
}


void Tester::singleTest(int rows, int cols)
{
	Maze maze{rows, cols};
	maze.generate();
	std::vector<std::vector<bool>> grid = maze.getMazeView();
	
	AStarSeq aStarSeq{ grid };
	Timer timer;
	aStarSeq.solve();
	auto elapsed = timer.elapsed();
	std::cout << "Single test " << rows << "x" << cols << " time: " << elapsed << '\n';
}

void Tester::singleTestWebDump(int rows, int cols)
{
	Maze maze{rows, cols};
	maze.generate();
	std::vector<std::vector<bool>> grid = maze.getMazeView();
	
	AStarSeq aStarSeq{ grid };
	Timer timer;
	aStarSeq.solve();
	auto elapsed = timer.elapsed();
	std::cout << "Single test " << rows << "x" << cols << " time: " << elapsed << '\n';
	
	json jsonData;
	json jsonSolutionData;
	to_json(jsonSolutionData, aStarSeq.getSolution());
	json jsonPathData;
	to_json(jsonPathData, aStarSeq.getPath());

	jsonData = json{ {"grid", grid}, {"path", jsonPathData}, {"solution", jsonSolutionData} };

	std::string outputDir{ projectPath + "/Web" };
	std::ofstream outputFile(outputDir + "/maze.json");
	outputFile << jsonData.dump();
	outputFile.close();
};

