#include "DGPathfinding.h"

DGPathfinding::DGPathfinding() {

}

void DGPathfinding::Init(std::shared_ptr<WorldManager> wmgr, bool obstacles[MAX_AR_X][MAX_AR_Y][MAX_AR_Z]) {
	mWorldManager = wmgr;

	WorldManager::Pos p = mWorldManager->GetWorldSize();
	mWorldSize = Vec3I(p.x, p.y, p.z);

	for (int i = 0; i < MAX_AR_X; i++) {
		for (int j = 0; j < MAX_AR_Y; j++) {
			for (int k = 0; k < MAX_AR_Z; k++) {
				mObstacles[i][j][k] = obstacles[i][j][k];
			}
		}
	}
}

void DGPathfinding::CreateBounds(Vec3I start, Vec3I dest) {
	mBoundsMax = Vec3I(abs(start.x - dest.x), abs(start.y - dest.y), abs(start.z - dest.z));

	//If the distance between the start and end is small enough, figure out the ratio to put them into the array
	if (mBoundsMax.x < MAX_AR_X || mBoundsMax.y < MAX_AR_Y || mBoundsMax.z < MAX_AR_Z) {

		float xRatio = (float)(mBoundsMax.x) / (float)(start.x + dest.x);
		float yRatio = (float)(mBoundsMax.y) / (float)(start.y + dest.y);
		float zRatio = (float)(mBoundsMax.z) / (float)(start.z + dest.z);

		DirectX::XMFLOAT3 startPos(start.x * xRatio, start.y * yRatio, start.z * zRatio);
		DirectX::XMFLOAT3 endPos(dest.x * xRatio, dest.y * yRatio, dest.z * zRatio);





		int a = mBoundsMax.x;
	}
}

bool DGPathfinding::IsValidIndex(size_t arrayI, size_t arrayJ, size_t arrayK) {
	//Is coord within array bounds
	if (arrayI < 0 || arrayJ < 0 || arrayK < 0 || arrayI >= MAX_AR_X || arrayJ >= MAX_AR_Y || arrayK >= MAX_AR_Z) {
		return false;
	}

	//Is coord an obstacle
	if (mObstacles[arrayI][arrayJ][arrayK]) {
		return false;
	}

	return true;
}

bool DGPathfinding::IsValidWorldCoord(Vec3I pos) {
	if (pos.x < 0 || pos.y < 0 || pos.z < 0 || pos.x >= mWorldSize.x || pos.y >= mWorldSize.y || pos.z >= mWorldSize.z)
		return false;

	return true;
}

bool DGPathfinding::IsDestination(int x, int y, int z, Node destination)
{
	if (x == destination.indexX && y == destination.indexY && z == destination.indexZ) {
		return true;
	}
	return false;
}

int DGPathfinding::CalculateH(int x, int y, int z, Node destination)
{
	int H = sqrt((x - destination.x) * (x - destination.x) + (y - destination.y) * (y - destination.y) + (z - destination.z) * (z - destination.z));
	return H;
}

std::vector<Vec3I> DGPathfinding::MakePath(std::array<std::array<std::array<Node, MAX_AR_Z>, MAX_AR_Y>, MAX_AR_X> map, Node destination)
{
	size_t x = destination.indexX;
	size_t y = destination.indexY;
	size_t z = destination.indexZ;

	int worldX = destination.x;
	int worldY = destination.y;
	int worldZ = destination.z;

	std::stack<Node>* path = new std::stack<Node>();
	//std::vector<Node> path;
	std::vector<Node> usablePath;
	std::vector<Vec3I> ret;

	//From the end point, this iterates through its parents, making a path from the end to the start
	while (!(map[x][y][z].parentX == worldX && map[x][y][z].parentY == worldY && map[x][y][z].parentZ == worldZ)
		&& map[x][y][z].x != -1 && map[x][y][z].y != -1 && map[x][y][z].z != -1)
	{
		path->push(map[x][y][z]);
		worldX = map[x][y][z].parentX;
		worldY = map[x][y][z].parentY;
		worldZ = map[x][y][z].parentZ;
		x = map[x][y][z].parentIndexX;
		y = map[x][y][z].parentIndexY;
		z = map[x][y][z].parentIndexZ;

	}
	path->push(map[x][y][z]);

	while (!path->empty()) {
		Node top = path->top();
		path->pop();
		ret.push_back({ top.x + 1, top.y + 1, top.z + 1 });
	}
	delete path;
	return ret;
}

