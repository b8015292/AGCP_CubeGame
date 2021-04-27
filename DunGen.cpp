#include "DunGen.h"


//***************************************************************
//							Vector 3 Int
//***************************************************************

Vec3I::Vec3I(int nx, int ny, int nz) {
	x = nx;
	y = ny;
	z = nz;
}
Vec3I::Vec3I(float nx, float ny, float nz) {
	x = (int)nx;
	y = (int)ny;
	z = (int)nz;
}

Vec3I& Vec3I::operator+(const Vec3I& v) {
	Vec3I ret(x + v.x, y + v.y, z + v.z);
	return ret;
}
Vec3I& Vec3I::operator-(const Vec3I& v) {
	Vec3I ret(x - v.x, y - v.y, z - v.z);
	return ret;
}
Vec3I& Vec3I::operator+=(const Vec3I& v) {
	x += v.x;
	y += v.y;
	z += v.z;
	return (*this);
}
Vec3I& Vec3I::operator-=(const Vec3I& v) {
	x -= v.x;
	y -= v.y;
	z -= v.z;
	return (*this);
}

bool Vec3I::operator==(const Vec3I& v) {
	return (x == v.x && y == v.y && z == v.z);
}
bool Vec3I::operator!=(const Vec3I& v) {
	return !(*this == v);
}

//***************************************************************
//							DunGen
//***************************************************************

DunGen::DunGen(std::shared_ptr<WorldManager> wrlmgr, DungeonInfo di) {
	Init(wrlmgr, di.noiseSeed);
	mDungeonInfo = di;

	if (IsValidDungeonInfo(di)) {

		GenerateMainPath(di.minDistanceOfMainPath, di.maxDistanceOfMainPath);

		mMinDistanceOfSidePath = (int)((float)di.minDistanceOfMainPath * di.ratioOfLengthOfMainPathToSidePaths);
		mMaxDistanceOfSidePath = (int)((float)di.maxDistanceOfMainPath * di.ratioOfLengthOfMainPathToSidePaths);
		mNumberOfSidePaths = (int)((float)mPaths.at(0).positions.size() * di.ratioOfLengthOfMainPathToNumberOfSidePaths);
		GenerateSidePaths(mNumberOfSidePaths, mMinDistanceOfSidePath, mMaxDistanceOfSidePath, di.minDistanceBeforeBranch, di.ratioOfDeadEndsToReconnectingBranches);

		mValid = true;
	}
}

void DunGen::Output() {
	if (mValid) {
		PrintInfo(mDungeonInfo, mMinDistanceOfSidePath, mMaxDistanceOfSidePath, mNumberOfSidePaths, (int)mPaths.at(0).positions.size());
		PrintDungeon(mDungeonInfo);
	}
	else {
		PrintInvalid(mDungeonInfo);
	}
}

bool DunGen::IsValidDungeonInfo(DungeonInfo& di) {
	if (di.maxDistanceOfMainPath <= di.minDistanceOfMainPath)
		return false;

	return true;
}

void DunGen::Init(std::shared_ptr<WorldManager> worldMgr, unsigned int noiseSeed) {
	//Set the world manager and world size
	mWorldMgr = worldMgr;
	WorldManager::Pos maxSize = mWorldMgr->GetWorldSize();
	mWorldSize = { maxSize.x, maxSize.y, maxSize.z };

	mPathFinding.Init(mWorldMgr);

	//Randomize the obstacle noise generation
	//srand(std::time(NULL));
	srand(noiseSeed);
	unsigned int seed = rand() % 10000;
	mNoise = PerlinNoise(noiseSeed);
	srand(mNoise.GetSeed());
}

