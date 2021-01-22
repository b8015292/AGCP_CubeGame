#include "Object.h"
#include <algorithm>

const float GameData::sGrav = -9.71f;
int GameObject::sMaxID = 0;
//std::unordered_map<std::string, DirectX::XMFLOAT2> Block::mBlockTexturePositions;

//************************************************************************************************************
// GameObject
//************************************************************************************************************

GameObject::GameObject(std::shared_ptr<std::vector<std::shared_ptr<GameObject>>> allGObjs, std::shared_ptr<RenderItem> rI) {
	mAllGObjs = allGObjs;
	mRI = rI;

	if (mID == 0) mID = ++sMaxID;	//Incase an entity is being made from a preconstructed GObj

	CreateBoundingBox();
}

GameObject::GameObject(std::shared_ptr<GameObject> gobj) : mRI(gobj->GetRI()){
	*this = *gobj;
}

GameObject::~GameObject() {
	--sMaxID;					//For degbugging
	mAllGObjs.~shared_ptr();	//Delete the pointer to the list of all game objects
	mRI.~shared_ptr();			//Delete the pointer to this render item
}

void GameObject::CreateBoundingBox() {
	std::array<XMFLOAT3, 8> coords = GetCoords();

	XMFLOAT3 topFrontRight = coords[EPos::tfr];
	XMFLOAT3 backBottomLeft = coords[EPos::bbl];

	float xDist = topFrontRight.x - backBottomLeft.x;
	float yDist = topFrontRight.y - backBottomLeft.y;
	float zDist = topFrontRight.z - backBottomLeft.z;

	XMFLOAT3 origin = { topFrontRight.x - (xDist / 2), topFrontRight.y - (yDist / 2), topFrontRight.z - (zDist / 2) };
	XMFLOAT3 extents = { xDist / 2, yDist / 2, zDist / 2 };

	BoundingBox box = BoundingBox(origin, extents);
	mBoundingBox = box;
}



void GameObject::SetActive(bool val) {
	mActive = val;
	mRI->active = val;
	SetDirtyFlag();
}

std::array<XMFLOAT3, 8> GameObject::GetCoords() {
	//Define constants
	const UINT vertsPerObj = 24;
	const UINT vertsNeeded = 8;
	const UINT numbOfVerts = vertsPerObj * (UINT)(mRI->Geo->DrawArgs.size());
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
	//Collision::ColCube c(vs[vertStart + 7].Pos, vs[vertStart + 6].Pos, vs[vertStart + 1].Pos, vs[vertStart + 2].Pos, vs[vertStart + 4].Pos, vs[vertStart + 5].Pos, vs[vertStart].Pos, vs[vertStart + 3].Pos);
	std::array<XMFLOAT3, 8> c = { vs[vertStart + 7].Pos, vs[vertStart + 6].Pos, vs[vertStart + 1].Pos, vs[vertStart + 2].Pos, vs[vertStart + 4].Pos, vs[vertStart + 5].Pos, vs[vertStart].Pos, vs[vertStart + 3].Pos };

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

	//Translate the bounding box
	mBoundingBox.Transform(mBoundingBox, translateMatrix);
}

void GameObject::SetPosition(XMFLOAT3 pos) {
	DirectX::XMMATRIX translateMatrix = DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);

	//Stores the new matrix, and marks the object as dirty
	XMStoreFloat4x4(&mRI->World, translateMatrix);
	SetDirtyFlag();

	CreateBoundingBox();
}

//************************************************************************************************************
// Entity
//************************************************************************************************************

Entity::Entity(std::shared_ptr<GameObject> gobj) : GameObject(gobj) {
	Init();
}

Entity::~Entity() {
	mAllGObjs.~shared_ptr();	//Delete the pointer to the list of all game objects
	mRI.~shared_ptr();			//Delete the pointer to this render item
}

void Entity::Init() {
	mID = GameObject::mID;
	mVel = { 0, 0, 0 };
	mMaxVel = { 50.f, 50.f, 50.f };
}

