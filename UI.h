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

	bool GetDirty() { return mDirty; };
	void SetDirtyFlag() { mDirty = true; };
	void SetRIDirty() { mRI->NumFramesDirty++; mDirty = false; };


private:
	Font mFnt;
	int mSizeX = 0;
	int mSizeZ = 0; //Y - because the UI is built on a plane which is parallel to the Z axis before rotation,
					//	  after rotation its Z becomes its height

	const float mScaleVal = 1.f;				//Scale
	const float offsetFromPlayer = 1.00001f;	//Translate
	const float mRotToPlayer = -1.57079633f;	//-90 degrees rotate
	DirectX::XMMATRIX mRotToPlayerMat = DirectX::XMMatrixRotationX(mRotToPlayer);
	DirectX::XMMATRIX mScale = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX mRotation = DirectX::XMMatrixIdentity();
	DirectX::XMVECTOR mLook = { 0, 0, 0 };

	float rotXX = 0;
	float rotYY = 0;
	float windWidth = 0;
	float windHeight = 0;

	std::shared_ptr<RenderItem> mRI;
	bool mDirty = false;
};