void DunGen::GenerateMainPath(int minDistance, int maxDistance) {
	//If a path already exists, exit
	if (!mPaths.empty())
		return;

	//Add an empty path
	Path p;
	p.index = 0;
	mPaths.push_back(p);

	int obsticalAttempts;
	int maxObsticalAttempts = 10;
	double obsticalChanceInitial = 0.5;
	double obsticalChance = obsticalChanceInitial;
	double obsticalChanceDecrement = obsticalChance / (double)maxObsticalAttempts;

	//Create the start/end points and obsticals and path. Re create the start/end if no path can be found
	do {
		obsticalAttempts = 0;
		obsticalChance = obsticalChanceInitial;

		//Generate a random start and end point
		GenerateMainStartAndEnd(minDistance, maxDistance);

		//Fill in the first path, keep re-making the obsticales until a path is found
		while (mPaths.at(0).positions.empty() && obsticalAttempts < maxObsticalAttempts) {
			GenerateObsticales(obsticalChance);
			mPathFinding.SetObstcales(mObstacles);
			GeneratePath(0);

			obsticalAttempts++;
			obsticalChance -= obsticalChanceDecrement;

		}
	} while (mPaths.at(0).positions.empty());

	mPathFinding.AddPathToObstacles(mPaths.at(0).positions);
}

void DunGen::GenerateSidePaths(int numberToGenerate, int minDistance, int maxDistance, int minDistanceBeforeBranch, float ratioOfDeadEndsToReconnectingBranches) {
	int numberOfDeadEnds = (int)((float)numberToGenerate * ratioOfDeadEndsToReconnectingBranches);

	for (int i = 0; i < numberToGenerate; i++) {
		GenerateSidePath(0, i <= numberOfDeadEnds, minDistance, maxDistance);
	}
}

void DunGen::GenerateSidePath(size_t parentPath, bool deadend, int minDistance, int maxDistance) {
	Path p;
	p.index = mPaths.size();
	p.parentIndex = parentPath;
	mPaths.push_back(p);
	mPaths.at(parentPath).children.push_back(p.index);

	GenerateSideStart(p.index);

	if (deadend) {
		GenerateSideDeadEnd(p.index, minDistance, maxDistance);
	}
	else {
		GenerateSideLinkEnd(p.index, minDistance, maxDistance);
	}

	int obsticalAttempts = 0;
	int maxObsticalAttempts = 10;
	double obsticalChanceInitial = 0.5;
	double obsticalChance = obsticalChanceInitial;
	double obsticalChanceDecrement = obsticalChance / (double)maxObsticalAttempts;

	//Keep trying new obstacle sets until a path is found
	do {
		//Generate new obstacles, and add the main path to that list
		GenerateObsticales(obsticalChance);
		mPathFinding.SetObstcales(mObstacles);
		mPathFinding.AddPathToObstacles(mPaths.at(0).positions);

		GeneratePath(p.index);

		obsticalAttempts++;
		obsticalChance -= obsticalChanceDecrement;
	} while (mPaths.at(p.index).positions.empty() && obsticalAttempts < maxObsticalAttempts);

	mPathFinding.AddPathToObstacles(mPaths.at(0).positions);
}

void DunGen::GenerateSideStart(size_t index) {
	size_t size = mPaths.at(mPaths.at(index).parentIndex).positions.size();
	size_t startIndex;

	do {
		//Exclude the first and last index
		startIndex = 1 + (((size_t)rand() % size) - 3);
	} while (startIndex >= size);

	//Set the start of this path to a position in its parents path
	mPaths.at(index).start = mPaths.at(mPaths.at(index).parentIndex).positions.at(startIndex);
}

bool DunGen::IsPositionValid(size_t index, int minDistance, int maxDistance) {
	int distance = abs(mPaths.at(index).start.x - mPaths.at(index).end.x) + abs(mPaths.at(index).start.z - mPaths.at(index).end.z);

	return !(mPaths.at(index).end == mPaths.at(index).start
		|| mPaths.at(index).end.x < 0 || mPaths.at(index).end.z < 0
		|| mPaths.at(index).end.x >= mWorldSize.x || mPaths.at(index).end.z >= mWorldSize.z
		|| distance < minDistance || distance > maxDistance);
}

