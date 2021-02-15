#include "Player.h"

Player::Player(std::shared_ptr<GameObject> gobj) : Entity(gobj), mPlayerHealth(100), mPlayerDamage(1) {

}

void Player::Update(const float dTime) {
	if (!GetActive()) return;
	if (mApplyGravity)
	{
		//Create a bounding box in the next location in the Y axis to check ceiling collision
		BoundingBox positiveOffset;
		BoundingBox negativeOffset;
		//Create an added offset to each side of the cube so player can jump while against an edge
		mBoundingBox.Transform(negativeOffset, DirectX::XMMatrixTranslation(-0.2f, mVel.y * dTime, -0.2f));
		mBoundingBox.Transform(positiveOffset, DirectX::XMMatrixTranslation(0.2f, mVel.y * dTime, 0.2f));

		if (!(CheckIfCollidingAtBox(negativeOffset) && CheckIfCollidingAtBox(positiveOffset))) {

			AddVelocity(0, dTime * (GameData::sGrav * 4), 0);
		}
		else {
			mVel.y = 0.0f;
			mJumped = false;
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

	//add 10% extra distance to next box collision calculation to make sure player doesnt travel into box next move
	float addedCollisionOffset = (d * 1.1f) * dTime;

	DirectX::XMMATRIX translateX = DirectX::XMMatrixTranslation(addedCollisionOffset, 0.2f, 0);
	BoundingBox nextBoxX;
	mBoundingBox.Transform(nextBoxX, translateX);

	DirectX::FXMMATRIX translateZ = DirectX::XMMatrixTranslation(0, 0.2f, addedCollisionOffset);
	BoundingBox nextBoxZ;
	mBoundingBox.Transform(nextBoxZ, translateZ);

	if (!(CheckIfCollidingAtBox(nextBoxX) && CheckIfCollidingAtBox(nextBoxZ))) {

		if (mDiagonal) d = d / 2;

		mCamera.Walk(3, d, dTime);

		DirectX::XMMATRIX oldWorldMatrix;
		GameData::StoreFloat4x4InMatrix(oldWorldMatrix, mRI->World);
		DirectX::XMMATRIX cameraMatrix = DirectX::XMMatrixTranslation(mCamera.GetPosition3f().x, mCamera.GetPosition3f().y, mCamera.GetPosition3f().z);
		DirectX::XMMATRIX newWorldMatrix = oldWorldMatrix - cameraMatrix;
		//if(newWorldMatrix.r[3].m128_f32[2] < mCamera.GetPosition3f().z)
		//Translate(1, -newWorldMatrix.r[3].m128_f32[0], -newWorldMatrix.r[3].m128_f32[1] - offsetY, -newWorldMatrix.r[3].m128_f32[2] + offsetZ);
		Translate(1, -newWorldMatrix.r[3].m128_f32[0], -newWorldMatrix.r[3].m128_f32[1] - mCameraOffsetY, -newWorldMatrix.r[3].m128_f32[2] + mCameraOffsetZ);

		SetDirtyFlag();
	}
	else if (CheckIfCollidingAtBox(nextBoxX) && !CheckIfCollidingAtBox(nextBoxZ)) {
		
		//if colliding on x

		if (mDiagonal) d = d / 2;

		mCamera.Walk(1, d, dTime);

		DirectX::XMMATRIX oldWorldMatrix;
		GameData::StoreFloat4x4InMatrix(oldWorldMatrix, mRI->World);
		DirectX::XMMATRIX cameraMatrix = DirectX::XMMatrixTranslation(mCamera.GetPosition3f().x, mCamera.GetPosition3f().y, mCamera.GetPosition3f().z);
		DirectX::XMMATRIX newWorldMatrix = oldWorldMatrix - cameraMatrix;
		//if(newWorldMatrix.r[3].m128_f32[2] < mCamera.GetPosition3f().z)
		//Translate(1, -newWorldMatrix.r[3].m128_f32[0], -newWorldMatrix.r[3].m128_f32[1] - offsetY, -newWorldMatrix.r[3].m128_f32[2] + offsetZ);
		Translate(1, -newWorldMatrix.r[3].m128_f32[0], -newWorldMatrix.r[3].m128_f32[1] - mCameraOffsetY, -newWorldMatrix.r[3].m128_f32[2] + mCameraOffsetZ);

		SetDirtyFlag();
	}
	else if (CheckIfCollidingAtBox(nextBoxZ) && !CheckIfCollidingAtBox(nextBoxX)) {
		
		//if colliding on z

		if (mDiagonal) d = d / 2;

		mCamera.Walk(2, d, dTime);

		DirectX::XMMATRIX oldWorldMatrix;
		GameData::StoreFloat4x4InMatrix(oldWorldMatrix, mRI->World);
		DirectX::XMMATRIX cameraMatrix = DirectX::XMMatrixTranslation(mCamera.GetPosition3f().x, mCamera.GetPosition3f().y, mCamera.GetPosition3f().z);
		DirectX::XMMATRIX newWorldMatrix = oldWorldMatrix - cameraMatrix;
		//if(newWorldMatrix.r[3].m128_f32[2] < mCamera.GetPosition3f().z)
		//Translate(1, -newWorldMatrix.r[3].m128_f32[0], -newWorldMatrix.r[3].m128_f32[1] - offsetY, -newWorldMatrix.r[3].m128_f32[2] + offsetZ);
		Translate(1, -newWorldMatrix.r[3].m128_f32[0], -newWorldMatrix.r[3].m128_f32[1] - mCameraOffsetY, -newWorldMatrix.r[3].m128_f32[2] + mCameraOffsetZ);

		SetDirtyFlag();
	}





	//bool move = false;

	////add 20% extra distance to next box collision calculation to make sure player doesnt travel into box next move
	//float addedCollisionOffset = (d * 2) * dTime;

	//DirectX::XMMATRIX translateX = DirectX::XMMatrixTranslation(addedCollisionOffset, 0.2f, 0);
	//BoundingBox nextBoxX;
	//mBoundingBox.Transform(nextBoxX, translateX);

	//DirectX::FXMMATRIX translateZ = DirectX::XMMatrixTranslation(0, 0.2f, addedCollisionOffset);
	//BoundingBox nextBoxZ;
	//mBoundingBox.Transform(nextBoxZ, translateZ);

	//if player is not colliding
	//if (!(CheckIfCollidingAtBox(nextBoxX))) {
	//	mCamera.Walk(d, dTime);

	//	DirectX::XMMATRIX oldWorldMatrix;
	//	GameData::StoreFloat4x4InMatrix(oldWorldMatrix, mRI->World);
	//	DirectX::XMMATRIX cameraMatrix = DirectX::XMMatrixTranslation(mCamera.GetPosition3f().x, mCamera.GetPosition3f().y, mCamera.GetPosition3f().z);
	//	mNewWorldMatrix = oldWorldMatrix - cameraMatrix;
	//	mNewWorldMatrix.r[3].m128_f32[2] = oldWorldMatrix.r[3].m128_f32[2];
	//	//Translate(1, -newWorldMatrix.r[3].m128_f32[0], -newWorldMatrix.r[3].m128_f32[1] - mCameraOffsetY, oldWorldMatrix.r[3].m128_f32[2] + mCameraOffsetZ);

	//	SetDirtyFlag();
	//	move = true;
	//}
	////if player is not colliding
	//if (!(CheckIfCollidingAtBox(nextBoxZ))) {  
	//	mCamera.Walk(d, dTime);

	//	DirectX::XMMATRIX oldWorldMatrix;
	//	GameData::StoreFloat4x4InMatrix(oldWorldMatrix, mRI->World);
	//	DirectX::XMMATRIX cameraMatrix = DirectX::XMMatrixTranslation(mCamera.GetPosition3f().x, mCamera.GetPosition3f().y, mCamera.GetPosition3f().z);
	//	mNewWorldMatrix = oldWorldMatrix - cameraMatrix;
	//	//Translate(1, -newWorldMatrix.r[3].m128_f32[0], -newWorldMatrix.r[3].m128_f32[1] - mCameraOffsetY, -newWorldMatrix.r[3].m128_f32[2] + mCameraOffsetZ);

	//	SetDirtyFlag();
	//	move = true;
	//}

	//if (move)
	//{
	//	mCamera.Walk(d, dTime);
	//	Translate(1, -mNewWorldMatrix.r[3].m128_f32[0], -mNewWorldMatrix.r[3].m128_f32[1] - mCameraOffsetY, -mNewWorldMatrix.r[3].m128_f32[2] + mCameraOffsetZ);
	//}


}
void Player::Strafe(float d, float dTime) {

	DirectX::FXMMATRIX translate = DirectX::XMMatrixTranslation(d * dTime, 0.2f, 0);
	BoundingBox nextBoxX;
	mBoundingBox.Transform(nextBoxX, translate);

	if (!CheckIfCollidingAtBox(nextBoxX)) {

		if (mDiagonal) d = d / 2;

		mCamera.Strafe(d, dTime);


		DirectX::XMMATRIX oldWorldMatrix;
		GameData::StoreFloat4x4InMatrix(oldWorldMatrix, mRI->World);
		DirectX::XMMATRIX cameraMatrix = DirectX::XMMatrixTranslation(mCamera.GetPosition3f().x, mCamera.GetPosition3f().y, mCamera.GetPosition3f().z);
		DirectX::XMMATRIX newWorldMatrix = oldWorldMatrix - cameraMatrix;
		//Translate(1, -newWorldMatrix.r[3].m128_f32[0], -newWorldMatrix.r[3].m128_f32[1] - offsetY, -newWorldMatrix.r[3].m128_f32[2] + offsetZ);
		Translate(1, -newWorldMatrix.r[3].m128_f32[0], -newWorldMatrix.r[3].m128_f32[1] - mCameraOffsetY, -newWorldMatrix.r[3].m128_f32[2] + mCameraOffsetZ);

		SetDirtyFlag();
	}

	//bool move = false;

	////add 20% extra distance to next box collision calculation to make sure player doesnt travel into box next move
	//float addedCollisionOffset = (d * 2) * dTime;

	//DirectX::XMMATRIX translateX = DirectX::XMMatrixTranslation(addedCollisionOffset, 0.2f, 0);
	//BoundingBox nextBoxX;
	//mBoundingBox.Transform(nextBoxX, translateX);

	//DirectX::FXMMATRIX translateZ = DirectX::XMMatrixTranslation(0, 0.2f, addedCollisionOffset);
	//BoundingBox nextBoxZ;
	//mBoundingBox.Transform(nextBoxZ, translateZ);

	////if player is not colliding
	//if (!(CheckIfCollidingAtBox(nextBoxX))) {
	//	mCamera.Strafe(d, dTime);

	//	DirectX::XMMATRIX oldWorldMatrix;
	//	GameData::StoreFloat4x4InMatrix(oldWorldMatrix, mRI->World);
	//	DirectX::XMMATRIX cameraMatrix = DirectX::XMMatrixTranslation(mCamera.GetPosition3f().x, mCamera.GetPosition3f().y, mCamera.GetPosition3f().z);
	//	newWorldMatrix = oldWorldMatrix - cameraMatrix;
	//	newWorldMatrix.r[3].m128_f32[2] = oldWorldMatrix.r[3].m128_f32[2];
	//	//Translate(1, -newWorldMatrix.r[3].m128_f32[0], -newWorldMatrix.r[3].m128_f32[1] - mCameraOffsetY, oldWorldMatrix.r[3].m128_f32[2] + mCameraOffsetZ);

	//	SetDirtyFlag();
	//	move = true;
	//}
	////if player is not colliding
	//if (!(CheckIfCollidingAtBox(nextBoxZ))) {
	//	mCamera.Strafe(d, dTime);

	//	DirectX::XMMATRIX oldWorldMatrix;
	//	GameData::StoreFloat4x4InMatrix(oldWorldMatrix, mRI->World);
	//	DirectX::XMMATRIX cameraMatrix = DirectX::XMMatrixTranslation(mCamera.GetPosition3f().x, mCamera.GetPosition3f().y, mCamera.GetPosition3f().z);
	//	newWorldMatrix = oldWorldMatrix - cameraMatrix;
	//	//Translate(1, -newWorldMatrix.r[3].m128_f32[0], -newWorldMatrix.r[3].m128_f32[1] - mCameraOffsetY, -newWorldMatrix.r[3].m128_f32[2] + mCameraOffsetZ);

	//	SetDirtyFlag();
	//	move = true;
	//}

	//if (move)
	//	Translate(1, -newWorldMatrix.r[3].m128_f32[0], -newWorldMatrix.r[3].m128_f32[1] - mCameraOffsetY, -newWorldMatrix.r[3].m128_f32[2] + mCameraOffsetZ);
}
void Player::Pitch(float dy) {
	mCamera.Pitch(dy);

}
void Player::RotateY(float dx) {
	mCamera.RotateY(dx);
}
bool Player::MovementCollisionCheck(float d, float dTime) {

	//add 20% extra distance to next box collision calculation to make sure player doesnt travel into box next move
	float addedCollisionOffset = (d * 1.25f) * dTime;

	DirectX::XMMATRIX translateX = DirectX::XMMatrixTranslation(addedCollisionOffset, 0.2f, 0);
	BoundingBox nextBoxX;
	mBoundingBox.Transform(nextBoxX, translateX);

	DirectX::FXMMATRIX translateZ = DirectX::XMMatrixTranslation(0, 0.2f, addedCollisionOffset);
	BoundingBox nextBoxZ;
	mBoundingBox.Transform(nextBoxZ, translateZ);

	return (!(CheckIfCollidingAtBox(nextBoxX) || CheckIfCollidingAtBox(nextBoxZ)));
}

void Player::increasePlayerHealth(int amount)
{
	if (amount >= (mMaxHealth - mPlayerHealth)) mPlayerHealth = mMaxHealth;
	else mPlayerHealth += amount;
}

void Player::decreasePlayerHealth(int amount)
{
	if (amount > mPlayerHealth) mPlayerHealth = 0;
	else mPlayerHealth -= amount;
}