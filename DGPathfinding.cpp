#include "DGPathfinding.h"

DGPathfinding::DGPathfinding(std::shared_ptr<WorldManager> wmgr) {
	worldManager = wmgr;
}

//Checks if the block is valid, and the block above and below it.
bool DGPathfinding::IsValid(int x, int y, int z) { //If our Node is an obstacle it is not valid
	//if (!worldManager->GetBlock(XMFLOAT3{ (float)x, (float)y, (float)z })->GetActive()) {
	//	if (!worldManager->GetBlock(XMFLOAT3{ (float)x, (float)y + 1, (float)z })->GetActive()) {
	//		if (worldManager->GetBlock(XMFLOAT3{ (float)x, (float)y - 2, (float)z })->GetActive()) {
	//			return true;
	//		}
	//	}
	//}
	//return false;
	return true;
}

bool DGPathfinding::IsDestination(int x, int y, int z, Node destination)
{
	if (x == destination.x && y == destination.y && z == destination.z) {
		return true;
	}
	return false;
}

double DGPathfinding::CalculateH(int x, int y, int z, Node destination)
{
	double H = sqrt((x - destination.x) * (x - destination.x) + (y - destination.y) * (y - destination.y) + (z - destination.z) * (z - destination.z));
	return H;
}

std::vector<Node> DGPathfinding::MakePath(std::array<std::array<std::array<Node, MAX_Z>, MAX_Y>, MAX_X> map, Node destination)
{
	int x = destination.x;
	int y = destination.y;
	int z = destination.z;
	std::stack<Node>* path = new std::stack<Node>();
	std::vector<Node> usablePath;

	while (!(map[x][y][z].parentX == x && map[x][y][z].parentY == y && map[x][y][z].parentZ == z)
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


	//TODO: Check if the destination is not air		
	//MAX_X, MAX_Y & MAX_Z = 12
std::vector<Node> DGPathfinding::AStar(Node start, Node destination) {

	bool closedList[MAX_X][MAX_Y][MAX_Z];
	std::array<std::array<std::array<Node, MAX_Z>, MAX_Y>, MAX_X> allMap;
	//Fill in allMap with the correct coords and initilize closedList
	{
		const int halfX = MAX_X / 2.f;
		const int halfY = MAX_Y / 2.f;
		const int halfZ = MAX_Z / 2.f;

		//DOES NOT FILL ALL SPACES
		for (int x = -halfX; x < halfX; x++) {
			for (int y = -halfY; y < halfY; y++) {
				for (int z = -halfZ; z < halfZ; z++) {


					allMap[(size_t)x + halfX][(size_t)y + halfY][(size_t)z + halfZ].x = start.x + x - 1;
					allMap[(size_t)x + halfX][(size_t)y + halfY][(size_t)z + halfZ].y = start.y + y - 1;
					allMap[(size_t)x + halfX][(size_t)y + halfY][(size_t)z + halfZ].z = start.z + z - 1;
				}
			}
		}
	}

	for each (bool& b in closedList) {
		b = false;
	}



	//Initialize our starting position
	{
		Node& n = allMap[(size_t)start.x][(size_t)start.y][(size_t)start.z];
		n.fCost = 0.0;
		n.gCost = 0.0;
		n.hCost = 0.0;
		n.parentX = start.x;
		n.parentY = start.y;
		n.parentZ = start.z;
	}

	std::vector<Node> openList;
	openList.emplace_back(allMap[start.x][start.y][start.z]);
	bool destinationFound = false;
	int x, y, z;

	while (!openList.empty() && openList.size() < MAX_X * MAX_Y * MAX_Z) {

		//Find a valid node. Each iteration removes the cheapest node
		Node node;
		do {
			//Set the comparison cost to the max value
			float temp = FLT_MAX;
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
			if (&itNode == nullptr) {
				break;
			}
			node = *itNode;
			openList.erase(itNode);
		} while (IsValid(node.x, node.y, node.z) == false);

		//Add the node to the closed list
		x = node.x;
		y = node.y;
		z = node.z;
		closedList[x][y][z] = true;



		//For each neighbour starting from North-West to South-East
		for (int newX = -1; newX <= 1; newX++) {
			for (int newY = -1; newY <= 1; newY++) {
				for (int newZ = -1; newZ <= 1; newZ++) {
					double gNew, hNew, fNew;

					//Check if the neighbor is valid
					if (IsValid(x + newX, y + newY, z + newZ)) {
						
						//Check if the neighbor is the destination, if so, return.
						if (IsDestination(x + newX, y + newY, z + newZ, destination))
						{
							//Destination found - make path
							allMap[(double)x + newX][(double)y + newY][(double)z + newZ].parentX = x;
							allMap[(double)x + newX][(double)y + newY][(double)z + newZ].parentY = y;
							allMap[(double)x + newX][(double)y + newY][(double)z + newZ].parentZ = z;
							destinationFound = true;
							return MakePath(allMap, destination);
						}

						//Check neighbor is not closed (so is an option)
						else if (closedList[x + newX][y + newY][z + newZ] == false)
						{
							gNew = node.gCost + 1.0;
							hNew = CalculateH(x + newX, y + newY, z + newZ, destination);
							fNew = gNew + hNew;

							// Check if this path is better than the one already present. If it is, add it to the open list and keep iterating the while loop
							if (allMap[(double)x + newX][(double)y + newY][(double)z + newZ].fCost == FLT_MAX 
								|| allMap[(double)x + newX][(double)y + newY][(double)z + newZ].fCost > fNew)
							{
								// Update the details of this neighbour node
								allMap[(double)x + newX][(double)y + newY][(double)z + newZ].fCost = fNew;
								allMap[(double)x + newX][(double)y + newY][(double)z + newZ].gCost = gNew;
								allMap[(double)x + newX][(double)y + newY][(double)z + newZ].hCost = hNew;
								allMap[(double)x + newX][(double)y + newY][(double)z + newZ].parentX = x;
								allMap[(double)x + newX][(double)y + newY][(double)z + newZ].parentY = y;
								allMap[(double)x + newX][(double)y + newY][(double)z + newZ].parentZ = z;
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
		std::vector<Node> empty;
		return empty;
	}
}