void DunGen::GenerateSideDeadEnd(size_t index, int minDistance, int maxDistance) {
	int distance = -1;
	do {
		mPaths.at(index).end = mPaths.at(0).start;
		mPaths.at(index).end.x += ((rand() % MAX_AR_X) - MAX_AR_X * 0.5f) - 1;
		mPaths.at(index).end.z += ((rand() % MAX_AR_Z) - MAX_AR_Z * 0.5f) - 1;

	} while (IsPositionValid(index, minDistance, maxDistance));
}

void DunGen::GenerateSideLinkEnd(size_t index, int minDistance, int maxDistance) {
	size_t size = mPaths.at(mPaths.at(index).parentIndex).positions.size();
	int distance;

	do {
		//Exclude the first and last index
		//size_t endIndex = 1 + (((size_t)rand() % size) - 2);
		int endIndexPREV = 1 + ((rand() % (int)size) - 1);
		size_t endIndex = (size_t)endIndexPREV;

		//Set the start of this path to a position in its parents path
		mPaths.at(index).end = mPaths.at(mPaths.at(index).parentIndex).positions.at(endIndex);
		distance = abs(mPaths.at(index).start.x - mPaths.at(index).end.x) + abs(mPaths.at(index).start.z - mPaths.at(index).end.z);

	} while (IsPositionValid(index, minDistance, maxDistance));
}

void DunGen::GenerateMainStartAndEnd(int minDistance, int maxDistance) {

	//Place the start where the player spawns
	mPaths.at(0).start = { 50, 5, 50 };

	//Randomize the start
	//mPaths.at(0).start.x = (rand() % mWorldSize.x);
	//mPaths.at(0).start.y = (rand() % mWorldSize.y);
	//mPaths.at(0).start.z = (rand() % mWorldSize.z);

	//Randomize the end. Cannot be more than half of MAX_AR_X or MAX_AR_Z away from the start
	int distance = -1;
	do {
		mPaths.at(0).end = mPaths.at(0).start;
		mPaths.at(0).end.x += ((rand() % MAX_AR_X) - MAX_AR_X * 0.5f) - 1;
		mPaths.at(0).end.z += ((rand() % MAX_AR_Z) - MAX_AR_Z * 0.5f) - 1;
		distance = abs(mPaths.at(0).start.x - mPaths.at(0).end.x) + abs(mPaths.at(0).start.z - mPaths.at(0).end.z);

	} while (IsPositionValid(0, minDistance, maxDistance));

	mPathFinding.SetMainPath(mPaths.at(0).start, mPaths.at(0).end);
}


void DunGen::GenerateObsticales(double chance) {
	//Re-randomize the noise each time this function is run to generate new obsticales each time
	for (size_t x = 0; x < MAX_AR_X; x++) {
		for (size_t z = 0; z < MAX_AR_Z; z++) {
			//Octaves is the number of times it applies noise to itself
			//Persistance is height of the noise
			double n = mNoise.OctavePerlin((double)x * 0.002, (double)5.0, (double)z * 0.002, 8, 20.0);
			if (n < chance) {
				mObstacles[x][0][z] = true;
			}
			else {
				mObstacles[x][0][z] = false;
			}
		}
	}
}

void DunGen::GeneratePath(size_t index) {
	mPaths.at(index).positions = mPathFinding.AStar(mPaths.at(index).start, mPaths.at(index).end);
}

