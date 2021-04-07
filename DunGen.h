#pragma once

#include "WorldManager.h"
#include "DGPathfinding.h"
#include "PerlinNoise.h"
#include <ctime>


struct TempChunk {
public:
	std::shared_ptr<WorldManager::Chunk> chunk;
	std::vector<Vec3I> positions;
};

struct Path {
public:
	size_t index;
	size_t parentIndex;
	Vec3I start;
	Vec3I end;

	std::vector<Vec3I> positions;
	std::vector<size_t> children;
};

class DunGen {
public:
	DunGen(std::shared_ptr<WorldManager>);

	void test();

private:
	std::shared_ptr<WorldManager> mWorldMgr;
	DGPathfinding mPathFinding;
	PerlinNoise mNoise;

	Vec3I mWorldSize;
	//Vec3I mStartPoint;
	//Vec3I mEndPoint;
	bool mObstacles[MAX_AR_X][MAX_AR_Y][MAX_AR_Z] = { false };

	std::vector<Path> mPaths;
	std::vector<TempChunk> mSplitPath;

	void GenerateMainStartAndEnd(int minDistance);
	void GenerateObsticales(double chance);
	void GeneratePath(size_t index);
	void SplitPathsIntoChunks();

	void GenerateMainPath(int minLength);
	void GenerateSidePath(size_t parentPath, bool deadend, int minLength);

	void GenerateSideStart(size_t index);
	void GenerateSideDeadEnd(size_t index, int minLength);
	void GenerateSideLinkEnd(size_t index, int minLength);

	void FillFloor();
};