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
    int x;
    int y;
    int z;
    int parentX;
    int parentY;
    int parentZ;
    float gCost;
    float hCost;
    float fCost;
};

inline bool operator < (const Node& lhs, const Node& rhs)
{
    return lhs.fCost < rhs.fCost;
}

class Pathfinding
{
public:

	std::shared_ptr<WorldManager> worldManager;

	bool isValid(int x, int y, int z) { //If our Node is an obstacle it is not valid
		if (!worldManager->GetBlock(XMFLOAT3{(float)x, (float)y, (float)z})->GetActive()) {
			if (!worldManager->GetBlock(XMFLOAT3{ (float)x, (float)y + 1, (float)z })->GetActive()) {
				if (worldManager->GetBlock(XMFLOAT3{ (float)x, (float)y - 2, (float)z })->GetActive()) {
					return true;
				}
			}
		}
		return false;
	}

    bool isDestination(int x, int y, int z, Node destination)
    {
        if (x == destination.x && y == destination.y && z == destination.z) {
            return true;
        }
        return false;
    }

    double calculateH(int x, int y, int z, Node destination) 
    {
        double H = sqrt((x - destination.x) * (x - destination.x) + (y - destination.y) * (y - destination.y) + (z - destination.z) * (z - destination.z));
        return H;
    }

    std::vector<Node> makePath(std::array<std::array<std::array<Node, MAX_Z>, MAX_Y>, MAX_X> map, Node destination)
    {
		int x = destination.x;
		int y = destination.y;
		int z = destination.z;
		std::stack<Node>* path = new std::stack<Node>();
		std::vector<Node> usablePath;

		while (!(map[x][y][z].parentX == x && map[x][y][z].parentY == y && map[x][y][z].parentZ  == z)
			&& map[x][y][z].x != -1 && map[x][y][z].y != -1 && map[x][y][z].z != -1)
		{
			path->push(map[x][y][z]);
			int tempX = map[x][y][z].parentX;
			int tempY = map[x][y][z].parentY;
			int tempZ = map[x][y][z].parentZ;
			x = tempX;
			y = tempY;
			z = tempZ;

		}
		path->push(map[x][y][z]);

		while (!path->empty()) {
			Node top = path->top();
			path->pop();
			usablePath.emplace_back(top);
		}
		delete path;
		return usablePath;
	}

	std::vector<Node> aStar(Node start, Node destination) {
		std::vector<Node> empty;

		//TODO: Check if the destination is not air		

		//TOOD: Check if the destination is the same as the start
		
		bool closedList[MAX_X][MAX_Y][MAX_Z];

		//Initialize whole map
		//Node allMap[50][25];
		std::array<std::array<std::array<Node, MAX_Z>, MAX_Y>, MAX_X> allMap;
		for (int x = start.x - MAX_X / 2; x < start.x + MAX_X / 2; x++) {
			for (int y = start.y - MAX_Y / 2; y < start.y + MAX_Y / 2; y++) {
				for (int z = start.z - MAX_Z / 2; z < start.z + MAX_Z / 2; z++) {
					allMap[x][y][z].fCost = FLT_MAX;
					allMap[x][y][z].gCost = FLT_MAX;
					allMap[x][y][z].hCost = FLT_MAX;
					allMap[x][y][z].parentX = -1;
					allMap[x][y][z].parentY = -1;
					allMap[x][y][z].x = x;
					allMap[x][y][z].y = y;

					closedList[x][y][z] = false;
				}
			}
		}

		//Initialize our starting list
		int x = start.x;
		int y = start.y;
		int z = start.z;
		allMap[x][y][z].fCost = 0.0;
		allMap[x][y][z].gCost = 0.0;
		allMap[x][y][z].hCost = 0.0;
		allMap[x][y][z].parentX = x;
		allMap[x][y][z].parentY = y;
		allMap[x][y][z].parentZ = z;

		std::vector<Node> openList;

		openList.emplace_back(allMap[x][y][z]);
		bool destinationFound = false;

		while (!openList.empty() && openList.size() < MAX_X * MAX_Y * MAX_Z) {
			Node node;
			do {
				float temp = FLT_MAX;
				std::vector<Node>::iterator itNode;
				for (std::vector<Node>::iterator it = openList.begin();
					it != openList.end(); it = next(it)) {
					Node n = *it;
					if (n.fCost < temp) {
						temp = n.fCost;
						itNode = it;
					}
				}
				node = *itNode;
				openList.erase(itNode);
			} while (isValid(node.x, node.y, node.z) == false);

			x = node.x;
			y = node.y;
			closedList[x][y][z] = true;

			//For each neighbour starting from North-West to South-East
			for (int newX = -1; newX <= 1; newX++) {
				for (int newY = -1; newY <= 1; newY++) {
					for (int newZ = -1; newZ <= 1; newZ++) {
						double gNew, hNew, fNew;
						if (isValid(x + newX, y + newY, z + newZ)) {
							if (isDestination(x + newX, y + newY, z + newZ, destination))
							{
								//Destination found - make path
								allMap[(double)x + newX][(double)y + newY][(double)z + newZ].parentX = x;
								allMap[(double)x + newX][(double)y + newY][(double)z + newZ].parentY = y;
								allMap[(double)x + newX][(double)y + newY][(double)z + newZ].parentZ = z;
								destinationFound = true;
								return makePath(allMap, destination);
							}
							else if (closedList[x + newX][y + newY] == false)
							{
								gNew = node.gCost + 1.0;
								hNew = calculateH(x + newX, y + newY, z + newZ, destination);
								fNew = gNew + hNew;
								// Check if this path is better than the one already present
								if (allMap[(double)x + newX][(double)y + newY][(double)z + newZ].fCost == FLT_MAX ||
									allMap[(double)x + newX][(double)y + newY][(double)z + newZ].fCost > fNew)
								{
									// Update the details of this neighbour node
									allMap[(double)x + newX][(double)y + newY][(double)z + newZ].fCost = fNew;
									allMap[(double)x + newX][(double)y + newY][(double)z + newZ].gCost = gNew;
									allMap[(double)x + newX][(double)y + newY][(double)z + newZ].hCost = hNew;
									allMap[(double)x + newX][(double)y + newY][(double)z + newZ].parentX = x;
									allMap[(double)x + newX][(double)y + newY][(double)z + newZ].parentY = y; 
									openList.emplace_back(allMap[(double)x + newX][(double)y + newY][(double)z + newZ]);
								}
							}
						}
					}
				}
			}
		}
		if (destinationFound == false) {
			//No path found
			return empty;
		}
	}

};