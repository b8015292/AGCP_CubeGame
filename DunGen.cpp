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

DunGen::DunGen(std::shared_ptr<WorldManager> wrlmgr) {
	//Set the world manager and world size
	mWorldMgr = wrlmgr;
	WorldManager::Pos maxSize = mWorldMgr->GetWorldSize();
	mWorldSize = { maxSize.x, maxSize.y, maxSize.z };

	mPathFinding.Init(mWorldMgr);

	//Randomize the noise
	srand(std::time(NULL));
	unsigned int seed = rand() % 10000;
	mNoise = PerlinNoise(seed);
	srand(mNoise.GetSeed());

	GenerateMainPath(30);
	GenerateSidePath(0, true, 20);


	FillFloor();

}

void DunGen::GenerateMainPath(int minLength) {
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
		GenerateMainStartAndEnd(minLength);

		//Fill in the first path, keep re-making the obsticales until a path is found
		while (mPaths.at(0).positions.empty() && obsticalAttempts < maxObsticalAttempts) {
			GenerateObsticales(obsticalChance);
			mPathFinding.SetObstcales(mObstacles);
			GeneratePath(0);

			obsticalAttempts++;
			obsticalChance -= obsticalChanceDecrement;

		}
	} while (mPaths.at(0).positions.empty());
}

void DunGen::GenerateSidePath(size_t parentPath, bool deadend, int minLength) {
	Path p;
	p.index = mPaths.size();
	p.parentIndex = parentPath;
	mPaths.push_back(p);
	mPaths.at(parentPath).children.push_back(p.index);

	GenerateSideStart(p.index);

	if (deadend) {
		GenerateSideDeadEnd(p.index, minLength);
	}
	else {
		GenerateSideLinkEnd(p.index, minLength);
	}

	GeneratePath(p.index);
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

void DunGen::GenerateSideDeadEnd(size_t index, int minLength) {
	int distance = -1;
	do {
		mPaths.at(index).end = mPaths.at(0).start;
		mPaths.at(index).end.x += ((rand() % MAX_AR_X) - MAX_AR_X * 0.5f) - 1;
		mPaths.at(index).end.z += ((rand() % MAX_AR_Z) - MAX_AR_Z * 0.5f) - 1;

		distance = abs(mPaths.at(index).start.x - mPaths.at(index).end.x) + abs(mPaths.at(index).start.z - mPaths.at(index).end.z);
	} while (mPaths.at(index).end == mPaths.at(index).start
		|| mPaths.at(index).end.x < 0 || mPaths.at(index).end.z < 0
		|| mPaths.at(index).end.x >= mWorldSize.x || mPaths.at(index).end.z >= mWorldSize.z
		|| distance < minLength
		);
}

void DunGen::GenerateSideLinkEnd(size_t index, int minLength) {
	size_t size = mPaths.at(mPaths.at(index).parentIndex).positions.size();

	do {
		//Exclude the first and last index
		size_t endIndex = 1 + (((size_t)rand() % size) - 3);

		//Set the start of this path to a position in its parents path
		mPaths.at(index).start = mPaths.at(mPaths.at(index).parentIndex).positions.at(endIndex);
	} while (mPaths.at(index).start == mPaths.at(index).end);
}

void DunGen::GenerateMainStartAndEnd(int minDistance) {

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

	} while ((mPaths.at(0).end == mPaths.at(0).start)
		|| mPaths.at(0).end.x < 0 || mPaths.at(0).end.z < 0
		|| mPaths.at(0).end.x >= mWorldSize.x || mPaths.at(0).end.z >= mWorldSize.z
		|| distance < minDistance
		);

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

void DunGen::SplitPathsIntoChunks() {

}

void DunGen::FillFloor() {
	for each (Path p in mPaths) {
		for each (Vec3I pos in p.positions) {
 			std::shared_ptr<WorldManager::Chunk> chunk = mWorldMgr->GetChunkFromWorldCoords({ (float)pos.x, (float)pos.y, (float)pos.z });
			WorldManager::Pos chunkPos = chunk->GetPos();

			WorldManager::Pos coordsInChunkSpace(pos.x - chunkPos.x * WorldManager::sChunkDimension, pos.y - chunkPos.y * WorldManager::sChunkDimension, pos.z - chunkPos.z * WorldManager::sChunkDimension);

			std::shared_ptr<Block> block = chunk->GetBlock(coordsInChunkSpace);
			block->SetActive(true);
			block->ChangeMaterial("mat_oak_log");
		}
	}
}

void DunGen::test() {

}