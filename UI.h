#pragma once

#include "Common/GeometryGenerator.h"
#include "GameData.h"
#include "Common/d3dUtil.h"

using Microsoft::WRL::ComPtr;



class UI {
public:
	//Temp
	struct Vertex
	{
		DirectX::XMFLOAT3 Pos;
		DirectX::XMFLOAT3 Normal;
		DirectX::XMFLOAT2 TexC;
	};

	void SetRenderItem(std::shared_ptr<RenderItem> ri);
	void InitFont();
	GeometryGenerator::MeshData CreateUIPlane(float width, float depth, int oM, int oN);
	~UI();

	void UpdateUIPos(DirectX::XMVECTOR camPos);
	void UpdateAspectRatio(float camNearWindowWidth, float camNearWindowHeight);
	void UpdateRotation(float rotX, float rotY, DirectX::XMVECTOR look);

	void SetChar(char character, int position, std::vector<Vertex>& vertices);
	void SetString(ID3D12GraphicsCommandList* cmdList, std::string str, float posX, float posY);
	//void DrawUI(ID3D12GraphicsCommandList* cmdList, const std::vector<std::shared_ptr<RenderItem>> ritems);
	Font* GetFont() { return &mFnt; };

	void UpdateBuffer(ID3D12GraphicsCommandList* cmdList, std::vector<Vertex> vertices);

private:
	Font mFnt;
	int mSizeX = 0;
	int mSizeZ = 0; //Y

	const float mScaleVal = 1.15f;
	const float mRotToPlayer = -1.57079633f;	//-90 degrees
	DirectX::XMMATRIX mScale = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX mRotation = DirectX::XMMatrixIdentity();
	DirectX::XMVECTOR mLook = { 0, 0, 0 };

	std::shared_ptr<RenderItem> mRI;
};