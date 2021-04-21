#include "DGPathfinding.h"

DGPathfinding::DGPathfinding() {

}

void DGPathfinding::Init(std::shared_ptr<WorldManager> wmgr) {
	mWorldManager = wmgr;

	WorldManager::Pos p = mWorldManager->GetWorldSize();
	mWorldSize = Vec3I(p.x, p.y, p.z);
	mMaxBounds = mWorldSize;

	mStartPoint = { -1, -1, -1 };
}

void DGPathfinding::SetObstcales(bool obstacles[MAX_AR_X][MAX_AR_Y][MAX_AR_Z]) {
	bool outsideX = false;
	bool outsideY = false;
	bool outsideZ = false;

	for (int i = 0; i < MAX_AR_X; i++) {
		//if (i < mMinBounds.x || i > mMaxBounds.x) outsideX = true;

		for (int j = 0; j < MAX_AR_Y; j++) {
			for (int k = 0; k < MAX_AR_Z; k++) {

				////If its outside the bounds, set it to false, otherwise set it to the obstacle value
				//if (outsideX || k < mMinBounds.z || k > mMaxBounds.z) {
				//	mObstacles[i][j][k] = true;
				//}
				//else {
					mObstacles[i][j][k] = obstacles[i][j][k];
				//}

			}
		}
	}
}

void DGPathfinding::AddPathToObstacles(std::vector<Vec3I> path) {
	if (mStartPoint.x == -1 || mStartPoint.z == -1)
		return;

	const size_t halfXT = MAX_AR_X / 2;
	const size_t halfYT = MAX_AR_Y / 2;
	const size_t halfZT = MAX_AR_Z / 2;
	size_t x, y, z;

	for each (Vec3I pos in path) {
		x = MAX_AR_X / 2 + ((size_t)pos.x - (size_t)mStartPoint.x);
		y = halfYT + ((size_t)pos.y - (size_t)mStartPoint.y);
		z = halfZT + ((size_t)pos.z - (size_t)mStartPoint.z);

		mObstacles[x][0][z] = true;
	}
}

void DGPathfinding::SetMainPath(Vec3I start, Vec3I end) {
	mStartPoint = start;

	//Fill in allMap with the correct coords
	const int halfX = MAX_AR_X / 2;
	const int halfY = MAX_AR_Y / 2;
	const int halfZ = MAX_AR_Z / 2;
	const size_t halfXT = MAX_AR_X / 2;
	const size_t halfYT = MAX_AR_Y / 2;
	const size_t halfZT = MAX_AR_Z / 2;

	size_t tempX, tempY, tempZ, index;

	tempY = 0;
	int y = 1;

	for (int x = -halfX; x < halfX; x++) {
		tempX = (size_t)x + halfXT;
		//for (int y = -halfY; y < halfY; y++) {
			//tempY = (size_t)y + halfYT;
		for (int z = -halfZ; z < halfZ; z++) {

			tempZ = (size_t)z + halfZT;
			index = GetIndexOf3DArray(tempX, tempY, tempZ);

			mAllMap[index].SetCoords(start.x + x - 1, start.y + y - 1, start.z + z - 1);
			mAllMap[index].SetIndex(tempX, tempY, tempZ);
		}
		//}
	}
}

bool DGPathfinding::IsValidIndex(size_t arrayI, size_t arrayJ, size_t arrayK) {
	//Is coord within array bounds
	if (arrayI < 0 || arrayJ < 0 || arrayK < 0 || arrayI >= MAX_AR_X || arrayJ >= MAX_AR_Y || arrayK >= MAX_AR_Z) {
		return false;
	}

	//Is the coor within bounds
	if (arrayI < mMinBounds.x || arrayJ < mMinBounds.y || arrayK < mMinBounds.z
		|| arrayI >= mMaxBounds.x || arrayJ >= mMaxBounds.y || arrayK >= mMaxBounds.z)
		return false;

	//Is coord an obstacle
	if (mObstacles[arrayI][arrayJ][arrayK]) {
		return false;
	}

	return true;
}

bool DGPathfinding::IsValidWorldCoord(Vec3I pos) {
	if (pos.x < 0 || pos.y < 0 || pos.z < 0
		|| pos.x >= mWorldSize.x || pos.y >= mWorldSize.y || pos.z >= mWorldSize.z)
		return false;

	return true;
}

