#include "Object.h"
#include "Pathfinding.h"
#include "Player.h"
#include "LivingEntity.h"

const float GameData::sGrav = -9.71f;
int GameObject::sMaxID = 0;

std::shared_ptr<std::vector<std::shared_ptr<GameObject>>> GameObject::sAllGObjs = std::make_shared<std::vector<std::shared_ptr<GameObject>>>();
std::shared_ptr<std::vector<std::shared_ptr<Entity>>> Entity::sAllEntities = std::make_shared<std::vector<std::shared_ptr<Entity>>>();
std::shared_ptr<std::vector<std::shared_ptr<ItemEntity>>> ItemEntity::sAllItemEntities = std::make_shared<std::vector<std::shared_ptr<ItemEntity>>>();
std::shared_ptr<std::vector<std::shared_ptr<LivingEntity>>> LivingEntity::sAllLivingEntities = std::make_shared<std::vector<std::shared_ptr<LivingEntity>>>();
std::shared_ptr<std::vector<std::shared_ptr<Block>>> Block::sAllBlocks = std::make_shared<std::vector<std::shared_ptr<Block>>>();
std::shared_ptr<RenderItemInstance> Block::sBlockInstance = std::make_shared<RenderItemInstance>();

//************************************************************************************************************
// GameObject
//************************************************************************************************************

GameObject::GameObject(std::shared_ptr<RenderItem> rI) {
	mRI = rI;

	if (mID == 0) mID = ++sMaxID;	//Incase an entity is being made from a preconstructed GObj
}

GameObject::GameObject(std::shared_ptr<GameObject> gobj) : mRI(gobj->GetRI()){
	*this = *gobj;
	if (mID == 0) mID = ++sMaxID;	//Incase an entity is being made from a preconstructed GObj
}

GameObject::GameObject(){// : mRI() {
}

void GameObject::SetActive(bool val) {
	mActive = val;
	mRI->active = val;
	SetDirtyFlag();
}
//
//std::array<XMFLOAT3, 8> GameObject::GetCoords() {
//	//Define constants
//	const UINT vertsPerObj = 24;
//	const UINT vertsNeeded = 8;
//	const UINT numbOfVerts = vertsPerObj * (UINT)(mRI->Geo->DrawArgs.size());
//	const UINT vbByteSize = numbOfVerts * sizeof(GeometryGenerator::Vertex);
//
//	//Where the verticies for this item start in the buffer
//	const int vertStart = mRI->BaseVertexLocation;
//
//	//A pointer to the buffer of vertices
//	ComPtr<ID3DBlob> verticesBlob = mRI->Geo->VertexBufferCPU;
//
//	//Move the data from the buffer onto a vector we can view/manipulate
//	std::vector<GeometryGenerator::Vertex> vs(numbOfVerts);
//	CopyMemory(vs.data(), verticesBlob->GetBufferPointer(), vbByteSize);
//
//	//Get the items world transformation matrix
//	XMMATRIX transform;
//	GameData::StoreFloat4x4InMatrix(transform, mRI->World);
//
//	//Transform each vertex by its world matrix
//	for (int i = vertStart; i < (vertStart + (int)vertsNeeded); i++) {
//
//		XMVECTOR temp = XMLoadFloat3(&vs.at(i).Pos);;
//		temp = XMVector3TransformCoord(temp, transform);
//
//		XMStoreFloat3(&vs.at(i).Pos, temp);
//	}
//
//	////Store all the proper positions in a struct
//	//Collision::ColCube c(vs[vertStart + 7].Pos, vs[vertStart + 6].Pos, vs[vertStart + 1].Pos, vs[vertStart + 2].Pos, vs[vertStart + 4].Pos, vs[vertStart + 5].Pos, vs[vertStart].Pos, vs[vertStart + 3].Pos);
//	std::array<XMFLOAT3, 8> c = { vs[vertStart + 7].Pos, vs[vertStart + 6].Pos, vs[vertStart + 1].Pos, vs[vertStart + 2].Pos, vs[vertStart + 4].Pos, vs[vertStart + 5].Pos, vs[vertStart].Pos, vs[vertStart + 3].Pos };
//
//	return c;
//}

void GameObject::Translate(const float dTime, float x, float y, float z) {
	//Gets the translation matrix (scaled by delta time)
	DirectX::XMMATRIX translateMatrix = DirectX::XMMatrixTranslation(x * dTime, y * dTime, z * dTime);

	//Gets the world matrix in XMMATRIX form
	DirectX::XMMATRIX oldWorldMatrix;
	GameData::StoreFloat4x4InMatrix(oldWorldMatrix, mRI->World);

	//Multiplies the two matricies together
	DirectX::XMMATRIX newWorldMatrix = XMMatrixMultiply(oldWorldMatrix, translateMatrix);

	//Stores the new matrix, and marks the object as dirty
	XMStoreFloat4x4(&mRI->World, newWorldMatrix);

	//Translate the bounding box
	mRI->mBoundingBox.Transform(mRI->mBoundingBox, translateMatrix);

	SetDirtyFlag();
}

