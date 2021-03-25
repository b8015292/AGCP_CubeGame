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
	mWorldMgr = wrlmgr;

	GenerateStartAndEnd();
	GenerateObsticales();
	GenerateDungeon();

	FillFloor();

}

void DunGen::GenerateStartAndEnd() {
	mStartPoint.x = 50;
	mStartPoint.y = 5;
	mStartPoint.z = 50;

	mEndPoint.x = 53;
	mEndPoint.y = 5;
	mEndPoint.z = 53;
}


void DunGen::GenerateObsticales() {
	//mObstacles[8][7][7] = true;
	//mObstacles[8][6][7] = true;
	//mObstacles[8][8][7] = true;
}

void DunGen::GenerateDungeon() {
	mPathFinding.Init(mWorldMgr, mObstacles);
	std::vector<Vec3I> main = mPathFinding.AStar(mStartPoint, mEndPoint);
	mPaths.push_back(main);
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