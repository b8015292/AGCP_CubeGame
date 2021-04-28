#pragma once

#include "WorldManager.h"
#include "DGPathfinding.h"
#include "PerlinNoise.h"
#include <ctime>
#include <fstream>

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

struct DungeonInfo {
public:
	DungeonInfo(std::string nFilePath, size_t nindex, size_t ngeneration, size_t nparentA, size_t nparentB, unsigned int nnoiseSeed, int nminDistanceOfMainPath, int nmaxDistanceOfMainPath, float nratioOfLengthOfMainPathToSidePaths, int nminDistanceBeforeBranch, float nratioOfDeadEndsToReconnectingBranches, float nratioOfLengthOfMainPathToNumberOfSidePaths) {
		//filePath = nFilePath + std::to_string(ngeneration) + "_" + std::to_string(nindex) + "_" + std::to_string(nnoiseSeed) + ".txt";
		filePath = nFilePath + std::to_string(ngeneration) + "_" + std::to_string(nindex) + ".txt";

		index = nindex;
		generation = ngeneration;
		parentA = nparentA;
		parentB = nparentB;

		noiseSeed = nnoiseSeed;
		minDistanceOfMainPath = nminDistanceOfMainPath;
		maxDistanceOfMainPath = nmaxDistanceOfMainPath;
		ratioOfLengthOfMainPathToSidePaths = nratioOfLengthOfMainPathToSidePaths;
		minDistanceBeforeBranch = nminDistanceBeforeBranch;
		ratioOfDeadEndsToReconnectingBranches = nratioOfDeadEndsToReconnectingBranches;
		ratioOfLengthOfMainPathToNumberOfSidePaths = nratioOfLengthOfMainPathToNumberOfSidePaths;
	};

	DungeonInfo(std::string nFilePath, size_t nindex, size_t ngeneration, size_t nparentA, size_t nparentB) {
		filePath = nFilePath + std::to_string(ngeneration) + "_" + std::to_string(nindex) + ".txt";

		index = nindex;
		generation = ngeneration;
		parentA = nparentA;
		parentB = nparentB;
	};

	DungeonInfo() {};

	size_t index = -1;
	size_t generation = -1;
	size_t parentA = -1;
	size_t parentB = -1;

	unsigned int noiseSeed = -1;
	int minDistanceOfMainPath = -1;
	int maxDistanceOfMainPath = -1;
	float ratioOfLengthOfMainPathToSidePaths = -1.f;
	int minDistanceBeforeBranch = -1;
	float ratioOfDeadEndsToReconnectingBranches = -1.f;
	float ratioOfLengthOfMainPathToNumberOfSidePaths = -1.f;

	std::string filePath = "NO_PATH_SET";
};

class DunGen {
public:
	DunGen(std::shared_ptr<WorldManager> wrlmgr);
	void Init(DungeonInfo di);

	void Spawn(bool walls);
	void Output();

	//For fitness funciton
	bool IsValid() { return mValid; };
	int NumberOfParallelPathSpaces();
	int TotalLength();
	int NumberOfStraightSectionsToBends();

private:
	bool mValid = false;

	std::shared_ptr<WorldManager> mWorldMgr;
	DGPathfinding mPathFinding;
	PerlinNoise mNoise;

	Vec3I mWorldSize;
	//Vec3I mStartPoint;
	//Vec3I mEndPoint;
	bool mObstacles[MAX_AR_X][MAX_AR_Y][MAX_AR_Z] = { false };
	bool mObstaclePaths[MAX_AR_X][MAX_AR_Y][MAX_AR_Z] = { false };

	std::vector<Path> mPaths;
	std::vector<TempChunk> mSplitPath;

	DungeonInfo mDungeonInfo;
	int mMinDistanceOfSidePath;
	int mMaxDistanceOfSidePath;
	int mNumberOfSidePaths;

	void Init(std::shared_ptr<WorldManager> worldMgr, unsigned int  noiseSeed);
	bool IsValidDungeonInfo(DungeonInfo& di);

	void GenerateMainStartAndEnd(int minDistance, int maxDistance);
	void GenerateObsticales(double chance);
	void GeneratePath(size_t index);

	void GenerateMainPath(int minDistance, int maxDistance);
	void GenerateSidePath(size_t parentPath, bool deadend, int minDistance, int maxLength);
	void GenerateSidePaths(int numberToGenerate, int minDistance, int maxDistance, int minDistanceBeforeBranch, float ratioOfDeadEndsToReconnectingBranches);

	void GenerateSideStart(size_t index);
	void GenerateSideDeadEnd(size_t index, int minLength, int maxDistance);
	void GenerateSideLinkEnd(size_t index, int minLength, int maxDistance);

	void AddPathsToPathList(size_t index);

	void SetBlock(Vec3I pos, std::string material, bool active = true);

	bool IsPositionValid(size_t index, int minDistance, int maxDistance);
	size_t GetIndex(int x, int z, Vec3I size);

	void PrintInfo(DungeonInfo di, int minDistanceOfSidePath, int maxDistanceOfSidePath, int numberOfSidePaths, int lengthOfMainPath);
	void PrintDungeon(DungeonInfo di);
	void PrintInvalid(DungeonInfo di);
};