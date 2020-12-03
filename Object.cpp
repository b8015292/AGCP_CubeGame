#include "Object.h"

const float GameData::sGrav = -9.71f;
int GameObject::sMaxID = 0;

GameObject::GameObject(std::shared_ptr<std::vector<std::shared_ptr<GameObject>>> allGObjs) : mRI() {
	mAllGObjs = allGObjs;
	if(mID == 0) mID = ++sMaxID;	//Incase a entity is being made from a preconstructed GObj

}

void GameObject::CreateBoundingBox() {
	Collision::ColCube coords = GetCoords();

	XMFLOAT3 topFrontRight = coords.list[Collision::EPos::tfr];
	XMFLOAT3 backBottomLeft = coords.list[Collision::EPos::bbl];

	float xDist = topFrontRight.x - backBottomLeft.x;
	float yDist = topFrontRight.y - backBottomLeft.y;
	float zDist = topFrontRight.z - backBottomLeft.z;

	XMFLOAT3 origin = { topFrontRight.x - (xDist / 2), topFrontRight.y - (yDist / 2), topFrontRight.z - (zDist / 2) };
	XMFLOAT3 extents = { xDist / 2, yDist / 2, zDist / 2 };

	BoundingBox box = BoundingBox(origin, extents);
	boundingBox = box;
}

GameObject::GameObject(std::shared_ptr<GameObject> gobj) : mRI(){
	*this = *gobj;
}

void GameObject::SetActive(bool val) {
	active = val;
	mRI->active = val;
}

Collision::ColCube GameObject::GetCoords() {
	//Define constants
	const UINT vertsPerObj = 24;
	const UINT vertsNeeded = 8;
	const UINT numbOfVerts = vertsPerObj * (UINT)(mAllGObjs->size());
	const UINT vbByteSize = numbOfVerts * sizeof(Vertex);

	//Where the verticies for this item start in the buffer
	const int vertStart = mRI->BaseVertexLocation;

	//A pointer to the buffer of vertices
	ComPtr<ID3DBlob> verticesBlob = mRI->Geo->VertexBufferCPU;

	//Move the data from the buffer onto a vector we can view/manipulate
	std::vector<Vertex> vs(numbOfVerts);
	CopyMemory(vs.data(), verticesBlob->GetBufferPointer(), vbByteSize);

	//Get the items world transformation matrix
	XMMATRIX transform;
	GameData::StoreFloat4x4InMatrix(transform, mRI->World);

	//Transform each vertex by its world matrix
	for (int i = vertStart; i < (vertStart + (int)vertsNeeded); i++) {

		XMVECTOR temp = XMLoadFloat3(&vs.at(i).Pos);;
		temp = XMVector3TransformCoord(temp, transform);

		XMStoreFloat3(&vs.at(i).Pos, temp);
	}

	////Store all the proper positions in a struct
	Collision::ColCube c(vs[vertStart + 7].Pos, vs[vertStart + 6].Pos, vs[vertStart + 1].Pos, vs[vertStart + 2].Pos, vs[vertStart + 4].Pos, vs[vertStart + 5].Pos, vs[vertStart].Pos, vs[vertStart + 3].Pos);

	return c;
}

void GameObject::Translate(const float dTime, float x, float y, float z) {
	//Gets the translation matrix (scaled by delta time
	DirectX::XMMATRIX translateMatrix = DirectX::XMMatrixTranslation(x * dTime, y * dTime, z * dTime);

	//Gets the world matrix in XMMATRIX form
	DirectX::XMMATRIX oldWorldMatrix;
	GameData::StoreFloat4x4InMatrix(oldWorldMatrix, mRI->World);

	//Multiplies the two matricies together
	DirectX::XMMATRIX newWorldMatrix = XMMatrixMultiply(oldWorldMatrix, translateMatrix);

	//Stores the new matrix, and marks the object as dirty
	XMStoreFloat4x4(&mRI->World, newWorldMatrix);
	mRI->NumFramesDirty++;
	boundingBox.Center = { boundingBox.Center.x + x, boundingBox.Center.y + y,  boundingBox.Center.z + z };
}

