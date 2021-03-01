#pragma once

#include "GameData.h"
#include "Common/d3dUtil.h"
#include "RenderItem.h"

using Microsoft::WRL::ComPtr;

class UI {
public:
	//Init
	void Init(std::shared_ptr<RenderItem> ri, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList);	//Sets the verticies list and other variables. Use after the RI has been made with CreateUIPlane2D
	GeometryGenerator::MeshData CreateUIPlane2D(float width, float depth, int oM, int oN);					//Creates a 2D plane which can display characters
	~UI();				//Releases the render item and command list

	//Mutators
	void SetTexture(const int VertexPos, DirectX::XMFLOAT2 texturePos, const DirectX::XMFLOAT2 textureSize);
	void SetWholeTexture(DirectX::XMFLOAT2 texturePos, const DirectX::XMFLOAT2 textureSize);
	void UpdateBuffer();									//Updates the GPUs version of the verticies. Use this in the main Draw function in CubeGame.cpp, before drawing this.
	void ResetVerticies() { mVertices = mStartVertices; };	//Clears the GUI.

	//Getters/Setters
	std::shared_ptr<RenderItem> GetRI() { return mRI; };

	//Dirty
	bool GetDirty() { return mDirty; };
	void SetDirtyFlag() { mDirty = true; mChanged = true; };
	void SetRIDirty() { mRI->NumFramesDirty++; mDirty = false; };

protected:
	int mSizeX = 0;			//The number of colomns (including the hidden colomns inbetween the visible)
	int mSizeY = 0;			//The number of rows (see above comment)

	UINT mVertsPerObj = 0;	//The number of verticies in the render item. 
	UINT mVbByteSize = 0;	//Number of verticies * size of vertex

	std::vector<GeometryGenerator::Vertex> mVertices;		//Holds all the verticies. The texCoords are changed to display different characters
	std::vector<GeometryGenerator::Vertex> mStartVertices;	//Holds the initial, unaltered verticies. Used to refresh the gui to make it blank.

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> mCmdList;
	std::shared_ptr<RenderItem> mRI;
	bool mDirty = false;				//Used to update the render items dirty flag.
	bool mChanged = false;				//Used in UpdateBuffer. If there has been no change, the buffer is not updated.
										//The mDirty flag is already reset by this time so it cannot be used.
};