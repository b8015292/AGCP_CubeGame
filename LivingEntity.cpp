#include "LivingEntity.h"

LivingEntity::LivingEntity(std::shared_ptr<GameObject> gobj, std::shared_ptr<Player> player, std::shared_ptr<WorldManager> worldManager) : Entity(gobj) {
	mPlayer = player;
	mWorldManager = worldManager;
	WalkToBlock(mPlayer->GetBoundingBox().Center);
}
LivingEntity::~LivingEntity() {
	//mAllGObjs.~shared_ptr();	//Delete the pointer to the list of all game objects
	//mRI.~shared_ptr();			//Delete the pointer to this render item
}

void LivingEntity::Update(const float dTime) {
	if (!GetActive()) return;
	if (debugging) return;
	if (mApplyGravity)
	{
		//Create a bounding box in the next location in the Y axis (X and Z are handled within the Walk and Strafe functions)
		BoundingBox nextBox;
		mRI->mBoundingBox.Transform(nextBox, DirectX::XMMatrixTranslation(0, mVel.y * dTime, 0));
		if (CheckIfCollidingAtBox(nextBox)) {
			if (mVel.y < 0) {
				mVel.y = 0.0f;
				mJumped = false;
			}
		}
		else {
			AddVelocity(0, dTime * (GameData::sGrav * 4.0f), 0);
		}

		if (mVel.y != 0) {
			Translate(dTime, 0, mVel.y, 0);
			SetDirtyFlag();
		}
	}
	XMFLOAT3 distance = XMFLOAT3{ abs((walkTo.x) - mRI->mBoundingBox.Center.x), mRI->mBoundingBox.Center.y - walkTo.y - mRI->mBoundingBox.Extents.y * 0.5f, abs((walkTo.z) - mRI->mBoundingBox.Center.z) };
	XMFLOAT3 playerDistance = XMFLOAT3{ abs((mPlayer->GetBoundingBox().Center.x) - mRI->mBoundingBox.Center.x), abs(mPlayer->GetBoundingBox().Center.y - mRI->mBoundingBox.Center.y - mRI->mBoundingBox.Extents.y * 0.5f), abs((mPlayer->GetBoundingBox().Center.z) - mRI->mBoundingBox.Center.z) };
	float distanceMagnitude = sqrt((playerDistance.x * playerDistance.x) + (playerDistance.y * playerDistance.y) + (playerDistance.z * playerDistance.z));
	if (distanceMagnitude < 1.5f) {
		//Attack player
		mPlayer->decreasePlayerHealth(1);
	}
	else {
		std::wostringstream woss;
		woss << distance.y << "\n";
		OutputDebugString(woss.str().c_str());
		if (distance.y < -1) {
			Jump();
		}
		else {
			//distance = XMFLOAT3{ abs((nextWalkTo.x) - mRI->mBoundingBox.Center.x), abs(nextWalkTo.y - mRI->mBoundingBox.Center.y), abs((nextWalkTo.z) - mRI->mBoundingBox.Center.z) };
			//if (distance.y > 0.4) {
			//	Jump();
			//}
		}
		//}
		MoveInLook(dTime);
		//}
		//else {
		if (mWorldManager->GetBlock(XMFLOAT3{ mRI->mBoundingBox.Center.x, mRI->mBoundingBox.Center.y, mRI->mBoundingBox.Center.z })->GetActive() == true) {
			mWorldManager->GetBlock(walkTo)->SetActive(false);
		}
		if (mWorldManager->GetBlock(XMFLOAT3{ mRI->mBoundingBox.Center.x, mRI->mBoundingBox.Center.y - 1, mRI->mBoundingBox.Center.z })->GetActive() == true) {
			WalkToBlock(mPlayer->GetBoundingBox().Center);
			if (mWorldManager->GetBlock(walkTo)->GetActive() == true) {
				mWorldManager->GetBlock(walkTo)->SetActive(false);
			}
			else {
				if (mWorldManager->GetBlock(XMFLOAT3{ walkTo.x, walkTo.y - 1, walkTo.z })->GetActive() == false) {
					if (walkTo.x == floor(mRI->mBoundingBox.Center.x) && walkTo.y - 1 == floor(mRI->mBoundingBox.Center.y - 1) && walkTo.z == floor(mRI->mBoundingBox.Center.z)) {
						Jump();
					}
					XMFLOAT3 playerPos = XMFLOAT3{ floor(mPlayer->GetBoundingBox().Center.x), floor(mPlayer->GetBoundingBox().Center.y), floor(mPlayer->GetBoundingBox().Center.z) };
					if (walkTo.x != playerPos.x && walkTo.y != playerPos.y - 1 && walkTo.z != playerPos.z) {
						mWorldManager->GetBlock(XMFLOAT3{ walkTo.x, walkTo.y - 1, walkTo.z })->SetActive(true);
					}
				}
			}
			if (mWorldManager->GetBlock(XMFLOAT3{ walkTo.x, walkTo.y + 1, walkTo.z })->GetActive() == true) {
				mWorldManager->GetBlock(XMFLOAT3{ walkTo.x, walkTo.y + 1, walkTo.z })->SetActive(false);
			}
		}
		//}
	}

	if (mDirty) SetRIDirty();
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
	mRI->mBoundingBox.Transform(nextBoxX, translateX);

	DirectX::FXMMATRIX translateZ = DirectX::XMMatrixTranslation(0, 0.2f, d * dTime);
	BoundingBox nextBoxZ;
	mRI->mBoundingBox.Transform(nextBoxZ, translateZ);

	if (!(CheckIfCollidingAtBox(nextBoxX) && CheckIfCollidingAtBox(nextBoxZ))) {

		d = d * dTime;
		XMVECTOR s = XMVectorReplicate(d);
		XMVECTOR l = { direction.m128_f32[0], 0.0f, direction.m128_f32[2] }; //dont move up along the y
		XMVECTOR p = XMLoadFloat3(&mRI->mBoundingBox.Center);
		XMFLOAT3 newpos;
		XMStoreFloat3(&newpos, XMVectorMultiplyAdd(s, l, p));

		DirectX::XMMATRIX oldWorldMatrix;
		GameData::StoreFloat4x4InMatrix(oldWorldMatrix, mRI->World);
		DirectX::XMMATRIX cameraMatrix = DirectX::XMMatrixTranslation(mRI->mBoundingBox.Center.x, mRI->mBoundingBox.Center.y, mRI->mBoundingBox.Center.z);
		DirectX::XMMATRIX newWorldMatrix = oldWorldMatrix - cameraMatrix;
		Translate(1, -newWorldMatrix.r[3].m128_f32[0], -newWorldMatrix.r[3].m128_f32[1], -newWorldMatrix.r[3].m128_f32[2]);

		SetDirtyFlag();
	}
}
void LivingEntity::Strafe(float d, float dTime) {

	DirectX::FXMMATRIX translate = DirectX::XMMatrixTranslation(d * dTime, 0.2f, 0);
	BoundingBox nextBoxX;
	mRI->mBoundingBox.Transform(nextBoxX, translate);

	if (!CheckIfCollidingAtBox(nextBoxX)) {
		DirectX::XMMATRIX oldWorldMatrix;
		GameData::StoreFloat4x4InMatrix(oldWorldMatrix, mRI->World);
		DirectX::XMMATRIX cameraMatrix = DirectX::XMMatrixTranslation(mRI->mBoundingBox.Center.x, mRI->mBoundingBox.Center.y, mRI->mBoundingBox.Center.z);
		DirectX::XMMATRIX newWorldMatrix = oldWorldMatrix - cameraMatrix;
		Translate(1, -newWorldMatrix.r[3].m128_f32[0], -newWorldMatrix.r[3].m128_f32[1], -newWorldMatrix.r[3].m128_f32[2]);

		SetDirtyFlag();
	}
}

