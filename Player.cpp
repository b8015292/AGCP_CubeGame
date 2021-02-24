#include "Player.h"

Player::Player(std::shared_ptr<GameObject> gobj) : Entity(gobj), mPlayerHealth(100), mPlayerDamage(1) {
	mRotate90 = XMMatrixRotationY(XMConvertToRadians(90.f));
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
			mCamera.Translate(0, mVel.y * dTime, 0);
			SetDirtyFlag();
		}
	}

	//Walk around
	if (mStrafing || mWalking) {
		if (mStrafing && mWalking) {
			mDiagonal = true;
			//mStrafing *= 0.75f;
			//mWalking *= 0.75f;
		}

		CheckCollisions(dTime);
		if ((mWalking && mCanMove[Dir::look]) || (mStrafing && mCanMove[Dir::lookStrafe]) || (mWalking && mStrafing && mCanMove[Dir::lookBoth])) {
			MoveInLook(dTime);
		}
		else if (mCanMove[Dir::axisForward] || mCanMove[Dir::axisSide]) {
			MoveInAxis(dTime);
		}

		SetDirtyFlag();
	}

	if (mSetJump) {
		Jump();
		SetDirtyFlag();
	}

	//Set camera look
	if (mPitch) {
		Pitch(mPitch);
		SetDirtyFlag();
	}
	if (mYaw) {
		RotateY(mYaw);
		SetDirtyFlag();
	}

	//Reset movement variables
	mDiagonal = false;
	mSetJump = false;
	mStrafing = 0.f;
	mWalking = 0.f;
	mPitch = 0.f;
	mYaw = 0.f;

	if(mDirty) SetRIDirty();
}

void Player::Jump() {
	if (!mJumped) {
		AddVelocity(0, 15.0f, 0);
		mJumped = true;
		SetDirtyFlag();
	}
}

void Player::SetMovement(float x, float z, bool jumping) {
	mStrafing = x;
	mWalking = z;
	mSetJump = jumping;
}

void Player::SetRotation(float x, float y) {
	mPitch = y;
	mYaw = x;
}


void Player::CheckCollisions(float dTime) {
	float deltaMove = mMoveSpeed * dTime;
	XMVECTOR camLook = mCamera.GetLook();

	//a for Axis
	XMVECTOR aForward;
	XMVECTOR aSide;
	//XMVECTOR aStrafe;

	//Get the look axis aligned
	//Looking primerally along Z
	if (abs(camLook.m128_f32[2]) >= abs(camLook.m128_f32[0])) {
		if (camLook.m128_f32[2] > 0) {
			aForward = mAxisVectors[Axis::posZ];
			if (camLook.m128_f32[0] > 0) {
				aSide = mAxisVectors[Axis::posX];
			}
			else {
				aSide = mAxisVectors[Axis::negX];
			}
		}
		else {
			aForward = mAxisVectors[Axis::negZ];	
			if (camLook.m128_f32[0] > 0) {
				aSide = mAxisVectors[Axis::posX];
			}
			else {
				aSide = mAxisVectors[Axis::negX];
			}
		}

		//if (aSide.m128_f32[0] == mStrafing) {
		//	aStrafe = aSide;
		//}
		//else {
		//	aStrafe = -aSide;
		//}
	}
	else {	//Otherwise X is the primary look axis
		if (camLook.m128_f32[0] > 0) { 
			aForward = mAxisVectors[Axis::posX];
			if (camLook.m128_f32[2] > 0) {
				aSide = mAxisVectors[Axis::posZ];
			}
			else {
				aSide = mAxisVectors[Axis::negZ];
			}
		}
		else {
			aForward = mAxisVectors[Axis::negX];		
			if (camLook.m128_f32[2] > 0) {
				aSide = mAxisVectors[Axis::posZ];
			}
			else {
				aSide = mAxisVectors[Axis::negZ];
			}
		}

		//if (aSide.m128_f32[2] == mStrafing) {
		//	aStrafe = aSide;
		//}
		//else {
		//	aStrafe = -aSide;
		//}
	}

	//FOR LOOPS UNROLLED
	//Calculate the possible next positions
	mMoveVectors[Dir::look] = camLook * deltaMove * mWalking;
	mMoveVectors[Dir::lookStrafe] = mCamera.GetRight() * deltaMove * mStrafing;
	mMoveVectors[Dir::lookBoth] = ((camLook * mWalking) + (mCamera.GetRight() * mStrafing)) * deltaMove;
	mMoveVectors[Dir::axisForward] = aForward * deltaMove;
	mMoveVectors[Dir::axisSide] = aSide * deltaMove;
	//mMoveVectors[Dir::axisStrafe] = -aStrafe * deltaMove;

	//Apply the next positions to bounding boxes
	float yOffset = 0.2f;	//Used to ensure the player doesn't clip the floor
	mBoundingBox.Transform(mNextBoxes[Dir::look], DirectX::XMMatrixTranslation(mMoveVectors[Dir::look].m128_f32[0], yOffset, mMoveVectors[Dir::look].m128_f32[2]));
	mBoundingBox.Transform(mNextBoxes[Dir::lookStrafe], DirectX::XMMatrixTranslation(mMoveVectors[Dir::lookStrafe].m128_f32[0], yOffset, mMoveVectors[Dir::lookStrafe].m128_f32[2]));
	mBoundingBox.Transform(mNextBoxes[Dir::lookBoth], DirectX::XMMatrixTranslation(mMoveVectors[Dir::lookBoth].m128_f32[0], yOffset, mMoveVectors[Dir::lookBoth].m128_f32[2]));
	mBoundingBox.Transform(mNextBoxes[Dir::axisForward], DirectX::XMMatrixTranslation(mMoveVectors[Dir::axisForward].m128_f32[0], yOffset, mMoveVectors[Dir::axisForward].m128_f32[2]));
	mBoundingBox.Transform(mNextBoxes[Dir::axisSide], DirectX::XMMatrixTranslation(mMoveVectors[Dir::axisSide].m128_f32[0], yOffset, mMoveVectors[Dir::axisSide].m128_f32[2]));
	//mBoundingBox.Transform(mNextBoxes[Dir::axisStrafe], DirectX::XMMatrixTranslation(mMoveVectors[Dir::axisStrafe].m128_f32[0], yOffset, mMoveVectors[Dir::axisStrafe].m128_f32[2]));

	//Rest the value
	mCanMove[Dir::look] = true;
	mCanMove[Dir::lookStrafe] = true;
	mCanMove[Dir::lookBoth] = true;
	mCanMove[Dir::axisForward] = true;
	mCanMove[Dir::axisSide] = true;
	//mCanMove[Dir::axisStrafe] = true;

	int collisions = 0;
	for (int i = 0; i < sAllGObjs->size(); i++) {
		//If the IDs arent the same, the block is active, and it is colliding
		std::shared_ptr<GameObject> go = sAllGObjs->at(i);
		if (mID != go->GetID()) {
			if (go->GetActive()) {

				//Checks the current GObjs against all possible future boxes
				for (int j = 0; j < Dir::count; j++) {

					//Dont double check the same box
					if (mCanMove[j] == true) {

						//Check if the box collides with the current GObj
						if (mNextBoxes[j].Contains(go->GetBoundingBox()) != DirectX::ContainmentType::DISJOINT) {
							mCanMove[j] = false;
							collisions++;
						}
					}


				}
			}
		}

		//If no direction can be moved in, escape the function
		if (collisions == Dir::count)
			break;
	}
}