void GameObject::Rotate(const float dTime, XMVECTOR axis, float angle) {
	float x = GetBoundingBox().Center.x;
	float y = GetBoundingBox().Center.y;
	float z = GetBoundingBox().Center.z;

	//Gets the translation matrix (scaled by delta time)
	DirectX::XMMATRIX translateMatrix = DirectX::XMMatrixRotationAxis(axis, angle * dTime);
	DirectX::XMMATRIX translateToOriginMatrix = DirectX::XMMatrixTranslation(-x, -y, -z);
	DirectX::XMMATRIX translateBackMatrix = DirectX::XMMatrixTranslation(x, y, z);

	DirectX::XMMATRIX finalMatrix = XMMatrixMultiply(translateToOriginMatrix, translateMatrix);
	finalMatrix = XMMatrixMultiply(finalMatrix, translateBackMatrix);

	//Gets the world matrix in XMMATRIX form
	DirectX::XMMATRIX oldWorldMatrix;
	GameData::StoreFloat4x4InMatrix(oldWorldMatrix, mRI->World);

	//Multiplies the two matricies together
	DirectX::XMMATRIX newWorldMatrix = XMMatrixMultiply(oldWorldMatrix, finalMatrix);

	//Stores the new matrix, and marks the object as dirty
	XMStoreFloat4x4(&mRI->World, newWorldMatrix);

	SetDirtyFlag();
}

void GameObject::SetPosition(XMFLOAT3 pos) {
	DirectX::XMMATRIX translateMatrix = DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);

	//Stores the new matrix, and marks the object as dirty
	XMStoreFloat4x4(&mRI->World, translateMatrix);
	SetDirtyFlag();

	mRI->mBoundingBox.Center = pos;
}

//************************************************************************************************************
// Entity
//************************************************************************************************************

Entity::Entity(std::shared_ptr<GameObject> gobj) : GameObject(gobj) {
	Init();
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
	mRI->mBoundingBox.Transform(nextBox, translate);

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
	
	if(mDirty) SetRIDirty();
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
	for (int i = 0; i < sAllGObjs->size(); i++){
		if (sAllGObjs->at(i)->GetActive() && nextPos.Contains(sAllGObjs->at(i)->GetBoundingBox()) != DirectX::ContainmentType::DISJOINT) {
			collisionIndexs.push_back(i);
		}
	}
	return collisionIndexs;
}

bool Entity::CheckIfCollidingAtBox(BoundingBox nextPos) {
	for (int i = 0; i < sAllGObjs->size(); i++) {
		//If the IDs arent the same, the block is active, and it is colliding
		std::shared_ptr<GameObject> go = sAllGObjs->at(i);
		if (mID != go->GetID())
			if(go->GetActive())
				if(nextPos.Contains(go->GetBoundingBox()) != DirectX::ContainmentType::DISJOINT) //check it does not contain the next box
					return true;
	}
	return false;
}



//************************************************************************************************************
// ItemEntity
//************************************************************************************************************



ItemEntity::ItemEntity(std::shared_ptr<GameObject> gobj, char texRef) : Entity(gobj) {
	//sBlockInstance->Instances.push_back(idata);
	
	mItemTextureReference = texRef;
	if (mID == 0) mID = ++sMaxID;	//Incase an entity is being made from a preconstructed GObj
}

void ItemEntity::Update(const float dTime) {
	Entity::Update(dTime);

	Rotate(dTime, XMVECTOR{ 0, 1, 0 }, 1.f);
}

void ItemEntity::Pickup() {
	SetActive(false);
	stackedAmount = 1;
}

void ItemEntity::AddStack() {
	stackedAmount++;
}
int ItemEntity::GetStackAmount() {
	return stackedAmount;
}



//************************************************************************************************************
// ItemEntity
//************************************************************************************************************







//************************************************************************************************************
// Block
//************************************************************************************************************



Block::Block(std::shared_ptr<InstanceData> idata) {
	mInstanceData = idata;

	
	//sBlockInstance->Instances.push_back(idata);

	//mRI->CreateBoundingBox();
	
	if (mID == 0) mID = ++sMaxID;	//Incase an entity is being made from a preconstructed GObj
}