Entity::Entity(std::shared_ptr<std::vector<std::shared_ptr<GameObject>>> allGObjs) : GameObject(allGObjs) {
	Init();
}

Entity::Entity(std::shared_ptr<GameObject> gobj) : GameObject(gobj) {
	Init();
}

void Entity::Init() {
	mID = GameObject::mID;
	mVel = { 0, 0, 0 };
	mMaxVel = { 1.f, 1.f, 1.f };
}

void Entity::Update(const float dTime) {
	if (!GetActive()) return;

	Collision::ColCube coords = GetCoords();
	Collision::ColCube nextCoords = coords;
	nextCoords.Translate({ mVel.x * dTime, mVel.y * dTime, mVel.z * dTime });

	mColPoints = GetAllCollisionPoints(nextCoords);

	if (applyGravity) {
		if (mColPoints.AnyBottom()) {
			mVel.y = 0.0f;
		}
		else {
			AddVelocity(0, GameData::sGrav / 10.f, 0);
		}
	}

	if (mVel.x != 0 || mVel.y != 0 || mVel.z != 0) {
		Translate(dTime, mVel.x, mVel.y, mVel.z);
	}
}

void Entity::AddVelocity(float x, float y, float z) {
	if (x != 0 && mVel.x < abs(mMaxVel.x)) {
		mVel.x += x;
	}
	if (y != 0 && mVel.y < abs(mMaxVel.y)) {
		mVel.y += y;
	}
	if (z != 0 && mVel.z < abs(mMaxVel.z)) {
		mVel.z += z;
	}
}

std::vector<int> Entity::CheckAllCollisions(Collision::ColCube thisCube) {
	std::vector<int> ret;

	for (int i = 0; i < mAllGObjs->size(); i++) {
		if (mAllGObjs->at(i)->mID != mID) {
			if (Collision::CheckCollisions(thisCube, mAllGObjs->at(i)->GetCoords())) {
				ret.push_back(i);
			}
		}
	}

	return ret;
}

Collision::ColPoints Entity::GetAllCollisionPoints(Collision::ColCube coordinates) {
	Collision::ColPoints ret;

	for (int i = 0; i < mAllGObjs->size(); i++) {
		if (mAllGObjs->at(i)->GetActive() && mAllGObjs->at(i)->mID != mID) {
			ret += Collision::CheckCollisionPoints(coordinates, mAllGObjs->at(i)->GetCoords());
		}
	}

	return ret;
}

bool Entity::IsPointColliding(const XMFLOAT3 point) {

	for (int i = 0; i < mAllGObjs->size(); i++) {
		if(Collision::Within(mAllGObjs->at(i)->GetCoords(), point)) return true;
	}

	return false;
}

Player::Player(std::shared_ptr<std::vector<std::shared_ptr<GameObject>>> allGObjs) : Entity(allGObjs) {

}
Player::Player(std::shared_ptr<GameObject> gobj) : Entity(gobj) {

}
void Player::Update(const float dTime) {
	if (!GetActive()) return;

	Collision::ColCube coords = GetCoords();
	Collision::ColCube nextCoords = coords;
	nextCoords.Translate({ mVel.x * dTime, mVel.y * dTime, mVel.z * dTime });

	mColPoints = GetAllCollisionPoints(nextCoords);

	if (applyGravity) {
		if (mColPoints.AnyBottom()) {
			mVel.y = 0.0f;
		}
		else {
			AddVelocity(0, GameData::sGrav / 10.f, 0);
		}
	}

	if (mVel.x != 0 || mVel.y != 0 || mVel.z != 0) {
		Translate(dTime, mVel.x, mVel.y, mVel.z);
		//Translate camera
		TranslateCamera(dTime, mVel.x, mVel.y, mVel.z);
		//Translate ui
		//???
	}
}

void Player::TranslateCamera(float dTime, float x, float y, float z) {
	
}