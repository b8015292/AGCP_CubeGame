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
	//mBoundsMax = Vec3I(abs(start.x - dest.x), abs(start.y - dest.y), abs(start.z - dest.z));

	////If the distance between the start and end is small enough, figure out the ratio to put them into the array
	//if (mBoundsMax.x < MAX_AR_X || mBoundsMax.y < MAX_AR_Y || mBoundsMax.z < MAX_AR_Z) {

	//	float xRatio = (float)(mBoundsMax.x) / (float)(start.x + dest.x);
	//	float yRatio = (float)(mBoundsMax.y) / (float)(start.y + dest.y);
	//	float zRatio = (float)(mBoundsMax.z) / (float)(start.z + dest.z);

	//	DirectX::XMFLOAT3 startPos(start.x * xRatio, start.y * yRatio, start.z * zRatio);
	//	DirectX::XMFLOAT3 endPos(dest.x * xRatio, dest.y * yRatio, dest.z * zRatio);





	//	int a = mBoundsMax.x;
	//}
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

size_t DGPathfinding::GetIndexOf3DArray(size_t x, size_t y, size_t z) {
	return (x * MAX_AR_Y * MAX_AR_Z) + (y * MAX_AR_Z) + z;
}

//std::vector<Vec3I> DGPathfinding::MakePath(std::array<Node, MAX_AR_Z * MAX_AR_Y * MAX_AR_X> map, Node destination)
std::vector<Vec3I> DGPathfinding::MakePath(Node map[MAX_AR_Z * MAX_AR_Y * MAX_AR_X] , Node destination)
{
	size_t x = destination.indexX;
	size_t y = destination.indexY;
	size_t z = destination.indexZ;
	size_t index = GetIndexOf3DArray(x, y, z);

	int worldX = destination.x;
	int worldY = destination.y;
	int worldZ = destination.z;

	std::vector<Node> path;
	std::vector<Node> usablePath;
	std::vector<Vec3I> ret;

	//From the end point, this iterates through its parents, making a path from the end to the start
	while (!(map[index].parentX == worldX && map[index].parentY == worldY && map[index].parentZ == worldZ)
		&& map[index].x != -1 && map[index].y != -1 && map[index].z != -1)
	{
		path.push_back(map[index]);
		worldX = map[index].parentX;
		worldY = map[index].parentY;
		worldZ = map[index].parentZ;
		x = map[index].parentIndexX;
		y = map[index].parentIndexY;
		z = map[index].parentIndexZ;
		index = GetIndexOf3DArray(x, y, z);
	}
	path.push_back(map[index]);

	for (size_t i = path.size() - 1; i != 0; i--) {
		Node top = path.at(i);
		ret.push_back({ top.x + 1, top.y + 1, top.z + 1 });
	}

	return ret;
}

