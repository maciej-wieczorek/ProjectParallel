
#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <stdio.h>

#include "../Maze/Maze.h"

struct Node
{
    int x, y, h;
};

__device__ int heuristic(int row, int col, int goalRow, int goalCol)
{
    return abs(row - goalRow) + abs(col - goalCol);
}

__device__ Node getBestAndErase(Node* list, int size)
{
    int idx = 0;
    Node bestNode = list[idx];
    for (int i = 0; i < size; ++i)
    {
        if (list[i].h < bestNode.h)
        {
            bestNode = list[i];
            idx = i;
        }
    }

    for (int i = idx + 1; i < size; ++i)
    {
        list[i - 1] = list[i];
    }

    return bestNode;
}

__device__ void reverse(Elem* list, int size)
{
    int start = 0;
    int end = size - 1;
    while (start < end)
    {
        Elem temp = list[start];
        list[start] = list[end];
        list[end] = temp;

        ++start;
        --end;
    }
}

struct AStarArgs
{
    unsigned int N;
    unsigned int M;
    bool* grid;
    bool* closed;
    Node* open;
    Elem* track;
    Elem* solution;
    Elem* path;
};

__global__ void AStarKernel(AStarArgs* args, int argsSize)
{
    int tid = blockDim.x * blockIdx.x + threadIdx.x;

    if (tid >= argsSize)
        return;

    unsigned int N = args[tid].N;
    unsigned int M = args[tid].M;
    bool* grid = args[tid].grid;
    bool* closed = args[tid].closed;
    Node* open = args[tid].open;
    Elem* track = args[tid].track;
    Elem* solution = args[tid].solution;
    Elem* path = args[tid].path;

    int startX = 0;
    int startY = 1;
    int targetX = M - 1;
    int targetY = N - 2;
    int openSize = 0;
    int pathSize = 0;

    Node start{ startX, startY, heuristic(startX, startY, targetX, targetY) };
    open[openSize] = start;
    ++openSize;

    while (openSize > 0)
    {
        Node X = getBestAndErase(open, openSize);
        --openSize;
        path[pathSize] = Elem{ X.x, X.y };
        ++pathSize;

        if (X.x == targetX && X.y == targetY)
        {
            Elem elem{ X.x, X.y };
            int solutionSize = 0;
            while (!(elem.x == startX && elem.y == startY))
            {
                solution[solutionSize] = elem;
                ++solutionSize;
                elem = track[M * elem.y + elem.x];
            }
			solution[solutionSize] = elem;
            ++solutionSize;
            reverse(solution, solutionSize);

            break;
        }

        if (X.x > 0)
        {
            int x = X.x - 1;
            int y = X.y;
            if (grid[M * y + x] == 0 && !closed[M * y + x])
            {
                track[M * y + x] = Elem{ X.x, X.y };
                open[openSize] = Node{ x, y, heuristic(x, y, targetX, targetY) };
                ++openSize;
            }
        }
        if (X.x < M - 1)
        {
            int x = X.x + 1;
            int y = X.y;
            if (grid[M * y + x] == 0 && !closed[M * y + x])
            {
                track[M * y + x] = Elem{ X.x, X.y };
                open[openSize] = Node{ x, y, heuristic(x, y, targetX, targetY) };
                ++openSize;
            }
        }
        if (X.y > 0)
        {
            int x = X.x;
            int y = X.y - 1;
            if (grid[M * y + x] == 0 && !closed[M * y + x])
            {
                track[M * y + x] = Elem{ X.x, X.y };
                open[openSize] = Node{ x, y, heuristic(x, y, targetX, targetY) };
                ++openSize;
            }
        }
        if (X.y < N - 1)
        {
            int x = X.x;
            int y = X.y + 1;
            if (grid[M * y + x] == 0 && !closed[M * y + x])
            {
                track[M * y + x] = Elem{ X.x, X.y };
                open[openSize] = Node{ x, y, heuristic(x, y, targetX, targetY) };
                ++openSize;
            }
        }

        closed[M * X.y + X.x] = true;
    }
}


