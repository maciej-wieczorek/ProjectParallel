#include "Tester.h"

#include <algorithm>

#define STRINGIFY(x) #x

#define EXPAND(x) STRINGIFY(x)

#ifdef PROJECT_PATH
    std::string projectPath = EXPAND(PROJECT_PATH);
#else
    std::string projectPath{ "path/to/project" };
#endif

int main()
{
#ifdef PROJECT_PATH
    projectPath.erase(0, 1); // erase the first quote
    projectPath.erase(projectPath.size() - 2); // erase the last quote and the dot
    size_t pos = projectPath.find("\\");
    while (pos != std::string::npos)
    {
        projectPath.replace(pos, 1, 1, '/');
        pos = projectPath.find("\\", pos + 1);
    }
#endif // PROJECT_PATH

	Tester tester;
	//tester.singleTestWebDump(400, 800);
    //tester.generateTests();
    tester.runTestsOMP();

	return 0;
}