#pragma once

#include <math.h>
#include <vector>
#include <array>
#include <stack>
#include <vector>
#include "WorldManager.h"
#include "Player.h"

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

	int indexx;
	int indexy;
	int indexz;
	int iparentX;
	int iparentY;
	int iparentZ;

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

	Pathfinding(std::shared_ptr<WorldManager> wm, std::shared_ptr<Player> p) {
		worldManager = wm;
		mPlayer = p;
	}

	std::shared_ptr<WorldManager> worldManager;
	std::shared_ptr<Player> mPlayer;

    bool isDestination(Node block, Node destination)
    {
        if (block.x == destination.x && block.y == destination.y && block.z == destination.z) {
            return true;
        }
        return false;
    }

    double calculateH(int x, int y, int z, Node destination) {
        double H = sqrt((x - destination.x) * (x - destination.x) + (y - destination.y) * (y - destination.y) + (z - destination.z) * (z - destination.z)) * mPlayer->getPlayerHealth();

        return H;
    }

	double calculateG(Node node) {
		float x = node.x;
		float y = node.y;
		float z = node.z;

		double G = 1;
		if (worldManager->GetBlock(XMFLOAT3{ x, y, z })->GetActive()) {
			G += 2;
		}
		else {
			if (!worldManager->GetBlock(XMFLOAT3{ x, y - 1, z })->GetActive()) {
				G += 1;
			}
		}
		if (worldManager->GetBlock(XMFLOAT3{ x, y + 1, z })->GetActive()) {
			G += 2;
		}
		return G;
	}

    std::vector<Node> makePath(std::array<std::array<std::array<Node, MAX_Z>, MAX_Y>, MAX_X> map, Node start, Node destination)
    {
		int x = destination.indexx;
		int y = destination.indexy;
		int z = destination.indexz;
		//std::stack<Node>* path = new std::stack<Node>();
		std::vector<Node> usablePath;

		while (!(map[x][y][z].iparentX == x && map[x][y][z].iparentY == y && map[x][y][z].iparentZ  == z)
			&& map[x][y][z].iparentX != -1 && map[x][y][z].iparentY != -1 && map[x][y][z].iparentZ != -1)
		{
			if (map[x][y][z].x != start.x || map[x][y][z].y != start.y || map[x][y][z].z != start.z) {
				std::wostringstream woss;
				woss << map[x][y][z].x << " " << map[x][y][z].y << " " << map[x][y][z].z << "    |||||    " << start.x << " " << start.y << " " << start.z << "\n";
				OutputDebugString(woss.str().c_str());
				usablePath.push_back(map[x][y][z]);
			}
			int tempx = map[x][y][z].iparentX;
			int tempy = map[x][y][z].iparentY;
			int tempz = map[x][y][z].iparentZ;
			x = tempx;
			y = tempy;
			z = tempz;
		}

		return usablePath;
	}

	std::vector<Node> aStar(Node start, Node destination) {

		int lowx = start.x - MAX_X / 2;
		int highx = start.x + MAX_X / 2;
		int lowy = start.y - MAX_Y / 2;
		int highy = start.y + MAX_Y / 2;
		int lowz = start.z - MAX_Z / 2;
		int highz = start.z + MAX_Z / 2;

		bool checkedList[MAX_X][MAX_Y][MAX_Z];

		// Initialise the array with the blocks around the start position
		// The max variables are how many blocks will be checked within the pathfinding
		// This will improve efficiency so we don't end up checking every block in the world
		std::array<std::array<std::array<Node, MAX_Z>, MAX_Y>, MAX_X> all;
		int globalx = lowx;
		int globaly = lowy;
		int globalz = lowz;
		for (int x = 0; x < MAX_X; x++) {
			for (int y = 0; y < MAX_Y; y++) {
				for (int z = 0; z < MAX_Z; z++) {
					all[x][y][z].fCost = FLT_MAX;
					all[x][y][z].gCost = FLT_MAX;
					all[x][y][z].hCost = FLT_MAX;
					all[x][y][z].parentX = -1;
					all[x][y][z].parentY = -1;
					all[x][y][z].parentZ = -1;
					all[x][y][z].x = globalx;
					all[x][y][z].y = globaly;
					all[x][y][z].z = globalz;
					all[x][y][z].iparentX = -1;
					all[x][y][z].iparentY = -1;
					all[x][y][z].iparentZ = -1;
					all[x][y][z].indexx = x;
					all[x][y][z].indexy = y;
					all[x][y][z].indexz = z;

					checkedList[x][y][z] = false;
					globalz++;
				}
				globalz = lowz;
				globaly++;
			}
			globaly = lowy;
			globalz = lowz;
			globalx++;
		}

		//Initialize our starting list
		size_t x = (MAX_X / 2) - 1;
		size_t y = (MAX_Y / 2) - 1;
		size_t z = (MAX_Z / 2) - 1;
		all[x][y][z].fCost = 0.0;
		all[x][y][z].gCost = 0.0;
		all[x][y][z].hCost = 0.0;
		all[x][y][z].parentX = all[x][y][z].x;
		all[x][y][z].parentY = all[x][y][z].y;
		all[x][y][z].parentZ = all[x][y][z].z;
		all[x][y][z].iparentX = x;
		all[x][y][z].iparentY = y;
		all[x][y][z].iparentZ = z;

		std::vector<Node> openList;

		openList.emplace_back(all[x][y][z]);
		bool destinationFound = false;

		while (!openList.empty() && openList.size() < MAX_X * MAX_Y * MAX_Z) {
			Node node;
			float temp = FLT_MAX;
			std::vector<Node>::iterator itNode;
			// Find the lowest cost block 
			for (std::vector<Node>::iterator it = openList.begin(); it != openList.end(); it = next(it)) {
				Node n = *it;
				if (n.fCost < temp) {
					temp = n.fCost;
					itNode = it;
				}
			}
			node = *itNode;
			openList.erase(itNode);

			x = node.indexx;
			y = node.indexy;
			z = node.indexz;
			checkedList[x][y][z] = true;

			//For each block surrounding the one we are on
			for (int newX = -1; newX <= 1; newX++) {
				for (int newY = -1; newY <= 1; newY++) {
					for (int newZ = -1; newZ <= 1; newZ++) {
						double gNew, hNew, fNew;
						if(all[(double)x + newX][(double)y + newY][(double)z + newZ].x < highx && all[(double)x + newX][(double)y + newY][(double)z + newZ].x > lowx 
							&& all[(double)x + newX][(double)y + newY][(double)z + newZ].y < highy && all[(double)x + newX][(double)y + newY][(double)z + newZ].y > lowy 
							&& all[(double)x + newX][(double)y + newY][(double)z + newZ].z < highz && all[(double)x + newX][(double)y + newY][(double)z + newZ].z > lowz){
							if (isDestination(all[x + newX][y + newY][z + newZ], destination))
							{
								//Destination found - make path
								all[(double)x + newX][(double)y + newY][(double)z + newZ].parentX = all[x][y][z].x;
								all[(double)x + newX][(double)y + newY][(double)z + newZ].parentY = all[x][y][z].y;
								all[(double)x + newX][(double)y + newY][(double)z + newZ].parentZ = all[x][y][z].z;
								all[(double)x + newX][(double)y + newY][(double)z + newZ].iparentX = x;
								all[(double)x + newX][(double)y + newY][(double)z + newZ].iparentY = y;
								all[(double)x + newX][(double)y + newY][(double)z + newZ].iparentZ = z;
								destination = all[x + newX][y + newY][z + newZ];
								destinationFound = true;
								return makePath(all, start, destination);
							}
							else if (checkedList[x + newX][y + newY][z + newZ] == false)
							{
								gNew = all[(double)x][(double)y][(double)z].gCost + calculateG(all[(double)x + newX][(double)y + newY][(double)z + newZ]);
								hNew = calculateH(x + newX, y + newY, z + newZ, destination);
								fNew = gNew + hNew;
								// Check if this path is better than the one already present
								if (all[(double)x + newX][(double)y + newY][(double)z + newZ].fCost == FLT_MAX ||
									all[(double)x + newX][(double)y + newY][(double)z + newZ].fCost > fNew)
								{
									// Update the details of this neighbour node
									all[(double)x + newX][(double)y + newY][(double)z + newZ].fCost = fNew;
									all[(double)x + newX][(double)y + newY][(double)z + newZ].gCost = gNew;
									all[(double)x + newX][(double)y + newY][(double)z + newZ].hCost = hNew;
									all[(double)x + newX][(double)y + newY][(double)z + newZ].parentX = all[x][y][z].x;
									all[(double)x + newX][(double)y + newY][(double)z + newZ].parentY = all[x][y][z].y;
									all[(double)x + newX][(double)y + newY][(double)z + newZ].parentZ = all[x][y][z].z;
									all[(double)x + newX][(double)y + newY][(double)z + newZ].iparentX = x;
									all[(double)x + newX][(double)y + newY][(double)z + newZ].iparentY = y;
									all[(double)x + newX][(double)y + newY][(double)z + newZ].iparentZ = z;
									openList.emplace_back(all[(double)x + newX][(double)y + newY][(double)z + newZ]);
								}
							}
						}
					}
				}
			}
		}
		if (destinationFound == false) {
			//No path found
			std::vector<Node> empty;
			return empty;
		}
	}

};