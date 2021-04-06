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

	//Randomize the noise
	srand(std::time(NULL));
	unsigned int seed = rand() % 10000;
	mNoise = PerlinNoise(seed);
	srand(mNoise.GetSeed());

	//Add an empty path
	std::vector<Vec3I> empty;
	mPaths.push_back(empty);

	int obsticalAttempts;
	int maxObsticalAttempts = 10;

	//Create the start/end points and obsticals and path. Re create the start/end if no path can be found
	do{
		obsticalAttempts = 0;

		//Generate a random start and end point
		GenerateStartAndEnd();

		//Fill in the first path, keep re-making the obsticales until a path is found
		while (mPaths.at(0).empty() && obsticalAttempts < maxObsticalAttempts) {
			GenerateObsticales(0.5);
			GenerateDungeon();
			obsticalAttempts++;
		}
	} while (mPaths.at(0).empty());

	FillFloor();

}

void DunGen::GenerateStartAndEnd() {
	mStartPoint.x = 50;
	mStartPoint.y = 5;
	mStartPoint.z = 50;

	//Randomize the start
	//mStartPoint.x = (rand() % mWorldSize.x);
	//mStartPoint.y = (rand() % mWorldSize.y);
	//mStartPoint.z = (rand() % mWorldSize.z);

	//Randomize the end. Cannot be more than half of MAX_AR_X or MAX_AR_Z away from the start
	do {
		mEndPoint = mStartPoint;
		mEndPoint.x += (rand() % MAX_AR_X) - MAX_AR_X * 0.5f;
		mEndPoint.z += (rand() % MAX_AR_Z) - MAX_AR_Z * 0.5f;
	} while ((mEndPoint.x == mStartPoint.x && mEndPoint.z == mStartPoint.z)
		|| mEndPoint.x < 0 || mEndPoint.z < 0
		|| mEndPoint.x >= mWorldSize.x || mEndPoint.z >= mWorldSize.z
		);
}


void DunGen::GenerateObsticales(double chance) {
	//Re-randomize the noise each time this function is run to generate new obsticales each time
	//unsigned int seed = rand() % 10000;
	//mNoise = PerlinNoise(seed);
	//srand(mNoise.GetSeed());

	for (size_t x = 0; x < MAX_AR_X; x++) {
		for (size_t z = 0; z < MAX_AR_Z; z++) {
			//Octaves is the number of times it applies noise to itself
			//Persistance is height of the noise
			double n = mNoise.OctavePerlin((double)x * 0.002, (double)mStartPoint.y, (double)z * 0.002, 8, 20.0);
			if (n > chance) {
				mObstacles[x][0][z] = true;
			}
		}
	}
}

void DunGen::GenerateDungeon() {
	mPathFinding.Init(mWorldMgr, mObstacles);
	std::vector<Vec3I> main = mPathFinding.AStar(mStartPoint, mEndPoint);
	//mPaths.push_back(main);
	mPaths.at(0) = main;
}

void DunGen::SplitPathsIntoChunks() {

}

void DunGen::FillFloor() {
	for each (std::vector<Vec3I> vec in mPaths) {
		for each (Vec3I pos in vec) {
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