std::vector<Vec3I> DGPathfinding::AStar(Vec3I start, Vec3I dest) {
	//Are the start and end points valid?

	if (!IsValidWorldCoord(start) || !IsValidWorldCoord(dest)) {
		std::vector<Vec3I> empty;
		return empty;
	}

	bool closedList[MAX_AR_X][MAX_AR_Y][MAX_AR_Z] = { false };
	std::array<std::array<std::array<Node, MAX_AR_Z>, MAX_AR_Y>, MAX_AR_X> allMap;
	std::vector<Node> openList;
	Node destination;


	{
		//Fill in allMap with the correct coords
		const size_t halfX = MAX_AR_X / 2.f;
		const size_t halfY = MAX_AR_Y / 2.f;
		const size_t halfZ = MAX_AR_Z / 2.f;

		for (size_t x = -halfX; x < halfX; x++) {
			for (size_t y = -halfY; y < halfY; y++) {
				for (size_t z = -halfZ; z < halfZ; z++) {
					allMap[x + halfX][y + halfY][z + halfZ].SetCoords((int)(start.x + x - 1), (int)(start.y + y - 1), (int)(start.z + z - 1));
					allMap[x + halfX][y + halfY][z + halfZ].SetIndex(x + halfX, y + halfY, z + halfZ);
				}
			}
		}

		//Initialize our starting position
		allMap[halfX][halfY][halfZ].SetParent(start.x, start.y, start.z, halfX, halfY, halfZ);
		allMap[halfX][halfY][halfZ].SetCosts(0, 0, 0);

		//Initialize the end position
		{
			size_t destX = halfX + (size_t)(dest.x - start.x);
			size_t destY = halfY + (size_t)(dest.y - start.y);
			size_t destZ = halfZ + (size_t)(dest.z - start.z);
			destination.SetIndex(destX, destY, destZ);
			destination.SetCoords(dest.x, dest.y, dest.z);
		}

		//Add the start position to the open list
		openList.emplace_back(allMap[halfX][halfY][halfZ]);
	}

	bool destinationFound = false;
	int x, y, z;

	while (!openList.empty() && openList.size() < MAX_AR_X * MAX_AR_Y * MAX_AR_Z) {

		//Find a valid node. Each iteration removes the cheapest node
		Node node;
		do {
			//Set the comparison cost to the max value
			float temp = floatmax;
			std::vector<Node>::iterator itNode;

			//Search the whole list for the lowest cost node
			for (std::vector<Node>::iterator it = openList.begin(); it != openList.end(); it = next(it)) {
				//If the cost if this node, is lower than the current lowest, save it as the lowest
				if (it->fCost < temp) {
					temp = it->fCost;
					itNode = it;
				}
			}
			
			//Save a local copy of the cheapest node, while erasing it from the list
			if (itNode._Ptr != nullptr) {
				node = *itNode;
				openList.erase(itNode);
			}

		} while (IsValidIndex(node.indexX, node.indexY, node.indexZ) == false);

		//Add the node to the closed list
		x = node.indexX;
		y = node.indexY;
		z = node.indexZ;
		Node parentNode = allMap[x][y][z];
		closedList[x][y][z] = true;

		Vec3I pos{ x, y, z };
		for each (DirectX::XMINT3 dir in mNeighbors) {
			size_t currX = (size_t)(pos.x + dir.x);
			size_t currY = (size_t)(pos.y + dir.y);
			size_t currZ = (size_t)(pos.z + dir.z);
			int gNew, hNew, fNew;

			//Check if the neighbor is valid
			if (IsValidIndex(currX, currY, currZ)) {
				Node tempNode = allMap[currX][currY][currZ];

				//Check if the neighbor is the destination, if so, return.
				if (IsDestination((int)currX, (int)currY, (int)currZ, destination))
				{
					//Destination found - make path
					allMap[currX][currY][currZ].SetParent(parentNode.x, parentNode.y,parentNode.z, parentNode.indexX, parentNode.indexY, parentNode.indexZ);
					destination.SetParent(parentNode.x, parentNode.y, parentNode.z, parentNode.indexX, parentNode.indexY, parentNode.indexZ);
					destinationFound = true;
					return MakePath(allMap, destination);
				}

				//Check neighbor is not closed (so is an option)
				else if (closedList[currX][currY][currZ] == false)
				{
					//Calculate the new costs
					gNew = node.gCost + 1;
					hNew = CalculateH((int)currX, (int)currY, (int)currZ, destination);
					fNew = gNew + hNew;

					int a = allMap[currX][currY][currZ].fCost - floatmax;
					// Check if this path is better than the one already present. If it is, add it to the open list and keep iterating the while loop
					if (a == 0 || a == -2147483648 || allMap[currX][currY][currZ].fCost == floatmax || allMap[currX][currY][currZ].fCost > fNew)
					{
						// Update the details of this neighbour node
						allMap[currX][currY][currZ].SetCosts(gNew, hNew, fNew);
						allMap[currX][currY][currZ].SetParent(parentNode.x, parentNode.y, parentNode.z, parentNode.indexX, parentNode.indexY, parentNode.indexZ);
						openList.emplace_back(allMap[currX][currY][currZ]);
					}
				}
			}

		}
	}
	if (destinationFound == false) {
		//No path found
		std::vector<Vec3I> empty;
		return empty;
	}
}