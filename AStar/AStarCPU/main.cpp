#include <iostream>
#include <vector>
#include <set>
#include <cassert>
#include <fstream>

#include "../Maze/Maze.h"
#include "../thirdparty/json.hpp"
#include "AStarSeq.h"


#define STRINGIFY(x) #x

#define EXPAND(x) STRINGIFY(x)

#ifdef PROJECT_PATH
    std::string projectPath = EXPAND(PROJECT_PATH);
#else
    std::string projectPath{ "path/to/project" };
#endif

using json = nlohmann::json;

void to_json(json& j, const std::vector<Elem>& vec)
{
    for (const auto& obj : vec)
    {
        json jsonObj;
        obj.toJson(jsonObj);
        j.push_back(jsonObj);
    }
}

int main(char* argc, char** argv) {
#ifdef PROJECT_PATH
    projectPath.erase(0, 1); // erase the first quote
    projectPath.erase(projectPath.size() - 2); // erase the last quote and the dot
#endif // PROJECT_PATH

        
    Maze maze{ (size_t)atoi(argv[1]), (size_t)atoi(argv[2])};
    maze.generate();
    std::vector<std::vector<bool>> grid = maze.getMazeView();

    //Maze::printMaze(grid);
    //std::cout << "\n\n";

    AStarSeq aStarSeq{ grid };
   
    //Maze::printMaze(grid, path, solution);

    json jsonData;
    json jsonSolutionData;
    to_json(jsonSolutionData, aStarSeq.solve());
    json jsonPathData;
    to_json(jsonPathData, aStarSeq.getPath());

    jsonData = json{ {"grid", grid}, {"path", jsonPathData}, {"solution", jsonSolutionData} };

    std::string outputDir{ projectPath + "/Web" };
    std::ofstream outputFile(outputDir + "/maze.json");
    outputFile << jsonData.dump();
    outputFile.close();

    return 0;
}
