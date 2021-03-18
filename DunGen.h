#pragma once

#include "WorldManager.h"
#include "DGPathfinding.h"

struct Vec3I {
public:
	Vec3I() = default;
	Vec3I(int x, int y, int z);
	Vec3I(float x, float y, float z);

	Vec3I& operator+(const Vec3I&);
	Vec3I& operator-(const Vec3I&);
	Vec3I& operator+=(const Vec3I&);
	Vec3I& operator-=(const Vec3I&);

	bool operator==(const Vec3I&);
	bool operator!=(const Vec3I&);

	int x = 0;
	int y = 0;
	int z = 0;
};

class DunGen {
public:
	DunGen(std::shared_ptr<WorldManager>);

	void test();

private:
	std::shared_ptr<WorldManager> mWorldMgr;

	Vec3I mStartPoint;
	Vec3I mEndPoint;
	std::vector<Node> mMainPath;


	void GenerateStartAndEnd();
	void GenerateObsticales();
	void GenerateDungeon();
};