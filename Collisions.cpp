#include "Collision.h"

bool Collision::Within(const ColCube cube, const XMFLOAT3 point) {
	bool inX = false;
	bool inY = false;
	bool inZ = false;

	if (cube.tfl.x <= point.x && cube.tfr.x >= point.x) inX = true;
	if (cube.bfl.y <= point.y && cube.tfr.y >= point.y) inY = true;
	if (cube.bbl.z <= point.z && cube.tfr.z >= point.z) inZ = true;

	if (inX && inY && inZ) return true;

	return false;
}

bool Collision::CheckCollisions(const ColCube a, const ColCube b) {
	
	//HOLD THIS INFORMATION TO DETERMINE THE DIRECTION OF COLLISION

	if (Within(b, a.bbl)) {
		return true;
	}
	else if (Within(b, a.bbr)) {
		return true;
	}
	else if (Within(b, a.bfl)) {
		return true;
	}
	else if (Within(b, a.bfr)) {
		return true;
	}
	else if (Within(b, a.tbl)) {
		return true;
	}
	else if (Within(b, a.tbr)) {
		return true;
	}
	else if (Within(b, a.tfl)) {
		return true;
	}
	else if (Within(b, a.tfr)) {
		return true;
	}

	return false;
}

Collision::ColPoints Collision::CheckCollisionPoints(const ColCube Entity, const ColCube Block) {
	Collision::ColPoints ret;
	if (Within(Block, Entity.bbl)) {
		ret.bbl = true;
	}
	else if (Within(Block, Entity.bbr)) {
		ret.bbr = true;
	}
	else if (Within(Block, Entity.bfl)) {
		ret.bfl = true;
	}
	else if (Within(Block, Entity.bfr)) {
		ret.bfr = true;
	}
	else if (Within(Block, Entity.tbl)) {
		ret.tbl = true;
	}
	else if (Within(Block, Entity.tbr)) {
		ret.tbr = true;
	}
	else if (Within(Block, Entity.tfl)) {
		ret.tfl = true;
	}
	else if (Within(Block, Entity.tfr)) {
		ret.tfr = true;
	}
	return ret;
}

void Collision::ColPoints::Reset() {
	tfl = false;
	tfr = false;
	tbl = false;
	tbr = false;
	bfl = false;
	bfr = false;
	bbl = false;
	bbr = false;
}

bool Collision::ColPoints::AnyBottom() {
	return bbl || bbr || bfl || bfr;
}
bool Collision::ColPoints::AnyTop() {
	return tbl || tbr || tfl || tfr;
}
bool Collision::ColPoints::AnyLeft() {
	return bbl || bfl || tbl || tfl;
}
bool Collision::ColPoints::AnyRight() {
	return bbr || bfr || tbr || tfr;
}
bool Collision::ColPoints::AnyFront() {
	return bfl || bfr || tfl || tfr;
}
bool Collision::ColPoints::AnyBack() {
	return bbl || bbr || tbl || tbr;
}