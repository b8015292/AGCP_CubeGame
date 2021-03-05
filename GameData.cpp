#include "GameData.h"

std::vector<UINT> GameData::sAvailableObjCBIndexes;
std::shared_ptr<std::map<std::string, std::shared_ptr<Material>>> GameData::sMaterials = std::make_shared<std::map<std::string, std::shared_ptr<Material>>>();

UINT GameData::GetObjectCBIndex() {
	UINT ret = sAvailableObjCBIndexes.at(sAvailableObjCBIndexes.size() - 1);
	sAvailableObjCBIndexes.pop_back();
	return ret;
}
void GameData::AddNewObjectCBIndex(UINT i) {
	sAvailableObjCBIndexes.push_back(i);
}

void GameData::StoreFloat4x4InMatrix(DirectX::XMMATRIX& dest, const DirectX::XMFLOAT4X4 source) {
	DirectX::XMMATRIX newMatrix(source._11, source._12, source._13, source._14,
		source._21, source._22, source._23, source._24,
		source._31, source._32, source._33, source._34,
		source._41, source._42, source._43, source._44);

	dest = newMatrix;
}

std::wstring GameData::StringToWString(std::string s) {
	std::wstring temp(s.length(), L' ');
	std::copy(s.begin(), s.end(), temp.begin());
	return temp;
}

void GameData::Print(std::string message) {
	OutputDebugStringW(StringToWString(message).c_str());
}

DirectX::XMFLOAT3 GameData::AddFloat3AndVector(DirectX::XMFLOAT3 inFloat3, DirectX::XMVECTOR inVector) {
	inFloat3.x += inVector.m128_f32[0];
	inFloat3.y += inVector.m128_f32[1];
	inFloat3.z += inVector.m128_f32[2];

	return inFloat3;
}
