
#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <stdio.h>

#include "../Maze/Maze.h"

struct Node
{
    int x, y, h;
};

struct SetNode
{
    Node node;
    SetNode* left;
    SetNode* right;
};

__device__ SetNode* createNode(Node node)
{
    SetNode* newNode = new SetNode;
    newNode->node = node;
    newNode->left = newNode->right = NULL;
    return newNode;
}

__device__ SetNode* insert(SetNode* root, Node node)
{
    if (root == NULL) {
        return createNode(node);
    }

    if (node.h < root->node.h) {
        root->left = insert(root->left, node);
    } 
    root->right = insert(root->right, node);

    return root;
}

__device__ bool contains(SetNode* root, Node node)
{
    if (root == NULL) {
        return false;
    }

    if (node.x == root->node.x && node.y == root->node.y && node.h == root->node.h) {
        return true;
    }

    if (node.h < root->node.h) {
        return contains(root->left, node);
    } else {
        return contains(root->right, node);
    }
}

__device__ void deleteSet(SetNode* root)
{
    if (root != NULL) {
        deleteSet(root->left);
        deleteSet(root->right);
        delete root;
    }
}

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

__global__ void AStarKernel(unsigned int N, unsigned int M, bool* grid, bool* closed, Node* open, Elem* track, Elem* solution, Elem* path)
{
    int startX = 0;
    int startY = 1;
    int targetX = M - 1;
    int targetY = N - 2;
    int openSize = 0;
    int pathSize = 0;

    Node start{ startX, startY, heuristic(startX, startY, targetX, targetY) };
    open[openSize] = start;
    ++openSize;

    bool found = false;
    while (openSize > 0)
    {
        Node X = getBestAndErase(open, openSize);
        --openSize;
        path[pathSize] = Elem{ X.x, X.y };
        ++pathSize;

        if (X.x == targetX && X.y == targetY)
        {
            found = true;
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
    
    if (!found)
        printf("No solution found\n");
}

struct AStarArgs
{
    unsigned int N;
    unsigned int M;
    bool* brid;
    bool* closed;
    Node* open;
    Elem* track;
    Elem* solution;
    Elem* path;
};

void dispachAStarCU(unsigned int N, unsigned int M, bool* grid, Elem* solution, Elem* path)
{
    unsigned int size = N * M;
    bool* dev_grid = 0;
    bool* dev_closed;
    Node* dev_open;
    Elem* dev_track;
    Elem* dev_solution;
    Elem* dev_path;
    cudaError_t cudaStatus;

    cudaStream_t stream;
    cudaStreamCreate(&stream);

    // Choose which GPU to run on
    cudaStatus = cudaSetDevice(0);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaSetDevice failed!  Do you have a CUDA-capable GPU installed?");
        goto Error;
    }

    // Allocate GPU buffers for three vectors (one input, two output)
    cudaStatus = cudaMallocAsync((void**)&dev_grid, size * sizeof(bool), stream);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMalloc failed!");
        goto Error;
    }

    cudaStatus = cudaMallocAsync((void**)&dev_closed, size * sizeof(bool), stream);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMalloc failed!");
        goto Error;
    }

    cudaStatus = cudaMallocAsync((void**)&dev_open, size * sizeof(Node), stream);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMalloc failed!");
        goto Error;
    }

    cudaStatus = cudaMallocAsync((void**)&dev_track, size * sizeof(Elem), stream);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMalloc failed!");
        goto Error;
    }

    cudaStatus = cudaMallocAsync((void**)&dev_solution, size * sizeof(Elem), stream);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMalloc failed!");
        goto Error;
    }

    cudaStatus = cudaMallocAsync((void**)&dev_path, size * sizeof(Elem), stream);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMalloc failed!");
        goto Error;
    }

    // Copy input vectors from host memory to GPU buffers.
    cudaStatus = cudaMemcpyAsync(dev_grid, grid, size * sizeof(bool), cudaMemcpyHostToDevice, stream);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMemcpy failed!");
        goto Error;
    }

    // Launch a kernel on the GPU with one thread for each element.
    AStarKernel<<<1, 1, 0, stream>>>(N, M, dev_grid, dev_closed, dev_open, dev_track, dev_solution, dev_path);

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

    // Copy solution from GPU buffer to host memory.
    cudaStatus = cudaMemcpyAsync(solution, dev_solution, size * sizeof(Elem), cudaMemcpyDeviceToHost, stream);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMemcpy failed!");
        goto Error;
    }

    cudaStatus = cudaMemcpyAsync(path, dev_path, size * sizeof(Elem), cudaMemcpyDeviceToHost, stream);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMemcpy failed!");
        goto Error;
    }

Error:
    cudaFreeAsync(dev_grid, stream);
    cudaFreeAsync(dev_closed, stream);
    cudaFreeAsync(dev_open, stream);
    cudaFreeAsync(dev_track, stream);
    cudaFreeAsync(dev_solution, stream);
    cudaFreeAsync(dev_path, stream);

    cudaStreamDestroy(stream);
    
    if (cudaStatus != cudaSuccess)
    {
        fprintf(stderr, "AStarCU failed!");
    }
}
