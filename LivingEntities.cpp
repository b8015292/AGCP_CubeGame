#include <algorithm>

#include "LivingEntities.h"
#include "Pathfinding.h"

LivingEntity::LivingEntity(std::shared_ptr<GameObject> gobj) : Entity(gobj) {

}
LivingEntity::~LivingEntity() {
	//mAllGObjs.~shared_ptr();	//Delete the pointer to the list of all game objects
	//mRI.~shared_ptr();			//Delete the pointer to this render item
}

void LivingEntity::Update(const float dTime) {
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
			AddVelocity(0, GameData::sGrav / 50.f, 0);
		}

		if (mVel.y != 0) {
			Translate(dTime, 0, mVel.y, 0);
			SetDirtyFlag();
		}
	}
	Walk(5.0f, dTime);
}
void LivingEntity::Jump() {
	if (!mJumped) {
		AddVelocity(0, 15.0f, 0);
		mJumped = true;
		SetDirtyFlag();
	}
}
void LivingEntity::Walk(float d, float dTime) {

	DirectX::XMMATRIX translateX = DirectX::XMMatrixTranslation(d * dTime, 0.2f, 0);
	BoundingBox nextBoxX;
	mBoundingBox.Transform(nextBoxX, translateX);

	DirectX::FXMMATRIX translateZ = DirectX::XMMatrixTranslation(0, 0.2f, d * dTime);
	BoundingBox nextBoxZ;
	mBoundingBox.Transform(nextBoxZ, translateZ);

	if (!(CheckIfCollidingAtBox(nextBoxX) && CheckIfCollidingAtBox(nextBoxZ))) {

		d = d * dTime;
		XMVECTOR s = XMVectorReplicate(d);
		XMVECTOR l = { direction.m128_f32[0], 0.0f, direction.m128_f32[2] }; //dont move up along the y
		XMVECTOR p = XMLoadFloat3(&mBoundingBox.Center);
		XMFLOAT3 newpos;
		XMStoreFloat3(&newpos, XMVectorMultiplyAdd(s, l, p));

		DirectX::XMMATRIX oldWorldMatrix;
		GameData::StoreFloat4x4InMatrix(oldWorldMatrix, mRI->World);
		DirectX::XMMATRIX cameraMatrix = DirectX::XMMatrixTranslation(mBoundingBox.Center.x, mBoundingBox.Center.y, mBoundingBox.Center.z);
		DirectX::XMMATRIX newWorldMatrix = oldWorldMatrix - cameraMatrix;
		Translate(1, -newWorldMatrix.r[3].m128_f32[0], -newWorldMatrix.r[3].m128_f32[1], -newWorldMatrix.r[3].m128_f32[2]);

		SetDirtyFlag();
	}
}
void LivingEntity::Strafe(float d, float dTime) {

	DirectX::FXMMATRIX translate = DirectX::XMMatrixTranslation(d * dTime, 0.2f, 0);
	BoundingBox nextBoxX;
	mBoundingBox.Transform(nextBoxX, translate);

	if (!CheckIfCollidingAtBox(nextBoxX)) {
		DirectX::XMMATRIX oldWorldMatrix;
		GameData::StoreFloat4x4InMatrix(oldWorldMatrix, mRI->World);
		DirectX::XMMATRIX cameraMatrix = DirectX::XMMatrixTranslation(mBoundingBox.Center.x, mBoundingBox.Center.y, mBoundingBox.Center.z);
		DirectX::XMMATRIX newWorldMatrix = oldWorldMatrix - cameraMatrix;
		Translate(1, -newWorldMatrix.r[3].m128_f32[0], -newWorldMatrix.r[3].m128_f32[1], -newWorldMatrix.r[3].m128_f32[2]);

		SetDirtyFlag();
	}
}

void LivingEntity::WalkToBlock(XMFLOAT3 blockLocation) {
	Node player;
	player.x = mBoundingBox.Center.x;
	player.y = mBoundingBox.Center.y;
	player.z = mBoundingBox.Center.z;

	Node destination;
	destination.x = blockLocation.x;
	destination.y = blockLocation.y;
	destination.z = blockLocation.z;

	Node node = Pathfinding::aStar(player, destination).at(0);
	walkTo = XMFLOAT3{ (float)node.x, (float)node.y, (float)node.z };
	direction = XMVECTOR{ walkTo.x - mBoundingBox.Center.x, walkTo.y - mBoundingBox.Center.y, walkTo.z - mBoundingBox.Center.z };

}