void DunGen::Spawn(bool walls) {
	//World size X and Z is 64
	//x = nothing, s = side, m = middle, w = wall
	//Fill the array with 'x'
	char taken[64][64];
	for (int i = 0; i < 64; i++) {
		for (int j = 0; j < 64; j++) {
			taken[i][j] = ' ';
		}
	}

	//Mark the paths
	for each (Path p in mPaths) {
		if (p.positions.size() > 0) {
			for each (Vec3I pos in p.positions) {
				if (taken[pos.x][pos.z] == ' ') {
					taken[pos.x][pos.z] = 'm';
				}
			}
		}
	}

	//Add sides the each path to make it three blocks wide
	for (int i = 0; i < 64; i++) {
		for (int j = 0; j < 64; j++) {
			if (taken[i][j] == 'm') {

				if (taken[i + 1][j] != 'm') 
					taken[i + 1][j] = 's';

				if (taken[i - 1][j] != 'm') 
					taken[i - 1][j] = 's';

				if (taken[i][j + 1] != 'm') 
					taken[i][j + 1] = 's';

				if (taken[i][j - 1] != 'm') 
					taken[i][j - 1] = 's';
			}
		}
	}

	//Add walls adjacent to each side
	for (int i = 0; i < 64; i++) {
		for (int j = 0; j < 64; j++) {
			if (taken[i][j] == 's') {

				if (taken[i + 1][j] == ' ')
					taken[i + 1][j] = 'w';

				if (taken[i - 1][j] == ' ')
					taken[i - 1][j] = 'w';

				if (taken[i][j + 1] == ' ')
					taken[i][j + 1] = 'w';

				if (taken[i][j - 1] == ' ')
					taken[i][j - 1] = 'w';
			}
		}
	}

	int height = 5;
	Vec3I pos;
	std::string material = "mat_oak_log";
	for (int i = 0; i < 64; i++) {
		for (int j = 0; j < 64; j++) {
			pos = { i, 7, j };

			if (taken[i][j] == 'm' || taken[i][j] == 's') {
				SetBlock(pos, material);
				pos.y++;
				SetBlock(pos, material, false);
				pos.y++;
				SetBlock(pos, material, false);
				pos.y++;
				SetBlock(pos, material, false);
				pos.y++;
				SetBlock(pos, material);
			}
			else if (taken[i][j] == 'w') {
				SetBlock(pos, material);
				pos.y++;
				SetBlock(pos, material);
				pos.y++;
				SetBlock(pos, material);
				pos.y++;
				SetBlock(pos, material);
				pos.y++;
				SetBlock(pos, material);
			}
		}
	}
}

void DunGen::SetBlock(Vec3I pos, std::string material, bool active) {
	std::shared_ptr<WorldManager::Chunk> chunk = mWorldMgr->GetChunkFromWorldCoords({ (float)pos.x, (float)pos.y, (float)pos.z });
	WorldManager::Pos chunkPos = chunk->GetPos();

	WorldManager::Pos coordsInChunkSpace(pos.x - chunkPos.x * WorldManager::sChunkDimension, pos.y - chunkPos.y * WorldManager::sChunkDimension, pos.z - chunkPos.z * WorldManager::sChunkDimension);

	std::shared_ptr<Block> block = chunk->GetBlock(coordsInChunkSpace);
	block->SetActive(active);
	block->ChangeMaterial(material);
}

//*******************************************
//
//For File Output
//
//*******************************************

void DunGen::PrintInvalid(DungeonInfo di) {
	std::ofstream output(di.filePath, std::ofstream::app);

	output << "\nDUNGEON INVALID\n\n\n";

	//Add data
	output << "Generation:\t" + std::to_string(di.generation);
	output << "\nNoise seed:\t" + std::to_string(di.noiseSeed);
	output << "\nParent A:\t" + std::to_string(di.parentA);
	output << "\nParent B:\t" + std::to_string(di.parentB);

	//Add the alleles
	output << "\n\nAlleles:";
	output << "\nNoise:\t\t\t\t\t\t\t\t\t" + std::to_string(di.noiseSeed);
	output << "\nMinimum distance of the main path:\t\t\t\t\t" + std::to_string(di.minDistanceOfMainPath);
	output << "\nMaximum distance of the main path:\t\t\t\t\t" + std::to_string(di.maxDistanceOfMainPath);
	output << "\nRatio of the length of the main path to the length of side paths:\t" + std::to_string(di.maxDistanceOfMainPath);
	output << "\nMinimum distance before a branch:\t\t\t\t\t" + std::to_string(di.minDistanceBeforeBranch);
	output << "\nRatio of deadends to reconnecting paths:\t\t\t\t" + std::to_string(di.ratioOfDeadEndsToReconnectingBranches);
	output << "\nRatio of length of the main path to the number of side paths:\t\t" + std::to_string(di.ratioOfLengthOfMainPathToNumberOfSidePaths);
}