bool DGPathfinding::IsDestination(int x, int y, int z, Node destination)
{
	//if (x == destination.indexX && y == destination.indexY && z == destination.indexZ) {
	if (x == destination.indexX && z == destination.indexZ) {
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
	//return (x * MAX_AR_Y * MAX_AR_Z) + (y * MAX_AR_Z) + z;
	return (x * MAX_AR_Y * MAX_AR_Z) + z;
}

void DGPathfinding::SetWorldBounds(int minX, int maxX, int minZ, int maxZ) {
	if (minX < 0 || minZ < 0 || maxX >= mWorldSize.x || maxZ >= mWorldSize.z) {
		if (minX < 0) {
			minX = abs(minX);
		}
		else {
			minX = 0;
		}

		if (minZ < 0) {
			minZ = abs(minZ);
		}
		else {
			minZ = 0;
		}

		if (maxX > mWorldSize.x) {
			maxX = MAX_AR_X - (maxX - mWorldSize.x);
		}
		else {
			maxX = mWorldSize.x + 1;
		}

		if (maxZ > mWorldSize.z) {
			maxZ = MAX_AR_Z - (maxZ = mWorldSize.z);
		}
		else {
			maxZ = mWorldSize.z + 1;
		}

		mMinBounds = { (int)minX, 0, (int)minZ };
		mMaxBounds = { (int)maxX, mWorldSize.y, (int)maxZ };
	}


}

std::vector<Vec3I> DGPathfinding::MakePath(Node map[MAX_TOTAL], Node destination)
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
	//path.push_back(map[index]);

	//Reverse the list, so the first becomes the last
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

	bool closedList[MAX_AR_X * MAX_AR_Y * MAX_AR_Z] = { false };
	Node allMap[MAX_TOTAL];
	std::vector<Node> openList;
	Node destination, startNode, tempNode;
	size_t tempX, tempY, tempZ, index;

	//Used for side paths:
	bool startAndFinishRemovedFromObstacles = false;
	bool wasStartAnObstacle, wasDestAnObstacle;

	{
		//Fill in allMap with the correct coords
		const int halfX = MAX_AR_X / 2;
		const int halfY = MAX_AR_Y / 2;
		const int halfZ = MAX_AR_Z / 2;
		const size_t halfXT = MAX_AR_X / 2;
		const size_t halfYT = MAX_AR_Y / 2;
		const size_t halfZT = MAX_AR_Z / 2;

		//Copy the mAllMap into the local version
		for (size_t i = 0; i < MAX_TOTAL; i++) {
			allMap[i] = mAllMap[i];
		}

		//If this is the main path
		if (start == mStartPoint) {
			//Initialize our starting position and add it to the open list
			index = GetIndexOf3DArray(halfXT, halfYT, halfZT);
			allMap[index].SetParent(start.x, start.y, start.z, halfX, halfY, halfZ);
			allMap[index].SetCosts(0, 0, 0);
			openList.emplace_back(allMap[index]);

			//Initialize the end position
			tempX = halfXT + (size_t)dest.x - (size_t)start.x;
			tempY = halfYT + (size_t)dest.y - (size_t)start.y;
			tempZ = halfZT + (size_t)dest.z - (size_t)start.z;
			destination.SetIndex(tempX, tempY, tempZ);
			destination.SetCoords(dest.x, dest.y, dest.z);

			//If any of the local map is outside of world bounds, set all the outer space to be an obstacle
			SetWorldBounds(start.x - halfX, start.x + halfX, start.z - halfZ, start.z + halfZ);

		}
		//If this is a side path
		else {
			//get the array position of the current start and add it to the open list
			tempX = halfXT + ((size_t)start.x - (size_t)mStartPoint.x);
			tempY = halfYT + ((size_t)start.y - (size_t)mStartPoint.y);
			tempZ = halfZT + ((size_t)start.z - (size_t)mStartPoint.z);
			index = GetIndexOf3DArray(tempX, tempY, tempZ);

			allMap[index].SetCosts(0, 0, 0);
			openList.emplace_back(allMap[index]);
			wasStartAnObstacle = mObstacles[tempX][tempY][tempZ];
			mObstacles[tempX][tempY][tempZ] = false;

			//Set the destination
			tempX = halfXT + ((size_t)dest.x - (size_t)mStartPoint.x);
			tempY = halfYT + ((size_t)dest.y - (size_t)mStartPoint.y);
			tempZ = halfZT + ((size_t)dest.z - (size_t)mStartPoint.z);
			index = GetIndexOf3DArray(tempX, tempY, tempZ);

			destination.SetIndex(tempX, tempY, tempZ);
			destination.SetCoords(dest.x, dest.y, dest.z);
			wasDestAnObstacle = mObstacles[tempX][tempY][tempZ];
			mObstacles[tempX][tempY][tempZ] = false;

			startAndFinishRemovedFromObstacles = true;
		}
	}

	bool destinationFound = false;
	int gNew, hNew, fNew, x, y, z;
	Node parentNode;
	Vec3I pos;

	while (!openList.empty() && openList.size() < MAX_AR_X * MAX_AR_Y * MAX_AR_Z) {

		//Find a valid node. Each iteration removes the cheapest node
		{
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
		}

		//Add the node to the closed list
		index = GetIndexOf3DArray(tempNode.indexX, tempNode.indexY, tempNode.indexZ);
		parentNode = allMap[index];
		closedList[index] = true;
		pos = { (int)tempNode.indexX, (int)tempNode.indexY, (int)tempNode.indexZ };

		for each (DirectX::XMINT3 dir in mNeighbors) {
			x = pos.x + dir.x;
			y = pos.y + dir.y;
			z = pos.z + dir.z;
			tempX = (size_t)x;
			tempY = (size_t)y;
			tempZ = (size_t)z;

			//Check if the neighbor is valid
			if (IsValidIndex(tempX, tempY, tempZ)) {
				index = GetIndexOf3DArray(tempX, tempY, tempZ);
				tempNode = allMap[index];

				//Check if the neighbor is the destination, if so, return.
				if (IsDestination(x, y, z, destination))
				{
					//Destination found - make path
					allMap[index].SetParent(parentNode.x, parentNode.y, parentNode.z, parentNode.indexX, parentNode.indexY, parentNode.indexZ);
					destination.SetParent(parentNode.x, parentNode.y, parentNode.z, parentNode.indexX, parentNode.indexY, parentNode.indexZ);
					destinationFound = true;
					return MakePath(allMap, destination);
				}

				//Check neighbor is not closed (so is an option)
				else if (closedList[index] == false)
				{
					//Calculate the new costs
					gNew = tempNode.gCost + 1;
					hNew = CalculateH(x, y, z, destination);
					fNew = gNew + hNew;

					int a = allMap[index].fCost - floatmax;
					// Check if this path is better than the one already present. If it is, add it to the open list and keep iterating the while loop
					if (a == 0 || a == -2147483648 || allMap[index].fCost == floatmax || allMap[index].fCost > fNew)
					{
						// Update the details of this neighbour node
						allMap[index].SetCosts(gNew, hNew, fNew);
						allMap[index].SetParent(parentNode.x, parentNode.y, parentNode.z, parentNode.indexX, parentNode.indexY, parentNode.indexZ);
						openList.emplace_back(allMap[index]);
						closedList[index] = true;
					}
				}
			}

		}
	}

	//If a side path was not found, and positions from its parent path were removed 
	//from the obstacle list, return them to their previous state
	if (startAndFinishRemovedFromObstacles) {
		const size_t halfXT = MAX_AR_X / 2;
		const size_t halfYT = MAX_AR_Y / 2;
		const size_t halfZT = MAX_AR_Z / 2;

		if (wasStartAnObstacle) {
			tempX = halfXT + ((size_t)start.x - (size_t)mStartPoint.x);
			tempY = halfYT + ((size_t)start.y - (size_t)mStartPoint.y);
			tempZ = halfZT + ((size_t)start.z - (size_t)mStartPoint.z);
			mObstacles[tempX][tempY][tempZ] = true;
		}

		if (wasDestAnObstacle) {
			tempX = halfXT + ((size_t)dest.x - (size_t)mStartPoint.x);
			tempY = halfYT + ((size_t)dest.y - (size_t)mStartPoint.y);
			tempZ = halfZT + ((size_t)dest.z - (size_t)mStartPoint.z);
			mObstacles[tempX][tempY][tempZ] = true;
		}
	}

	//No path found
	std::vector<Vec3I> empty;
	return empty;
}