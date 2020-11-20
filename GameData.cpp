#include "GameData.h"

void GameData::StoreFloat4x4InMatrix(DirectX::XMMATRIX& dest, const DirectX::XMFLOAT4X4 source) {
	DirectX::XMMATRIX newMatrix(source._11, source._12, source._13, source._14,
		source._21, source._22, source._23, source._24,
		source._31, source._32, source._33, source._34,
		source._41, source._42, source._43, source._44);

	dest = newMatrix;
}