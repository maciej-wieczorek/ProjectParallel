#include "Tester.h"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <omp.h>
#include <thread>
#include <mutex>
#include <iomanip>

#include "Timer.h"
#include "../AStarCPU/AStarSeq.h"
#include "../AStarGPU/AStarCU.h"

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

	for (const auto& entry : std::filesystem::directory_iterator(path)) 
        std::filesystem::remove_all(entry.path());


	constexpr int startSize = 5;
	constexpr int endSize = 105;
	constexpr int numOfTests = 2000;
	constexpr int sizeInc = (endSize - startSize) / numOfTests;
	constexpr int incEvery = numOfTests / (endSize - startSize);

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
		filename << "test-" << i << "-" << gridRows << "x" << gridCols << ".bin";
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
		if (solutionSize < rows || solutionSize < cols)
			throw std::exception{};
		outputFile.write(reinterpret_cast<const char*>(&solutionSize), sizeof(solutionSize));

		outputFile.close();

		if (sizeInc > 0)
		{
			size += sizeInc;
		}
		else
		{
			if (i % incEvery == 0)
				++size;
		}
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
		test.filename = entry.path().filename().string();
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

std::vector<std::vector<bool>> Tester::loadGrid(std::string filename)
{
	std::filesystem::path path{ projectPath + "/tests/" + filename };
	std::ifstream inputFile{ path };
	int gridRows = 0, gridCols = 0;
	inputFile.read(reinterpret_cast<char*>(&gridRows), sizeof(gridRows));
	inputFile.read(reinterpret_cast<char*>(&gridCols), sizeof(gridCols));

	std::vector<std::vector<bool>> grid(gridRows, std::vector<bool>(gridCols, false));

	for (int i = 0; i < gridRows; ++i)
	{
		for (int j = 0; j < gridCols; ++j)
		{
			bool field;
			inputFile.read(reinterpret_cast<char*>(&field), sizeof(field));
			grid[i][j] = field;
		}
	}

	inputFile.close();

	return grid;
}

Test Tester::loadTest(std::string filename)
{
	std::filesystem::path path{ projectPath + "/tests/" + filename };
	std::ifstream inputFile{ path };
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

	return test;
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
		std::cout << "Test " << std::setw(30) << test.filename << "\ttime:\t" << FIXED_FLOAT(elapsed)
			<< "\tsolution: " << testSolution(test, aStarSeq.getSolution()) << '\n';
	}
	std::cout << "Total time: " << fullTimer << '\n';
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
			std::cout << "Test " << std::setw(30) << tests[i].filename << "\ttime:\t" << FIXED_FLOAT(elapsed)
				<< "\tsolution: " << testSolution(tests[i], aStarSeq.getSolution()) << '\n';
		}
	}
	std::cout << "Number of threads: " << numThreads << '\n';
	std::cout << "Total time: " << fullTimer / numThreads << '\n';
	std::cout << "Total CPU time: " << fullTimer << '\n';
}

void Tester::runTestsCU()
{
	if (tests.empty())
		loadTests();

	std::vector<const Grid*> grids;
	for (const Test& test : tests)
	{
		grids.push_back(&test.grid);
	}
	
	AStarCU aStarCU{ grids };
	Timer timer;
	aStarCU.solve();
	double elapsed = timer.elapsed();

	for (size_t i = 0; i < tests.size(); ++i)
	{
		std::cout << "Test " << std::setw(30) << tests[i].filename << "\tsolution: " <<
			testSolution(tests[i], aStarCU.getSolution(i)) << '\n';
	}

	std::cout << "Total time: " << elapsed << '\n';
}

void Tester::seqTestWithGrid(const std::vector<std::vector<bool>>& grid)
{
	int rows = grid.size();
	int cols = grid[0].size();
	AStarSeq aStarSeq{ grid };
	Timer timer;
	aStarSeq.solve();
	auto elapsed = timer.elapsed();
	std::cout << "Single test " << rows << "x" << cols << " time: " << elapsed << '\n';

	webDump(grid, aStarSeq.getSolution(), aStarSeq.getPath());
}

void Tester::cudaTestWithGrid(const std::vector<std::vector<bool>>& grid)
{
	int rows = grid.size();
	int cols = grid[0].size();
	std::vector<const Grid*> grids;
	grids.push_back(&grid);
	AStarCU aStarCU{ grids };
	Timer timer;
	aStarCU.solve();
	auto elapsed = timer.elapsed();
	std::cout << "Single test " << rows << "x" << cols << " time: " << elapsed << '\n';

	webDump(grid, aStarCU.getSolution(0), aStarCU.getPath(0));
}

void Tester::seqTestRandomGrid(int rows, int cols)
{
	Maze maze{rows, cols};
	maze.generate();
	std::vector<std::vector<bool>> grid = maze.getMazeView();
	
	AStarSeq aStarSeq{ grid };
	Timer timer;
	aStarSeq.solve();
	auto elapsed = timer.elapsed();
	std::cout << "Single test " << rows << "x" << cols << " time: " << elapsed << '\n';
	
	webDump(grid, aStarSeq.getSolution(), aStarSeq.getPath());
}
void Tester::cudaTestRandomGrid(int rows, int cols)
{
	Maze maze{rows, cols};
	maze.generate();
	std::vector<std::vector<bool>> grid = maze.getMazeView();
	
	std::vector<const Grid*> grids;
	grids.push_back(&grid);
	AStarCU aStarCU{ grids };
	Timer timer;
	aStarCU.solve();
	auto elapsed = timer.elapsed();
	std::cout << "Single test " << rows << "x" << cols << " time: " << elapsed << '\n';
	
	webDump(grid, aStarCU.getSolution(0), aStarCU.getPath(0));
}

void Tester::webDump(const std::vector<std::vector<bool>>& grid, const std::vector<Elem>& solution, const std::vector<Elem>& path)
{
	json jsonData;
	json jsonSolutionData;
	to_json(jsonSolutionData, solution);
	json jsonPathData;
	to_json(jsonPathData, path);

	jsonData = json{ {"grid", grid}, {"path", jsonPathData}, {"solution", jsonSolutionData} };

	std::string outputDir{ projectPath + "/Web" };
	std::ofstream outputFile(outputDir + "/maze.json");
	outputFile << jsonData.dump();
	outputFile.close();
}