void dispachAStarCU(const std::vector<const Grid*>& grids, std::vector<std::vector<Elem>>& paths, std::vector<std::vector<Elem>>& solutions)
{
    bool* dev_grid;
    bool* dev_closed;
    Node* dev_open;
    Elem* dev_track;
    Elem* dev_solution;
    Elem* dev_path;
    AStarArgs* dev_args;

    cudaError_t cudaStatus;
    unsigned int offset = 0;
    std::vector<AStarArgs> args;

    int numOfThreads = 1;
    int numOfBlocks = 1;

    if (grids.size() > 32)
    {
        numOfThreads = 32;
        numOfBlocks = std::ceil((double)grids.size() / (double)numOfThreads);
    }
    else
    {
        numOfThreads = grids.size();
    }
    
    unsigned int size = 0;
    for (const Grid* grid : grids)
    {
        unsigned int N = grid->size();
        unsigned int M = grid->at(0).size();
		size += N * M;
    }


    cudaStatus = cudaSetDevice(0);
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaSetDevice failed!  Do you have a CUDA-capable GPU installed?");
		goto Error;
	}
		
	cudaStatus = cudaMalloc((void**)&dev_grid, size * sizeof(bool));
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaMalloc failed!");
		goto Error;
	}

	cudaStatus = cudaMalloc((void**)&dev_closed, size * sizeof(bool));
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaMalloc failed!");
		goto Error;
	}

	cudaStatus = cudaMalloc((void**)&dev_open, size * sizeof(Node));
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaMalloc failed!");
		goto Error;
	}

	cudaStatus = cudaMalloc((void**)&dev_track, size * sizeof(Elem));
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaMalloc failed!");
		goto Error;
	}

	cudaStatus = cudaMalloc((void**)&dev_solution, size * sizeof(Elem));
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaMalloc failed!");
		goto Error;
	}

	cudaStatus = cudaMalloc((void**)&dev_path, size * sizeof(Elem));
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaMalloc failed!");
		goto Error;
	}

    for (const Grid* grid : grids)
    {
        unsigned int N = grid->size();
        unsigned int M = grid->at(0).size();
		unsigned int size = N * M;

        AStarArgs arg;
        arg.N = N;
        arg.M = M;
        arg.grid = dev_grid + offset;
        arg.closed = dev_closed + offset;
        arg.open = dev_open + offset;
        arg.track = dev_track + offset;
        arg.solution = dev_solution + offset;
        arg.path = dev_path + offset;

        bool* gridCpy = new bool[size];
        for (size_t i = 0; i < N; ++i)
        {
            for (size_t j = 0; j < M; ++j)
            {
                gridCpy[M * i + j] = grid->at(i)[j];
            }
        }

		cudaStatus = cudaMemcpy(arg.grid, gridCpy, size * sizeof(bool), cudaMemcpyHostToDevice);
		if (cudaStatus != cudaSuccess) {
			fprintf(stderr, "cudaMemcpy failed!");
			goto Error;
		}

        args.push_back(arg);
        delete[] gridCpy;
        offset += size;
    }
    
    cudaStatus = cudaMalloc((void**)&dev_args, args.size() * sizeof(AStarArgs));
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaMalloc failed!");
		goto Error;
	}

	cudaStatus = cudaMemcpy(dev_args, args.data(), args.size() * sizeof(AStarArgs), cudaMemcpyHostToDevice);
	if (cudaStatus != cudaSuccess) {
		fprintf(stderr, "cudaMemcpy failed!");
		goto Error;
	}

    // Launch a kernel on the GPU with one thread for each element.
    AStarKernel<<<numOfBlocks, numOfThreads>>>(dev_args, args.size());

    // Check for any errors launching the kernel
    cudaStatus = cudaGetLastError();
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "AStarKernel launch failed: %s\n", cudaGetErrorString(cudaStatus));
        goto Error;
    }
    
    // cudaDeviceSynchronize waits for the kernel to finish, and returns
    // any errors encountered during the launch.
    cudaStatus = cudaDeviceSynchronize();
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaDeviceSynchronize returned error code %d after launching kernel!\n", cudaStatus);
        goto Error;
    }

    for (size_t i = 0; i < args.size(); ++i)
    {
		// Copy solution from GPU buffer to host memory.
        unsigned int size = args[i].N * args[i].M;
		cudaStatus = cudaMemcpyAsync(solutions[i].data(), args[i].solution, size * sizeof(Elem), cudaMemcpyDeviceToHost);
		if (cudaStatus != cudaSuccess) {
			fprintf(stderr, "cudaMemcpy failed!");
			goto Error;
		}

		cudaStatus = cudaMemcpyAsync(paths[i].data(), args[i].path, size * sizeof(Elem), cudaMemcpyDeviceToHost);
		if (cudaStatus != cudaSuccess) {
			fprintf(stderr, "cudaMemcpy failed!");
			goto Error;
		}
    }

Error:
    cudaFree(dev_grid);
    cudaFree(dev_closed);
    cudaFree(dev_open);
    cudaFree(dev_track);
    cudaFree(dev_solution);
    cudaFree(dev_path);
    cudaFree(dev_args);
    
    if (cudaStatus != cudaSuccess)
    {
        fprintf(stderr, "AStarCU failed!");
    }

    cudaStatus = cudaDeviceReset();
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaDeviceReset failed!");
    }
}
