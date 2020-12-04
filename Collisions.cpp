#include "Collision.h"

bool Collision::Within(const ColCube cube, const XMFLOAT3 point) {
	bool inX = false;
	bool inY = false;
	bool inZ = false;

	if (cube.list[EPos::tfl].x <= point.x && cube.list[EPos::tfr].x >= point.x) inX = true;
	if (cube.list[EPos::bfl].y <= point.y && cube.list[EPos::tfr].y >= point.y) inY = true;
	if (cube.list[EPos::bbl].z <= point.z && cube.list[EPos::tfr].z >= point.z) inZ = true;

	if (inX && inY && inZ) return true;

	return false;
}

bool Collision::CheckCollisions(const ColCube a, const ColCube b) {

	for each (XMFLOAT3 f3 in a.list) {
		if (Within(b, f3)) {
			return true;
		}
	}

	return false;


	
}

void Collision::ColCube::Translate(XMFLOAT3 move) {
	DirectX::XMMATRIX translateMatrix = DirectX::XMMatrixTranslation(move.x, move.y, move.z);

	for each (auto& el in list) {
		XMVECTOR temp = XMLoadFloat3(&el);
		temp = XMVector3TransformCoord(temp, translateMatrix);
		XMStoreFloat3(&el, temp);
	}
}

Collision::ColPoints Collision::CheckCollisionPoints(const ColCube Entity, const ColCube Block) {
	Collision::ColPoints ret;

	for (int i = 0; i < (int)EPos::size; i++) {
		if (Within(Block, Entity.list[i])) {
			ret.list[i] = true;
		}
	}

	return ret;
}

void Collision::ColPoints::Reset() {
	for each (bool b in list) {
		b = false;
	}
}

bool Collision::ColPoints::AnyBottom() {
	return list[EPos::bbl] || list[EPos::bbr] || list[EPos::bfl] || list[EPos::bfr];
}
bool Collision::ColPoints::AnyTop() {
	return list[EPos::tbl] || list[EPos::tbr] || list[EPos::tfl] || list[EPos::tfr];
}
bool Collision::ColPoints::AnyLeft() {
	return list[EPos::bbl] || list[EPos::bfl] || list[EPos::tbl] || list[EPos::tfl];
}
bool Collision::ColPoints::AnyRight() {
	return list[EPos::bbr] || list[EPos::bfr] || list[EPos::tbr] || list[EPos::tfr];
}
bool Collision::ColPoints::AnyFront() {
	return list[EPos::bfl] || list[EPos::bfr] || list[EPos::tfl] || list[EPos::tfr];
}
bool Collision::ColPoints::AnyBack() {
	return list[EPos::bbl] || list[EPos::bbr] || list[EPos::tbl] || list[EPos::tbr];
}