std::vector<Vec3I> DGPathfinding::AStar(Vec3I start, Vec3I dest) {
	//Are the start and end points valid?

	if (!IsValidWorldCoord(start) || !IsValidWorldCoord(dest)) {
		std::vector<Vec3I> empty;
		return empty;
	}


	bool closedList[MAX_TOTAL] = { false };
	//std::array<Node, MAX_AR_Z * MAX_AR_Y * MAX_AR_X> allMap;
	Node allMap[MAX_TOTAL];

	std::vector<Node> openList;
	Node destination, tempNode;

	size_t tempX, tempY, tempZ, index;

	{
		//Fill in allMap with the correct coords
		const int halfX = MAX_AR_X / 2;
		const int halfY = MAX_AR_Y / 2;
		const int halfZ = MAX_AR_Z / 2;
		const size_t halfXT = MAX_AR_X / 2;
		const size_t halfYT = MAX_AR_Y / 2;
		const size_t halfZT = MAX_AR_Z / 2;

		for (int x = -halfX; x < halfX; x++) {
			tempX = (size_t)x + halfXT;
			//for (int y = -halfY; y < halfY; y++) {
				//tempY = (size_t)y + halfYT;
				tempY = 0;
				for (int z = -halfZ; z < halfZ; z++) {

					tempZ = (size_t)z + halfZT;
					index = GetIndexOf3DArray(tempX, tempY, tempZ);

					//allMap[index].SetCoords(start.x + x - 1, start.y + y - 1, start.z + z - 1);
					allMap[index].SetCoords(start.x + x - 1, start.y, start.z + z - 1);
					allMap[index].SetIndex(tempX, tempY, tempZ);
				}
			//}
		}

		//Initialize our starting position and add it to the open list
		index = GetIndexOf3DArray(halfXT, halfYT, halfZT);
		allMap[index].SetParent(start.x, start.y, start.z, halfX, halfY, halfZ);
		allMap[index].SetCosts(0, 0, 0);
		openList.emplace_back(allMap[index]);

		//Initialize the end position
		{
			size_t destX = halfXT + (size_t)dest.x - (size_t)start.x;
			size_t destY = halfYT + (size_t)dest.y - (size_t)start.y;
			size_t destZ = halfZT + (size_t)dest.z - (size_t)start.z;
			destination.SetIndex(destX, destY, destZ);
			destination.SetCoords(dest.x, dest.y, dest.z);
		}
	}

	bool destinationFound = false;
	size_t x, y, z;

	int iterations = 0;

	while (!openList.empty() && openList.size() < MAX_TOTAL) {

		GameData::Print("\nIteration: " + std::to_string(iterations++));

		//Find a valid node. Each iteration removes the cheapest node
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
				tempNode = *itNode;
				openList.erase(itNode);
			}

		} while (IsValidIndex(tempNode.indexX, tempNode.indexY, tempNode.indexZ) == false);

		//Add the node to the closed list
		x = tempNode.indexX;
		y = tempNode.indexY;
		z = tempNode.indexZ;
		size_t index = GetIndexOf3DArray((size_t)x, (size_t)y, (size_t)z);
		Node parentNode = allMap[index];
		closedList[index] = true;

		float gNew, hNew, fNew;
		for each (DirectX::XMINT3 dir in mNeighbors) {
			tempX = x + (size_t)dir.x;
			tempY = y + (size_t)dir.y;
			tempZ = z + (size_t)dir.z;

			//Check if the neighbor is valid
			if (IsValidIndex(tempX, tempY, tempZ)) {
				index = GetIndexOf3DArray(tempX, tempY, tempZ);
				tempNode = allMap[index];

				//Check if the neighbor is the destination, if so, return.
				if (IsDestination((int)tempX, (int)tempY, (int)tempZ, destination))
				{
					//Destination found - make path
					tempNode.SetParent(parentNode.x, parentNode.y,parentNode.z, parentNode.indexX, parentNode.indexY, parentNode.indexZ);
					destination.SetParent(parentNode.x, parentNode.y, parentNode.z, parentNode.indexX, parentNode.indexY, parentNode.indexZ);
					destinationFound = true;
					return MakePath(allMap, destination);
				}

				//Check neighbor is not closed (so is an option)
				else if (closedList[index] == false)
				{
					//Calculate the new costs
					gNew = tempNode.gCost + 1;
					hNew = CalculateH((int)tempX, (int)tempY, (int)tempZ, destination);
					fNew = hNew;// gNew + hNew;

					int a = allMap[index].fCost - floatmax;
					// Check if this path is better than the one already present. If it is, add it to the open list and keep iterating the while loop
					if (a == 0 || a == -2147483648 || allMap[index].fCost == floatmax || allMap[index].fCost > fNew)
					//if (allMap[index].hCost == floatmax || allMap[index].hCost > hNew)
					//if (tempNode.hCost >= hNew)
					//if (parentNode.hCost >= hNew)
					{
						// Update the details of this neighbour node
						allMap[index].SetCosts(gNew, hNew, fNew);
						tempNode.hCost = hNew;
						tempNode.SetParent(parentNode.x, parentNode.y, parentNode.z, parentNode.indexX, parentNode.indexY, parentNode.indexZ);
						openList.emplace_back(tempNode);
					}
				}
			}

		}
	}

	//If the function has not already returned, it has not found a destination. So return an empty list
	std::vector<Vec3I> empty;
	return empty;
}