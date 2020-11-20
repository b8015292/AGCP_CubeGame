#include "Object.h"

const float GameData::sGrav = -9.71f;

GameObject::GameObject(std::shared_ptr<std::vector<std::shared_ptr<GameObject>>> allGObjs) : mRI() {
	mAllGObjs = allGObjs;

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

//void GameObject::Translate(const int rItemIndex, const float dTime, float x, float y, float z) {
//	//Gets the translation matrix (scaled by delta time
//	DirectX::XMMATRIX translateMatrix = DirectX::XMMatrixTranslation(x * dTime, y * dTime, z * dTime);
//
//	//Gets the world matrix in XMMATRIX form
//	DirectX::XMMATRIX oldWorldMatrix;
//	GameData::StoreFloat4x4InMatrix(oldWorldMatrix, mAllGObjs->at(rItemIndex)->ri->World);
//
//	//Multiplies the two matricies together
//	DirectX::XMMATRIX newWorldMatrix = XMMatrixMultiply(oldWorldMatrix, translateMatrix);
//
//	//Stores the new matrix, and marks the object as dirty
//	XMStoreFloat4x4(&mAllGObjs->at(rItemIndex)->ri->World, newWorldMatrix);
//	mAllGObjs->at(rItemIndex)->ri->NumFramesDirty++;
//}

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
}

Entity::Entity(std::shared_ptr<std::vector<std::shared_ptr<GameObject>>> allGObjs) : GameObject(allGObjs) {
	mVel = { 0, 0, 0 };
	mMaxVel = mVel;
}

void Entity::Update(const float dTime) {
	if (!active) return;

	if (applyGravity) {
		mVel.y = GameData::sGrav;
	}

	if (mVel.x != 0 || mVel.y != 0 || mVel.z != 0) {
		Translate(dTime, mVel.x, mVel.y, mVel.z);
	}
}


