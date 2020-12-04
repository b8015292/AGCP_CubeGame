#include "GameData.h"

int RenderItem::sCBIndex = -1;

RenderItem::RenderItem(MeshGeometry* meshGeo, std::string meshName, Material* mat, DirectX::XMMATRIX world) {
	ObjCBIndex = ++sCBIndex;
	Mat = mat;
	Geo = meshGeo;
	MeshName = meshName;
	IndexCount = Geo->DrawArgs[MeshName].IndexCount;
	StartIndexLocation = Geo->DrawArgs[MeshName].StartIndexLocation;
	BaseVertexLocation = Geo->DrawArgs[MeshName].BaseVertexLocation;
	XMStoreFloat4x4(&World, world);
}

void GameData::StoreFloat4x4InMatrix(DirectX::XMMATRIX& dest, const DirectX::XMFLOAT4X4 source) {
	DirectX::XMMATRIX newMatrix(source._11, source._12, source._13, source._14,
		source._21, source._22, source._23, source._24,
		source._31, source._32, source._33, source._34,
		source._41, source._42, source._43, source._44);

	dest = newMatrix;
}