void LivingEntity::CheckCollisions(float dTime) {
	float deltaMove = mMoveSpeed * dTime;

	//FOR LOOPS UNROLLED
	//Calculate the possible next positions
	mMoveVectors[Dir::look] = direction * deltaMove;
	//mMoveVectors[Dir::axisStrafe] = -aStrafe * deltaMove;

	float yOffset = 0.2f;
	mMoveVectors[Dir::look].m128_f32[1] = yOffset;


	DirectX::XMFLOAT3 pos = mRI->mBoundingBox.Center;
	mNextBoxes[Dir::look].Center = GameData::AddFloat3AndVector(pos, mMoveVectors[Dir::look]);

	//Rest the value
	mCanMove[Dir::look] = true;

	int collisions = 0;
	int i, j;
#pragma omp parallel for private(j)
	for (i = 0; i < sAllGObjs->size(); i++) {
		//If the IDs arent the same, the block is active, and it is colliding
		std::shared_ptr<GameObject> go = sAllGObjs->at(i);
		if (mID != go->GetID()) {
			if (go->GetActive()) {

				//Checks the current GObjs against all possible future boxes
				for (j = 0; j < Dir::count; j++) {

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

void LivingEntity::MoveInLook(float dTime) {

	XMVECTOR moveDirection = direction * mMoveSpeed * dTime;

	Translate(1, moveDirection.m128_f32[0], 0.f, moveDirection.m128_f32[2]);

	SetDirtyFlag();

	/*
	//Calculate the walk (forward) and strafe (side) then add them together
	XMVECTOR translate = { 0.f, 0.f, 0.f, 0.f };

	//If you can move both directions, do it
	if (mCanMove[Dir::look]) translate = mMoveVectors[Dir::look];

	//Move the camera and the player
	Translate(1, translate.m128_f32[0], 0.f, translate.m128_f32[2]);

	SetDirtyFlag();



	*/
}

void LivingEntity::DebugPath() {
	if (debugging) {
		debugging = false;
		for (Node n : path) {
			mWorldManager->GetBlock(XMFLOAT3{ (float)n.x, (float)n.y, (float)n.z })->SetActive(false);
			mWorldManager->GetBlock(XMFLOAT3{ (float)n.x, (float)n.y, (float)n.z })->ChangeMaterial("mat_grass");
		}
	}
	else {
		debugging = true;
		WalkToBlock(mPlayer->GetBoundingBox().Center);
	}
}

void LivingEntity::WalkToBlock(XMFLOAT3 blockLocation) {
	Node player;
	player.x = floor(mRI->mBoundingBox.Center.x);
	player.y = floor(mRI->mBoundingBox.Center.y);
	player.z = floor(mRI->mBoundingBox.Center.z);

	Node destination;
	destination.x = floor(blockLocation.x);
	destination.y = floor(blockLocation.y);
	destination.z = floor(blockLocation.z);

	Pathfinding pf = Pathfinding(mWorldManager, mPlayer);

	path = pf.aStar(player, destination);

	if (path.size() > 0) {

		for (int i = path.size() - 1; i >= 0; i--) {
			std::wostringstream woss;
			woss << path[i].x << " " << path[i].y << " " << path[i].z << "\n";
			//OutputDebugString(woss.str().c_str());
		}

		Node node = path[path.size() - 1];
		if (path.size() > 1) {
			nextWalkTo = XMFLOAT3{ (float)path[path.size() - 2].x, (float)path[path.size() - 2].y, (float)path[path.size() - 2].z };
		}
		else {
			nextWalkTo = XMFLOAT3{ (float)node.x, (float)node.y, (float)node.z };
		}

		if (debugging) {
			for (Node n : path) {
				mWorldManager->GetBlock(XMFLOAT3{ (float)n.x, (float)n.y, (float)n.z })->SetActive(true);
				mWorldManager->GetBlock(XMFLOAT3{ (float)n.x, (float)n.y, (float)n.z })->ChangeMaterial("mat_iron_ore");
			}
		}

		std::wostringstream woss;
		woss << player.x << " " << walkTo.x << " " << node.x << " | " << player.y << " " << walkTo.y << " " << node.y << " | " << player.z << " " << walkTo.z << " " << node.z;
		//OutputDebugString(woss.str().c_str());
		walkTo = XMFLOAT3{ (float)node.x, (float)node.y, (float)node.z };
		direction = XMVector3Normalize(XMVECTOR{ (walkTo.x + 0.1f) - mRI->mBoundingBox.Center.x, walkTo.y - mRI->mBoundingBox.Center.y - mRI->mBoundingBox.Extents.y * 0.5f, (walkTo.z + 0.1f) - mRI->mBoundingBox.Center.z });
	}
	else {
		OutputDebugString(L"cant find path");
	}
}