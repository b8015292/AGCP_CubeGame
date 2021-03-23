#pragma once

#include "WorldManager.h"
#include "DGPathfinding.h"

struct TempChunk {
public:
	std::shared_ptr<WorldManager::Chunk> chunk;
	std::vector<Vec3I> positions;
};

class DunGen {
public:
	DunGen(std::shared_ptr<WorldManager>);

	void test();

private:
	std::shared_ptr<WorldManager> mWorldMgr;
	DGPathfinding mPathFinding;

	Vec3I mStartPoint;
	Vec3I mEndPoint;
	bool mObstacles[MAX_AR_X][MAX_AR_Y][MAX_AR_Z] = { false };

	std::vector<std::vector<Vec3I>> mPaths;
	std::vector<TempChunk> mSplitPath;

	void GenerateStartAndEnd();
	void GenerateObsticales();
	void GenerateDungeon();
	void SplitPathsIntoChunks();

	void FillFloor();
};