void DunGen::PrintInfo(DungeonInfo di, int minDistanceOfSidePath, int maxDistanceOfSidePath, int numberOfSidePaths, int lengthOfMainPath) {
	std::ofstream output(di.filePath, std::ofstream::app);

	//Add data
	output << "Generation:\t" + std::to_string(di.generation);
	output << "\nNoise seed:\t" + std::to_string(di.noiseSeed);
	output << "\nParent A:\t" + std::to_string(di.parentA);
	output << "\nParent B:\t" + std::to_string(di.parentB);

	//Add the alleles
	output << "\n\nAlleles:";
	output << "\nNoise:\t\t\t\t\t\t\t\t\t" + std::to_string(di.noiseSeed);
	output << "\nMinimum distance of the main path:\t\t\t\t\t" + std::to_string(di.minDistanceOfMainPath);
	output << "\nMaximum distance of the main path:\t\t\t\t\t" + std::to_string(di.maxDistanceOfMainPath);
	output << "\nRatio of the length of the main path to the length of side paths:\t" + std::to_string(di.maxDistanceOfMainPath);
	output << "\nMinimum distance before a branch:\t\t\t\t\t" + std::to_string(di.minDistanceBeforeBranch);
	output << "\nRatio of deadends to reconnecting paths:\t\t\t\t" + std::to_string(di.ratioOfDeadEndsToReconnectingBranches);
	output << "\nRatio of length of the main path to the number of side paths:\t\t" + std::to_string(di.ratioOfLengthOfMainPathToNumberOfSidePaths);

	//Calculated values
	output << "\n\nCalculated values:";
	output << "\nMinimum length of side paths:\t" + std::to_string(minDistanceOfSidePath);
	output << "\nMaximum length of side paths:\t" + std::to_string(maxDistanceOfSidePath);
	output << "\nNumber of side paths:\t\t" + std::to_string(numberOfSidePaths);
}

void DunGen::PrintDungeon(DungeonInfo di) {
	//"output/" + std::to_string(di.generation) + "_" + std::to_string(di.index) +"_" + std::to_string(di.noiseSeed) +".txt"
	std::ofstream output(di.filePath, std::ofstream::app);


	//Display the axis
	output << "\n\n0 Z+\nX\n+\n";


	Vec3I minBounds = mPathFinding.GetMinBounds();
	Vec3I maxBounds = mPathFinding.GetMaxBounds();

	std::string cBounds = "!!";
	std::string cEmpty = "  ";

	int inPath = -1;
	int pathCounter = 0;

	output << "   ";
	for (int z = 0; z < MAX_AR_Z; z++) {
		if (z % 2 != 0) {
			if (z < 10) {
				output << "0" + std::to_string(z) + "  ";
			}
			else if (z < 100) {
				output << std::to_string(z) + "  ";
			}
			else {
				output << std::to_string(z) + " ";
			}
		}
	}

	for (int x = 0; x < MAX_AR_X; x++) {
		output << "\n";
		if (x < 10) {
			output << "0" + std::to_string(x) + " ";
		}
		else if(x < 100){
			output << std::to_string(x) + " ";
		}
		else {
			output << std::to_string(x);
		}

		for (int z = 0; z < MAX_AR_Z; z++) {

			if (x < minBounds.x  || x >= maxBounds.x || z < minBounds.z || z >= maxBounds.z) {
				output << cBounds;
			}
			else {
				inPath = -1;
				pathCounter = 0;

				while (inPath == -1 && pathCounter < mPaths.size()) {
					for (size_t a = 0; a < mPaths.at(pathCounter).positions.size(); a++) {
						if (mPaths.at(pathCounter).positions.at(a).x == x && mPaths.at(pathCounter).positions.at(a).z == z) {
							inPath = pathCounter;
						}
					}
					pathCounter++;
				}

				if (inPath != -1) {
					if (inPath < 10) {
						output << "0" + std::to_string(inPath);
					}
					else {
						output << std::to_string(inPath);
					}
				}
				else {
					output << cEmpty;
				}
			}
		}
	}
}