GeometryGenerator::MeshData Block::CreateCubeGeometry(float width, float height, float depth, float texWidth, float texHeight) {
	GeometryGenerator::MeshData meshData;

	// Create the vertices.

	float w2 = 0.5f * width;
	float h2 = 0.5f * height;
	float d2 = 0.5f * depth;

	GeometryGenerator::Vertex v[24] = {
		// Fill in the front face vertex data.
		//Coords,       Normal,            Tex Coords
		{-w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 0.0f, texHeight},
		{-w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f},
		{+w2, +h2, -d2, 0.0f, 0.0f, -1.0f, texWidth, 0.0f},
		{+w2, -h2, -d2, 0.0f, 0.0f, -1.0f, texWidth, texHeight},

		//Back
		{-w2, -h2, +d2, 0.0f, 0.0f, 1.0f, texWidth, texHeight},
		{+w2, -h2, +d2, 0.0f, 0.0f, 1.0f, 0.0f, texHeight},
		{+w2, +h2, +d2, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f},
		{-w2, +h2, +d2, 0.0f, 0.0f, 1.0f, texWidth, 0.0f},

		// Fill in the top face vertex data.
		{-w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 0.0f, texHeight},
		{-w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f},
		{+w2, +h2, +d2, 0.0f, 1.0f, 0.0f, texWidth, 0.0f},
		{+w2, +h2, -d2, 0.0f, 1.0f, 0.0f, texWidth, texHeight},

		// Fill in the bottom face vertex data.
		{-w2, -h2, -d2, 0.0f, -1.0f, 0.0f, texWidth, texHeight},
		{+w2, -h2, -d2, 0.0f, -1.0f, 0.0f, 0.0f, texHeight},
		{+w2, -h2, +d2, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f},
		{-w2, -h2, +d2, 0.0f, -1.0f, 0.0f, texWidth, 0.0f},

		// Fill in the left face vertex data.
		{-w2, -h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, texHeight},
		{-w2, +h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f},
		{-w2, +h2, -d2, -1.0f, 0.0f, 0.0f, texWidth, 0.0f},
		{-w2, -h2, -d2, -1.0f, 0.0f, 0.0f, texWidth, texHeight},

		// Fill in the right face vertex data.
		{+w2, -h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, texHeight},
		{+w2, +h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f},
		{+w2, +h2, +d2, 1.0f, 0.0f, 0.0f, texWidth, 0.0f},
		{+w2, -h2, +d2, 1.0f, 0.0f, 0.0f, texWidth, texHeight},
	};

	meshData.Vertices.assign(&v[0], &v[24]);

	// Create the indices.

	GeometryGenerator::uint32 i[36];

	// Fill in the front face index data
	i[0] = 0; i[1] = 1; i[2] = 2;
	i[3] = 0; i[4] = 2; i[5] = 3;

	// Fill in the back face index data
	i[6] = 4; i[7] = 5; i[8] = 6;
	i[9] = 4; i[10] = 6; i[11] = 7;

	// Fill in the top face index data
	i[12] = 8; i[13] = 9; i[14] = 10;
	i[15] = 8; i[16] = 10; i[17] = 11;

	// Fill in the bottom face index data
	i[18] = 12; i[19] = 13; i[20] = 14;
	i[21] = 12; i[22] = 14; i[23] = 15;

	// Fill in the left face index data
	i[24] = 16; i[25] = 17; i[26] = 18;
	i[27] = 16; i[28] = 18; i[29] = 19;

	// Fill in the right face index data
	i[30] = 20; i[31] = 21; i[32] = 22;
	i[33] = 20; i[34] = 22; i[35] = 23;

	meshData.Indices32.assign(&i[0], &i[36]);

	//// Put a cap on the number of subdivisions.
	//numSubdivisions = std::min<GeometryGenerator::uint32>(numSubdivisions, 6u);

	//for (int i = 0; i < numSubdivisions; ++i)
	//	Subdivide(meshData);

	return meshData;
}

void Block::SetActive(bool val) {
	mActive = val;
	mInstanceData->Active = val;
	SetDirtyFlag();
}

void Block::SetPosition(XMFLOAT3 pos) {
	DirectX::XMMATRIX translateMatrix = DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);

	//Stores the new matrix, and marks the object as dirty
	XMStoreFloat4x4(&mInstanceData->World, translateMatrix);
	SetDirtyFlag();

	mRI->UpdateBoundingBox();
}

void Block::Translate(const float dTime, float x, float y, float z) {
	//Gets the translation matrix (scaled by delta time
	DirectX::XMMATRIX translateMatrix = DirectX::XMMatrixTranslation(x * dTime, y * dTime, z * dTime);

	//Gets the world matrix in XMMATRIX form
	DirectX::XMMATRIX oldWorldMatrix;
	GameData::StoreFloat4x4InMatrix(oldWorldMatrix, mInstanceData->World);

	//Multiplies the two matricies together
	DirectX::XMMATRIX newWorldMatrix = XMMatrixMultiply(oldWorldMatrix, translateMatrix);

	//Stores the new matrix, and marks the object as dirty
	XMStoreFloat4x4(&mInstanceData->World, newWorldMatrix);
	mRI->NumFramesDirty++;

	//Translate the bounding box
	mRI->mBoundingBox.Transform(mRI->mBoundingBox, translateMatrix);
}

void Block::ChangeMaterial(std::string newMaterial) {
	mInstanceData->MaterialIndex = GameData::sMaterials->at(newMaterial)->MatCBIndex;
}