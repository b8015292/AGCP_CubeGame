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
	void UpdateRotation(float rotX, float rotY, DirectX::XMVECTOR);

	void SetChar(char character, int position, std::vector<Vertex>& vertices);
	void SetString(ID3D12GraphicsCommandList* cmdList, std::string str, float posX, float posY);
	//void DrawUI(ID3D12GraphicsCommandList* cmdList, const std::vector<std::shared_ptr<RenderItem>> ritems);
	Font* GetFont() { return &mFnt; };

	void UpdateBuffer(ID3D12GraphicsCommandList* cmdList, std::vector<Vertex> vertices);

private:
	Font mFnt;
	int mSizeX = 0;
	int mSizeZ = 0; //Y

	float mWindowWidth = 0;
	float mWindowHeight = 0;
	float mRotationX = 0;
	float mRotationY = 0;
	DirectX::XMVECTOR mLook = { 0, 0, 1.f };

	std::shared_ptr<RenderItem> mRI;
};