void Entity::Update(const float dTime) {
	if (!GetActive()) return;

	//Collision::ColCube coords = GetCoords();
	//Collision::ColCube nextCoords = coords;
	//nextCoords.Translate({ mVel.x * dTime, mVel.y * dTime, mVel.z * dTime });

	//mColPoints = GetAllCollisionPoints(nextCoords);

	//Create a bounding box in the next location
	DirectX::FXMMATRIX translate = DirectX::XMMatrixTranslation(mVel.x * dTime, mVel.y * dTime, mVel.z * dTime);
	BoundingBox nextBox;
	mBoundingBox.Transform(nextBox, translate);

	//Check if the next location is colliding
	if (CheckIfCollidingAtBox(nextBox)) {
		mVel.y = 0.0f;
	}

	if (mApplyGravity) {
		//Check if the next location is colliding
		if (CheckIfCollidingAtBox(nextBox)) {
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

void Entity::SetVelocity(XMFLOAT3 newVel) {
	mVel = newVel;
}
void Entity::SetMaxVelocity(XMFLOAT3 newMaxVel) {
	mMaxVel = newMaxVel;
}

std::vector<int> Entity::CheckAllCollisionsAtBox(BoundingBox nextPos) {
	std::vector<int> collisionIndexs;
	for (int i = 0; i < mAllGObjs->size(); i++){
		if (mAllGObjs->at(i)->GetActive() && nextPos.Contains(mAllGObjs->at(i)->GetBoundingBox()) != DirectX::ContainmentType::DISJOINT) {
			collisionIndexs.push_back(i);
		}
	}
	return collisionIndexs;
}

bool Entity::CheckIfCollidingAtBox(BoundingBox nextPos) {
	for (int i = 0; i < mAllGObjs->size(); i++) {
		//If the IDs arent the same, the block is active, and it is colliding
		if (mID != mAllGObjs->at(i)->GetID() && mAllGObjs->at(i)->GetActive() && nextPos.Contains(mAllGObjs->at(i)->GetBoundingBox()) != DirectX::ContainmentType::DISJOINT) {
			return true;
		}
	}
	return false;
}

//************************************************************************************************************
// Player
//************************************************************************************************************

Player::Player(std::shared_ptr<GameObject> gobj) : Entity(gobj) {

}
Player::~Player() {
	mAllGObjs.~shared_ptr();	//Delete the pointer to the list of all game objects
	mRI.~shared_ptr();			//Delete the pointer to this render item
}

void Player::Update(const float dTime) {
	if (!GetActive()) return;
	if (mApplyGravity)
	{
		//Create a bounding box in the next location in the Y axis (X and Z are handled within the Walk and Strafe functions)
		BoundingBox nextBox;
		mBoundingBox.Transform(nextBox, DirectX::XMMatrixTranslation(0, mVel.y * dTime, 0));
		if (CheckIfCollidingAtBox(nextBox)) {
			mVel.y = 0.0f;
			mJumped = false;
		}
		else {
			AddVelocity(0, dTime * (GameData::sGrav * 4), 0);
		}

		if (mVel.y != 0) {
			Translate(dTime, 0, mVel.y, 0);
			TranslateCamera(dTime, 0, mVel.y, 0);
			SetDirtyFlag();
		}
	}
}
void Player::TranslateCamera(float dTime, float x, float y, float z) {
	mCamera.Jump(dTime, x, y, z);
}
void Player::Jump() {
	if (!mJumped) {
		AddVelocity(0, 15.0f, 0);
		mJumped = true;
		SetDirtyFlag();
	}
}
void Player::Walk(float d, float dTime) {

	bool move = false;

	//add 20% extra distance to next box collision calculation to make sure player doesnt travel into box next move
	float addedCollisionOffset = (d * 2) * dTime;

	DirectX::XMMATRIX translateX = DirectX::XMMatrixTranslation(addedCollisionOffset, 0.2f, 0);
	BoundingBox nextBoxX;
	mBoundingBox.Transform(nextBoxX, translateX);

	DirectX::FXMMATRIX translateZ = DirectX::XMMatrixTranslation(0, 0.2f, addedCollisionOffset);
	BoundingBox nextBoxZ;
	mBoundingBox.Transform(nextBoxZ, translateZ);

	//if player is not colliding
	if (!(CheckIfCollidingAtBox(nextBoxX))) {
		mCamera.Walk(d, dTime);

		DirectX::XMMATRIX oldWorldMatrix;
		GameData::StoreFloat4x4InMatrix(oldWorldMatrix, mRI->World);
		DirectX::XMMATRIX cameraMatrix = DirectX::XMMatrixTranslation(mCamera.GetPosition3f().x, mCamera.GetPosition3f().y, mCamera.GetPosition3f().z);
		newWorldMatrix = oldWorldMatrix - cameraMatrix;
		newWorldMatrix.r[3].m128_f32[2] = oldWorldMatrix.r[3].m128_f32[2];
		//Translate(1, -newWorldMatrix.r[3].m128_f32[0], -newWorldMatrix.r[3].m128_f32[1] - mCameraOffsetY, oldWorldMatrix.r[3].m128_f32[2] + mCameraOffsetZ);

		SetDirtyFlag();
		move = true;
	}
	//if player is not colliding
	if (!(CheckIfCollidingAtBox(nextBoxZ))) {
		mCamera.Walk(d, dTime);

		DirectX::XMMATRIX oldWorldMatrix;
		GameData::StoreFloat4x4InMatrix(oldWorldMatrix, mRI->World);
		DirectX::XMMATRIX cameraMatrix = DirectX::XMMatrixTranslation(mCamera.GetPosition3f().x, mCamera.GetPosition3f().y, mCamera.GetPosition3f().z);
		newWorldMatrix = oldWorldMatrix - cameraMatrix;
		//Translate(1, -newWorldMatrix.r[3].m128_f32[0], -newWorldMatrix.r[3].m128_f32[1] - mCameraOffsetY, -newWorldMatrix.r[3].m128_f32[2] + mCameraOffsetZ);

		SetDirtyFlag();
		move = true;
	}

	if(move)
	Translate(1, -newWorldMatrix.r[3].m128_f32[0], -newWorldMatrix.r[3].m128_f32[1] - mCameraOffsetY, -newWorldMatrix.r[3].m128_f32[2] + mCameraOffsetZ);



	//else
	//{
	//	DirectX::XMMATRIX translateX = DirectX::XMMatrixTranslation(addedCollisionOffset, 0.2f, 0);
	//	BoundingBox nextBoxX;
	//	mBoundingBox.Transform(nextBoxX, translateX);

	//	DirectX::FXMMATRIX translateZ = DirectX::XMMatrixTranslation(0, 0.2f, addedCollisionOffset);
	//	BoundingBox nextBoxZ;
	//	mBoundingBox.Transform(nextBoxZ, translateZ);

	//}
}

//sideways movement
void Player::Strafe(float d, float dTime) {

	bool move = false;

	//add 20% extra distance to next box collision calculation to make sure player doesnt travel into box next move
	float addedCollisionOffset = (d * 2) * dTime;

	DirectX::XMMATRIX translateX = DirectX::XMMatrixTranslation(addedCollisionOffset, 0.2f, 0);
	BoundingBox nextBoxX;
	mBoundingBox.Transform(nextBoxX, translateX);

	DirectX::FXMMATRIX translateZ = DirectX::XMMatrixTranslation(0, 0.2f, addedCollisionOffset);
	BoundingBox nextBoxZ;
	mBoundingBox.Transform(nextBoxZ, translateZ);

	//if player is not colliding
	if (!(CheckIfCollidingAtBox(nextBoxX))) {
		mCamera.Strafe(d, dTime);

		DirectX::XMMATRIX oldWorldMatrix;
		GameData::StoreFloat4x4InMatrix(oldWorldMatrix, mRI->World);
		DirectX::XMMATRIX cameraMatrix = DirectX::XMMatrixTranslation(mCamera.GetPosition3f().x, mCamera.GetPosition3f().y, mCamera.GetPosition3f().z);
		newWorldMatrix = oldWorldMatrix - cameraMatrix;
		newWorldMatrix.r[3].m128_f32[2] = oldWorldMatrix.r[3].m128_f32[2];
		//Translate(1, -newWorldMatrix.r[3].m128_f32[0], -newWorldMatrix.r[3].m128_f32[1] - mCameraOffsetY, oldWorldMatrix.r[3].m128_f32[2] + mCameraOffsetZ);

		SetDirtyFlag();
		move = true;
	}
	//if player is not colliding
	if (!(CheckIfCollidingAtBox(nextBoxZ))) {
		mCamera.Strafe(d, dTime);

		DirectX::XMMATRIX oldWorldMatrix;
		GameData::StoreFloat4x4InMatrix(oldWorldMatrix, mRI->World);
		DirectX::XMMATRIX cameraMatrix = DirectX::XMMatrixTranslation(mCamera.GetPosition3f().x, mCamera.GetPosition3f().y, mCamera.GetPosition3f().z);
		newWorldMatrix = oldWorldMatrix - cameraMatrix;
		//Translate(1, -newWorldMatrix.r[3].m128_f32[0], -newWorldMatrix.r[3].m128_f32[1] - mCameraOffsetY, -newWorldMatrix.r[3].m128_f32[2] + mCameraOffsetZ);

		SetDirtyFlag();
		move = true;
	}

	if (move)
	Translate(1, -newWorldMatrix.r[3].m128_f32[0], -newWorldMatrix.r[3].m128_f32[1] - mCameraOffsetY, -newWorldMatrix.r[3].m128_f32[2] + mCameraOffsetZ);

	////if player is not going to collide
	//if (!(CheckIfCollidingAtBox(nextBoxX) || CheckIfCollidingAtBox(nextBoxZ))) {

	//	mCamera.Strafe(d, dTime);

	//	DirectX::XMMATRIX oldWorldMatrix;
	//	GameData::StoreFloat4x4InMatrix(oldWorldMatrix, mRI->World);
	//	DirectX::XMMATRIX cameraMatrix = DirectX::XMMatrixTranslation(mCamera.GetPosition3f().x, mCamera.GetPosition3f().y, mCamera.GetPosition3f().z);
	//	DirectX::XMMATRIX newWorldMatrix = oldWorldMatrix - cameraMatrix;
	//	//Translate(1, -newWorldMatrix.r[3].m128_f32[0], -newWorldMatrix.r[3].m128_f32[1] - offsetY, -newWorldMatrix.r[3].m128_f32[2] + offsetZ);
	//	Translate(1, -newWorldMatrix.r[3].m128_f32[0], -newWorldMatrix.r[3].m128_f32[1] - mCameraOffsetY, -newWorldMatrix.r[3].m128_f32[2] + mCameraOffsetZ);

	//	SetDirtyFlag();
	//}



	//else //if he is going to collide
	//{
	//	DirectX::XMMATRIX translateX = DirectX::XMMatrixTranslation(addedCollisionOffset, 0.2f, 0);
	//	BoundingBox nextBoxX;
	//	mBoundingBox.Transform(nextBoxX, translateX);

	//	DirectX::FXMMATRIX translateZ = DirectX::XMMatrixTranslation(0, 0.2f, addedCollisionOffset);
	//	BoundingBox nextBoxZ;
	//	mBoundingBox.Transform(nextBoxZ, translateZ);


	//	if (CheckIfCollidingAtBox(nextBoxX))
	//	{
	//		mCamera.SetPosition(mCamera.GetPosition3f().x - d * dTime, mCamera.GetPosition3f().y, mCamera.GetPosition3f().z);

	//		DirectX::XMMATRIX oldWorldMatrix;
	//		GameData::StoreFloat4x4InMatrix(oldWorldMatrix, mRI->World);
	//		DirectX::XMMATRIX cameraMatrix = DirectX::XMMatrixTranslation(mCamera.GetPosition3f().x, mCamera.GetPosition3f().y, mCamera.GetPosition3f().z);
	//		DirectX::XMMATRIX newWorldMatrix = oldWorldMatrix - cameraMatrix;
	//		//Translate(1, -newWorldMatrix.r[3].m128_f32[0], -newWorldMatrix.r[3].m128_f32[1] - offsetY, -newWorldMatrix.r[3].m128_f32[2] + offsetZ);
	//		Translate(1, -newWorldMatrix.r[3].m128_f32[0], -newWorldMatrix.r[3].m128_f32[1] - mCameraOffsetY, -newWorldMatrix.r[3].m128_f32[2] + mCameraOffsetZ);

	//		SetDirtyFlag();
	//	}
	//	if (CheckIfCollidingAtBox(nextBoxZ))
	//	{
	//		mCamera.SetPosition(mCamera.GetPosition3f().x, mCamera.GetPosition3f().y, mCamera.GetPosition3f().z - d * dTime);

	//		DirectX::XMMATRIX oldWorldMatrix;
	//		GameData::StoreFloat4x4InMatrix(oldWorldMatrix, mRI->World);
	//		DirectX::XMMATRIX cameraMatrix = DirectX::XMMatrixTranslation(mCamera.GetPosition3f().x, mCamera.GetPosition3f().y, mCamera.GetPosition3f().z);
	//		DirectX::XMMATRIX newWorldMatrix = oldWorldMatrix - cameraMatrix;
	//		//Translate(1, -newWorldMatrix.r[3].m128_f32[0], -newWorldMatrix.r[3].m128_f32[1] - offsetY, -newWorldMatrix.r[3].m128_f32[2] + offsetZ);
	//		Translate(1, -newWorldMatrix.r[3].m128_f32[0], -newWorldMatrix.r[3].m128_f32[1] - mCameraOffsetY, -newWorldMatrix.r[3].m128_f32[2] + mCameraOffsetZ);

	//		SetDirtyFlag();
	//	}
	//}

}
void Player::Pitch(float dy) {
	mCamera.Pitch(dy);

}
void Player::RotateY(float dx) {
	mCamera.RotateY(dx);
}
bool Player::MovementCollisionCheck(float d, float dTime) {

	//add 20% extra distance to next box collision calculation to make sure player doesnt travel into box next move
	float addedCollisionOffset = (d * 1.25) * dTime;

	DirectX::XMMATRIX translateX = DirectX::XMMatrixTranslation(addedCollisionOffset, 0.2f, 0);
	BoundingBox nextBoxX;
	mBoundingBox.Transform(nextBoxX, translateX);

	DirectX::FXMMATRIX translateZ = DirectX::XMMatrixTranslation(0, 0.2f, addedCollisionOffset);
	BoundingBox nextBoxZ;
	mBoundingBox.Transform(nextBoxZ, translateZ);

	return (!(CheckIfCollidingAtBox(nextBoxX) || CheckIfCollidingAtBox(nextBoxZ)));

}

//************************************************************************************************************
// Block
//************************************************************************************************************

Block::Block(std::shared_ptr<GameObject> gobj) : GameObject(gobj) {
	Init();
}

Block::~Block() {
	mAllGObjs.~shared_ptr();	//Delete the pointer to the list of all game objects
	mRI.~shared_ptr();			//Delete the pointer to this render item
}

void Block::Init() {
	mID = GameObject::mID;
}

void Block::createBlock(blockType newType)
{
	SetActive(true);
	type = newType;
	//SetTexture(type);
}

void Block::destroyBlock()
{
	SetActive(false);
	type = type_Default;
}