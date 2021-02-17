#include "Player.h"

Player::Player(std::shared_ptr<GameObject> gobj) : Entity(gobj), mPlayerHealth(100), mPlayerDamage(1) {

}

void Player::SetRIDirty() {
	mRI->NumFramesDirty++; 
	mDirty = false;
	mCamera.UpdateViewMatrix();
}

void Player::Update(const float dTime) {
	if (!GetActive()) return;

	//Apply gravity
	if (mApplyGravity)
	{
		//Create a bounding box in the next location in the Y axis to check ceiling collision
		BoundingBox positiveOffset;
		BoundingBox negativeOffset;
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

	//Walk around
	if (mMoveX || mMoveZ) {
		if (mMoveX && mMoveZ) {
			mDiagonal = true;
		}

		//Walk and strafe. If either were unsucessful, try walk along the X or Z axis (slide along the side of blocks in looking at them)
		if (!Walk(mMoveZ * mMoveSpeed, dTime)
			|| !Strafe(mMoveX * mMoveSpeed, dTime))
			MoveInRawDirection(mMoveX, mMoveZ, dTime);

		SetDirtyFlag();
	}

	if (mSetJump) {
		Jump();
		SetDirtyFlag();
	}

	//Set camera look
	if (mPitch)
		Pitch(mPitch);
	if (mYaw)
		RotateY(mYaw);

	//Reset movement variables
	mDiagonal = false;
	mSetJump = false;
	mMoveX = 0.f;
	mMoveZ = 0.f;
	mPitch = 0.f;
	mYaw = 0.f;

	SetRIDirty();
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

void Player::SetMovement(float x, float z, bool jumping) {
	mMoveX = x;
	mMoveZ = z;
	mSetJump = jumping;
}

void Player::SetRotation(float x, float y) {
	mPitch = y;
	mYaw = x;
}

bool Player::Walk(float d, float dTime) {

	//If moving diagonally, half the speed
	if (mDiagonal) d = d / 1.5f;

	//Get the movement direction & distance
	XMVECTOR look = mCamera.GetLook() * (d * dTime);

	//Create a bounding box in the position that the player is about to walk to. Increasing the Y slightly so it doesn't clip the floor
	BoundingBox nextBoxX;
	mBoundingBox.Transform(nextBoxX, DirectX::XMMatrixTranslation(look.m128_f32[0], 0.2f, look.m128_f32[2]));

	//If there is no collision, move the player and camera
	if (!CheckIfCollidingAtBox(nextBoxX)) {

		//Move the camera
		mCamera.Walk(d, dTime);

		//Get the previous position
		DirectX::XMMATRIX oldWorldMatrix;
		GameData::StoreFloat4x4InMatrix(oldWorldMatrix, mRI->World);

		//Get the camera position
		DirectX::XMMATRIX cameraMatrix = DirectX::XMMatrixTranslation(mCamera.GetPosition3f().x, mCamera.GetPosition3f().y, mCamera.GetPosition3f().z);

		//Get the difference between the players old position and the cameras new position
		DirectX::XMMATRIX newWorldMatrix = oldWorldMatrix - cameraMatrix;

		//Translate the player by the difference, adding the offsets
		Translate(1, -newWorldMatrix.r[3].m128_f32[0], -newWorldMatrix.r[3].m128_f32[1] - mCameraOffsetY, -newWorldMatrix.r[3].m128_f32[2] + mCameraOffsetZ);

		SetDirtyFlag();
		return true;
	}

	return false;
}

bool Player::Strafe(float d, float dTime) {

	//If moving diaganoly, half the speed
	if (mDiagonal) d = d / 1.5f;

	//Get a bounding box in the next position
	BoundingBox nextBoxX;
	mBoundingBox.Transform(nextBoxX, DirectX::XMMatrixTranslation(d * dTime, 0.2f, 0));

	//If it's not colliding move
	if (!CheckIfCollidingAtBox(nextBoxX)) {

		//Move the camera
		mCamera.Strafe(d, dTime);

		//Calculate the amount to move the player
		DirectX::XMMATRIX oldWorldMatrix;
		GameData::StoreFloat4x4InMatrix(oldWorldMatrix, mRI->World);
		DirectX::XMMATRIX cameraMatrix = DirectX::XMMatrixTranslation(mCamera.GetPosition3f().x, mCamera.GetPosition3f().y, mCamera.GetPosition3f().z);
		DirectX::XMMATRIX newWorldMatrix = oldWorldMatrix - cameraMatrix;

		//Move the player
		Translate(1, -newWorldMatrix.r[3].m128_f32[0], -newWorldMatrix.r[3].m128_f32[1] - mCameraOffsetY, -newWorldMatrix.r[3].m128_f32[2] + mCameraOffsetZ);

		SetDirtyFlag();
		return true;
	}
	return false;
}

void Player::MoveInRawDirection(float x, float z, float dTime) {
	if (mDiagonal) {
		x /= 1.5f;
		z /= 1.5f;
	}
	//Get a bounding box in the next position
	BoundingBox nextBoxX;
	mBoundingBox.Transform(nextBoxX, DirectX::XMMatrixTranslation(x * dTime, 0.2f, 0.f));
	BoundingBox nextBoxZ;
	mBoundingBox.Transform(nextBoxZ, DirectX::XMMatrixTranslation(0.f, 0.2f, z * dTime));

	if (!CheckIfCollidingAtBox(nextBoxX)) {
		z = x;
		x = 0.f;
	}
	if (!CheckIfCollidingAtBox(nextBoxZ)) {
		x = z;
		z = 0.f;
	}

	x *= mMoveSpeed;
	z *= mMoveSpeed;

	//If it's not colliding move
	if (z != 0 || x != 0) {

		//Move the camera
		mCamera.Strafe(x, dTime);
		mCamera.Walk(z, dTime);

		//Calculate the amount to move the player
		DirectX::XMMATRIX oldWorldMatrix;
		GameData::StoreFloat4x4InMatrix(oldWorldMatrix, mRI->World);
		DirectX::XMMATRIX cameraMatrix = DirectX::XMMatrixTranslation(mCamera.GetPosition3f().x, mCamera.GetPosition3f().y, mCamera.GetPosition3f().z);
		DirectX::XMMATRIX newWorldMatrix = oldWorldMatrix - cameraMatrix;

		//Move the player
		Translate(1, -newWorldMatrix.r[3].m128_f32[0], -newWorldMatrix.r[3].m128_f32[1] - mCameraOffsetY, -newWorldMatrix.r[3].m128_f32[2] + mCameraOffsetZ);

		SetDirtyFlag();
	}
}

void Player::Pitch(float dy) {
	mCamera.Pitch(dy);

}
void Player::RotateY(float dx) {
	mCamera.RotateY(dx);
}

void Player::SetPosition(XMFLOAT3 newPos) {
	mCamera.SetPosition(newPos);
	Walk(0, 0);
}

//bool Player::MovementCollisionCheck(float d, float dTime) {
//
//	//add 20% extra distance to next box collision calculation to make sure player doesnt travel into box next move
//	float addedCollisionOffset = (d * 1.25f) * dTime;
//
//	DirectX::XMMATRIX translateX = DirectX::XMMatrixTranslation(addedCollisionOffset, 0.2f, 0);
//	BoundingBox nextBoxX;
//	mBoundingBox.Transform(nextBoxX, translateX);
//
//	DirectX::FXMMATRIX translateZ = DirectX::XMMatrixTranslation(0, 0.2f, addedCollisionOffset);
//	BoundingBox nextBoxZ;
//	mBoundingBox.Transform(nextBoxZ, translateZ);
//
//	return (!(CheckIfCollidingAtBox(nextBoxX) || CheckIfCollidingAtBox(nextBoxZ)));
//}

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