//*******************************************
//
//For Fitness
//
//*******************************************

size_t DunGen::GetIndex(int x, int z, Vec3I size) {
	return (size_t)x * (size_t)size.x + (size_t)z;
}


int DunGen::NumberOfParallelPathSpaces() {
	std::map<size_t, std::pair<Vec3I, int>> taken;
	std::map<size_t, bool> removed;
	Vec3I bounds = mPathFinding.GetMaxBounds();
	size_t curr;

	int count = 0;


	for each (Path p in mPaths) {
		for each (Vec3I pos in p.positions) {
			curr = GetIndex(pos.x, pos.z, bounds);
			taken[curr] = std::pair<Vec3I, int>(pos, p.index);
			removed[curr] = false;
		}
	}

	std::map<size_t, std::pair<Vec3I, int>>::iterator it = taken.begin();
	while(it != taken.end()){
		Vec3I p0 = it->second.first;
		int currIndex = it->second.second;

		//Pos X
		size_t pos1 = GetIndex(p0.x + 1, p0.z, bounds);
		size_t pos2 = GetIndex(p0.x + 2, p0.z, bounds);
		if (taken.count(pos1) != 0 && taken.count(pos2))
			if(!removed.at(pos1) && !removed.at(pos2))
				if(taken.at(pos1).second != currIndex && taken.at(pos2).second != currIndex)
					count++;

		//Neg X
		pos1 = GetIndex(p0.x - 1, p0.z, bounds);
		pos2 = GetIndex(p0.x - 2, p0.z, bounds);
		if (taken.count(pos1) != 0 && taken.count(pos2))
			if (!removed.at(pos1) && !removed.at(pos2))
				if (taken.at(pos1).second != currIndex && taken.at(pos2).second != currIndex)
					count++;

		//Pos Z
		pos1 = GetIndex(p0.x, p0.z + 1, bounds);
		pos2 = GetIndex(p0.x, p0.z + 2, bounds);
		if (taken.count(pos1) != 0 && taken.count(pos2))
			if (!removed.at(pos1) && !removed.at(pos2))
				if (taken.at(pos1).second != currIndex && taken.at(pos2).second != currIndex)
					count++;

		//Neg Z
		pos1 = GetIndex(p0.x, p0.z - 1, bounds);
		pos2 = GetIndex(p0.x, p0.z - 2, bounds);
		if (taken.count(pos1) != 0 && taken.count(pos2))
			if (!removed.at(pos1) && !removed.at(pos2))
				if (taken.at(pos1).second != currIndex && taken.at(pos2).second != currIndex)
					count++;

		//Remove the current position from the map so the same parallel sections aren't counted multiple times
		removed[it->first] = true;
		it++;
	}

	return count;
}

int DunGen::TotalLength() {
	size_t count = 0;
	for each (Path p in mPaths) {
		count += p.positions.size();
	}
	return (int)count;
}

int DunGen::NumberOfStraightSectionsToBends() {
	int straightSections = 0;
	int bentSections = 0;

	for each (Path p in mPaths) {
		if (p.positions.size() != 0) {

			Vec3I prevPrevPos = p.positions.at(0);
			Vec3I prevPos = p.positions.at(1);
			Vec3I currPos;

			for (int i = 2; i < p.positions.size(); i++) {
				currPos = p.positions.at(i);

				//If all the positions share an X or Z value, they are in a line
				if (prevPrevPos.x == prevPos.x == currPos.x
					|| prevPrevPos.z == prevPos.z == currPos.z) {
					straightSections++;
				}
				else {
					bentSections++;
				}
			}
		}
	}

	return straightSections - bentSections;
}