void Player::MoveInLook(float dTime) {
	//Calculate the walk (forward) and strafe (side) then add them together
	XMVECTOR translate = { 0.f, 0.f, 0.f, 0.f };

	//If you can move both directions, do it
	if (mCanMove[Dir::lookBoth] && mDiagonal) {
		translate = mMoveVectors[Dir::lookBoth];
	}
	else {
		//Otherwise, move either forward or to the side
		if (mCanMove[Dir::look] && mWalking) translate = mMoveVectors[Dir::look];
		else translate = mMoveVectors[Dir::lookStrafe];
	}

	//Move the camera and the player
	Translate(1, translate.m128_f32[0], 0.f, translate.m128_f32[2]);
	mCamera.Translate(translate.m128_f32[0], 0.f, translate.m128_f32[2]);

	SetDirtyFlag();
}

void Player::MoveInAxis(float dTime) {
	XMVECTOR translate = { 0.f, 0.f, 0.f, 0.f };

	if (mWalking) {
		if (mCanMove[Dir::axisForward]) {
			translate = mMoveVectors[Dir::axisForward];
		}
		else if (mCanMove[Dir::axisSide]) {
			translate = mMoveVectors[Dir::axisSide];
		}
	}
	//else if (mStrafing) {
	//	if (mCanMove[Dir::axisStrafe]) {
	//		translate = mMoveVectors[Dir::axisStrafe];
	//	}
	//}

	//Move the camera and the player
	Translate(1, translate.m128_f32[0], 0.f, translate.m128_f32[2]);
	mCamera.Translate(translate.m128_f32[0], 0.f, translate.m128_f32[2]);

	SetDirtyFlag();
}

bool Player::Walk(float d, float dTime) {

	//If moving diagonally, half the speed
	if (mDiagonal) d = d / 1.5f;

	//Get the movement direction & distance
	XMVECTOR look = mCamera.GetLook() * (d * dTime);

	//Create a bounding box in the position that the player is about to walk to. Increasing the Y slightly so it doesn't clip the floor
	XMMATRIX translation = DirectX::XMMatrixTranslation(look.m128_f32[0], 0.2f, look.m128_f32[2]);
	BoundingBox nextBoxX;
	mBoundingBox.Transform(nextBoxX, translation);

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

void Player::Pitch(float dy) {
	mCamera.Pitch(dy);

}
void Player::RotateY(float dx) {
	mCamera.RotateY(dx);
}

void Player::SetPosition(XMFLOAT3 newPos) {
	mCamera.SetPosition(newPos);
	mCamera.UpdateViewMatrix();

	mBoundingBox.Center = newPos;
	mBoundingBox.Center.y -= mCameraOffsetY;
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