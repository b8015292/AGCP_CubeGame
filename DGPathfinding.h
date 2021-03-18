#pragma once

#pragma once

#include <math.h>
#include <vector>
#include <array>
#include <stack>
#include <vector>
#include "WorldManager.h"

#define MAX_X 12
#define MAX_Y 12
#define MAX_Z 12

struct Node
{
public:
	int x = 0;
	int y = 0;
	int z = 0;
	int parentX = -1;
	int parentY = -1;
	int parentZ = 0;
	float gCost = FLT_MAX;
	float hCost = FLT_MAX;
	float fCost = FLT_MAX;
};

inline bool operator < (const Node& lhs, const Node& rhs)
{
	return lhs.fCost < rhs.fCost;
}

class DGPathfinding
{
public:
	DGPathfinding(std::shared_ptr<WorldManager>);

	std::vector<Node> AStar(Node start, Node destination);

private:
	std::shared_ptr<WorldManager> worldManager;

	bool IsValid(int x, int y, int z);
	bool IsDestination(int x, int y, int z, Node destination);
	double CalculateH(int x, int y, int z, Node destination);

	std::vector<Node> MakePath(std::array<std::array<std::array<Node, MAX_Z>, MAX_Y>, MAX_X> map, Node destination);
};