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
}

void DunGen::GenerateStartAndEnd() {
	mStartPoint.x = 7;
	mStartPoint.y = 7;
	mStartPoint.z = 7;

	mEndPoint.x = 11;
	mEndPoint.z = 7;
	mEndPoint.y = 7;
}

void DunGen::GenerateObsticales() {

}

void DunGen::GenerateDungeon() {

	Node start;
	start.x = mStartPoint.x;
	start.y = mStartPoint.y;
	start.z = mStartPoint.z;

	Node end;
	end.x = mEndPoint.x;
	end.y = mEndPoint.y;
	end.z = mEndPoint.z;

	DGPathfinding pf(mWorldMgr);
	mMainPath = pf.AStar(start, end);


}

void DunGen::test() {

}