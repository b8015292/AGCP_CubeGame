#pragma once

#pragma once

#include <math.h>
#include <vector>
#include <array>
#include <stack>
#include <vector>
#include "WorldManager.h"

//Maximum size of the array - not world size
#define MAX_AR_X 128
#define MAX_AR_Y 1
#define MAX_AR_Z 128
#define MAX_TOTAL MAX_AR_X * MAX_AR_Y * MAX_AR_Z

//const int intmax = 32767;
//const float floatmax = 3.402823466e+38F;
//const size_t MAX_AR_X = 12;
//const size_t MAX_AR_Y = 12;
//const size_t MAX_AR_Z = 12;

const int intmax = 2147483647;
const float floatmax = 3.40282e+38F;

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

struct Node
{
public:
	int x = 0;
	int y = 0;
	int z = 0;
	int parentX = -1;
	int parentY = -1;
	int parentZ = -1;
	//int buffer3 = -1;
	//int buffer4 = -1;

	size_t indexX = 0;
	size_t indexY = 0;
	size_t indexZ = 0;
	size_t parentIndexX = 0;
	size_t parentIndexY = 0;
	size_t parentIndexZ = 0;
	//size_t buffer1 = 0;
	//size_t buffer2 = 0;

	//float gCost = FLT_MAX;
	float gCost = floatmax;
	float hCost = floatmax;
	float fCost = floatmax;
	//float buffer5 = floatmax;

	void SetNode(size_t ix, size_t iy, size_t iz, int nx, int ny, int nz, size_t pix, size_t piy, size_t piz, int npx, int npy, int npz, float ngc = floatmax, float nhc = floatmax, float nfc = floatmax) {
		indexX = ix;
		indexY = iy;
		indexZ = iz;
		x = nx;
		y = ny;
		z = nz;
		parentIndexX = pix;
		parentIndexY = piy;
		parentIndexZ = piz;
		parentX = npx;
		parentY = npy;
		parentZ = npz;
		gCost = ngc;
		hCost = nhc;
		fCost = nfc;
	}
	void SetIndex(size_t ix, size_t iy, size_t iz) {
		indexX = ix;
		indexY = iy;
		indexZ = iz;
	}
	void SetCoords(int nx, int ny, int nz) {
		x = nx;
		y = ny;
		z = nz;
	}
	void SetParent(int npx, int npy, int npz, size_t pix, size_t piy, size_t piz) {
		parentX = npx;
		parentY = npy;
		parentZ = npz;
		parentIndexX = pix;
		parentIndexY = piy;
		parentIndexZ = piz;
	}
	void SetCosts(float ngc, float nhc, float nfc) {
		gCost = ngc;
		hCost = nhc;
		fCost = nfc;
	}
};

inline bool operator < (const Node& lhs, const Node& rhs)
{
	return lhs.fCost < rhs.fCost;
}

class DGPathfinding
{
public:
	DGPathfinding();

	void Init(std::shared_ptr<WorldManager>);
	void SetObstcales(bool obstacles[MAX_AR_X][MAX_AR_Y][MAX_AR_Z]);
	void SetMainPath(Vec3I start, Vec3I end);
	void AddPathToObstacles(std::vector<Vec3I> path);

	std::vector<Vec3I> AStar(Vec3I start, Vec3I destination);

	Vec3I GetMinBounds() { return mMinBounds; };
	Vec3I GetMaxBounds() { return mMaxBounds; };

private:
	size_t GetIndexOf3DArray(size_t x, size_t y, size_t z);
	bool IsValidIndex(size_t x, size_t y, size_t z);
	bool IsValidWorldCoord(Vec3I pos);
	bool IsDestination(int x, int y, int z, Node destination);
	int CalculateH(int x, int y, int z, Node destination);

	std::vector<Vec3I> MakePath(Node map[MAX_TOTAL], Node destination);

	void SetWorldBounds(int minX, int maxX, int minZ, int maxZ);

	std::shared_ptr<WorldManager> mWorldManager;
	Vec3I mWorldSize;

	bool mObstacles[MAX_AR_X][MAX_AR_Y][MAX_AR_Z];
	Vec3I mMinBounds = { -1, -1, -1 };
	Vec3I mMaxBounds = { -1, -1, -1 };

	Node mAllMap[MAX_TOTAL];
	Vec3I mStartPoint;

	DirectX::XMINT3 mNeighbors[4] = {
		{1, 0, 0},
		{-1, 0, 0},
		{0, 0, 1},
		{0, 0, -1},
		//{0, 1, 0},
		//{0, -1, 0},
	};
};