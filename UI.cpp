#include "UI.h"

UI::~UI() {
	mRI.~shared_ptr();
	mCmdList.~ComPtr();
}

void UI::Init(std::shared_ptr<RenderItem> ri, Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList) {
	mRI = ri;
	mCmdList = cmdList;

	//Move the data from the vertex buffer onto a vector we can view/manipulate. This moves it from a 'blob' format into the Vertex vector format
	CopyMemory(mVertices.data(), mRI->Geo->VertexBufferCPU->GetBufferPointer(), mVbByteSize);
	mStartVertices = mVertices;

	//Move the image so it fills the whole screen
	//DirectX::XMMatrixMultiply();
	DirectX::XMMATRIX world;
	GameData::StoreFloat4x4InMatrix(world, mRI->World);

	XMStoreFloat4x4(&mRI->World, DirectX::XMMatrixMultiply(world, DirectX::XMMatrixTranslation(-0.02f, 0.02f, 0)));
	SetDirtyFlag();
}

GeometryGenerator::MeshData UI::CreateUIPlane2D(float widthOfPlane, float heightOfPlane, int numbOfCols, int numbOfRows) {
	GeometryGenerator::MeshData meshData;

	//Multiplies the rows and colomns by 2
	mSizeX = (int)numbOfCols * 2;
	mSizeY = (int)numbOfRows * 2;

	//Set these for use in later functions
	mVertsPerObj = mSizeX * mSizeY;
	mVbByteSize = mVertsPerObj * sizeof(GeometryGenerator::Vertex);
	mVertices.resize(mVertsPerObj);


	int faceCount = (mSizeX - 1) * (mSizeY - 1) * 2;
	float halfWidth = 0.5f * widthOfPlane;
	float halfHeight = 0.5f * heightOfPlane;

	float faceWidth = (widthOfPlane / ((float)mSizeY - 1)) * 2.f; //Vertices per row
	float faceHeight = (heightOfPlane / ((float)mSizeX - 1)) * 2.f; //Vertices per column

	meshData.Vertices.resize(mVertsPerObj);

	//Iterate along the colomns
	float y = halfHeight;
	for (int i = 0; i < mSizeX; ++i)
	{
		float x = -halfWidth;
		//Iterate along the rows
		for (int j = 0; j < mSizeY; ++j)
		{

			meshData.Vertices[i * mSizeY + j].Pos = DirectX::XMFLOAT3(x, y, 0.0f);

			if (j != 0 && j < mSizeY - 1) {
				j++;

				meshData.Vertices[i * mSizeY + j].Pos = DirectX::XMFLOAT3(x, y, 0.0f);
			}

			x += faceWidth;
		}

		if (i != 0 && i < mSizeX - 1) {
			i++;
			x = -halfWidth;

			for (int j = 0; j < mSizeY; ++j)
			{

				meshData.Vertices[i * mSizeY + j].Pos = DirectX::XMFLOAT3(x, y, 0.0f);

				if (j != 0 && j < mSizeY - 1) {
					j++;

					meshData.Vertices[i * mSizeY + j].Pos = DirectX::XMFLOAT3(x, y, 0.0f);
				}

				x += faceWidth;
			}
		}

		y -= faceHeight;
	}

	//
	// Create the indices.
	//

	meshData.Indices32.resize(faceCount * 3); // 3 indices per face

	// Iterate over each quad and compute indices.
	GeometryGenerator::uint32 k = 0;
	for (GeometryGenerator::uint32 i = 0; i < (GeometryGenerator::uint32)mSizeX - 1; ++i)
	{
		for (GeometryGenerator::uint32 j = 0; j < (GeometryGenerator::uint32)mSizeY - 1; ++j)
		{
			meshData.Indices32[k] = i * mSizeY + j;
			meshData.Indices32[k + 1] = i * mSizeY + j + 1;
			meshData.Indices32[k + 2] = (i + 1) * mSizeY + j;

			meshData.Indices32[k + 3] = (i + 1) * mSizeY + j;
			meshData.Indices32[k + 4] = i * mSizeY + j + 1;
			meshData.Indices32[k + 5] = (i + 1) * mSizeY + j + 1;

			k += 6; // next quad
		}
	}

	return meshData;
}

GeometryGenerator::MeshData UI::CreateUIPlane2DWithSpaces(float widthOfPlane, float heightOfPlane, int numbOfRows, int numbOfCols, float gapWidthRatio, float gapHeightRatio) {
	GeometryGenerator::MeshData meshData;

	bool colGap = true;
	bool rowGap = true;

	if (gapHeightRatio != 0) {
		mSizeX = (int)numbOfRows * 4 - 2;
	}
	else {
		rowGap = false;
		mSizeX = (int)numbOfRows * 2;
	}

	if (gapWidthRatio != 0) {
		mSizeY = (int)numbOfCols * 4 - 2;
	}
	else {
		colGap = false;
		mSizeY = (int)numbOfCols * 2;
	}

	//Set these for use in later functions
	mVertsPerObj = mSizeX * mSizeY;
	mVbByteSize = mVertsPerObj * sizeof(GeometryGenerator::Vertex);
	mVertices.resize(mVertsPerObj);

	//Number of faces, including the gaps (used for indicies)
	int faceCount = (mSizeX - 1) * (mSizeY - 1) * 2;

	//Because the centre (0,0) is in the centre of the screen instead of a corner, half the width is in both directions
	float halfWidth = 0.5f * widthOfPlane;
	float halfHeight = 0.5f * heightOfPlane;

	////Get the size of each face without considering the gaps
	//float faceWidth = (widthOfPlane / ((float)mSizeY - 1)) * 2.f; //Vertices per row
	//float faceHeight = (heightOfPlane / ((float)mSizeX - 1)) * 2.f; //Vertices per column

	////Get the size of the gaps
	//float gapWidth = faceWidth * gapWidthRatio;
	//float gapHeight = faceHeight * gapHeightRatio;

	////Remove the size of the gaps from the faces
	//faceWidth -= gapWidth;
	//faceHeight -= gapHeight;




	//Get the size of each face without considering the gaps
	float faceWidth = (widthOfPlane / ((float)mSizeY - 1)) * 4.f; //Vertices per row
	float faceHeight = (heightOfPlane / ((float)mSizeX - 1)) * 2.f; //Vertices per column

	//Get the size of the gaps
	float gapWidth = faceWidth* gapWidthRatio;
	float gapHeight = faceHeight * gapHeightRatio;

	//Remove the size of the gaps from the faces
	faceWidth -= gapWidth;
	faceHeight -= gapHeight;




	meshData.Vertices.resize(mVertsPerObj);

	//Iterate along the colomns
	float y = halfHeight;
	for (int i = 0; i < mSizeX; ++i)
	{
		float x = -halfWidth;
		//Iterate along the rows
		for (int j = 0; j < mSizeY; ++j)
		{

			meshData.Vertices[i * mSizeY + j].Pos = DirectX::XMFLOAT3(x, y, 0.0f);

			if (j != 0 && j < mSizeY - 3) {
				j++;
				meshData.Vertices[i * mSizeY + j].Pos = DirectX::XMFLOAT3(x, y, 0.0f);

				if (colGap) {
					x += gapWidth;
					j++;
					meshData.Vertices[i * mSizeY + j].Pos = DirectX::XMFLOAT3(x, y, 0.0f);
					j++;
					meshData.Vertices[i * mSizeY + j].Pos = DirectX::XMFLOAT3(x, y, 0.0f);
				}
			}

			x += faceWidth;
		}

		if (i != 0 && i < mSizeX - 1) {
			i++;
			x = -halfWidth;
			y += gapHeight;
			for (int j = 0; j < mSizeY; ++j)
			{

				meshData.Vertices[i * mSizeY + j].Pos = DirectX::XMFLOAT3(x, y, 0.0f);

				if (j != 0 && j < mSizeY - 1) {
					j++;
					meshData.Vertices[i * mSizeY + j].Pos = DirectX::XMFLOAT3(x, y, 0.0f);

					if (colGap) {
						x += gapWidth;
						j++;
						meshData.Vertices[i * mSizeY + j].Pos = DirectX::XMFLOAT3(x, y, 0.0f);
						j++;
						meshData.Vertices[i * mSizeY + j].Pos = DirectX::XMFLOAT3(x, y, 0.0f);
					}
				}

				x += faceWidth;
			}
		}

		y -= faceHeight;
	}

	//
	// Create the indices.
	//

	meshData.Indices32.resize((size_t)faceCount * 3); // 3 indices per face

	// Iterate over each quad and compute indices.
	GeometryGenerator::uint32 k = 0;
	for (GeometryGenerator::uint32 i = 0; i < (GeometryGenerator::uint32)mSizeX - 1; ++i)
	{
		for (GeometryGenerator::uint32 j = 0; j < (GeometryGenerator::uint32)mSizeY - 1; ++j)
		{
			meshData.Indices32[k] = i * mSizeY + j;
			meshData.Indices32[k + 1] = i * mSizeY + j + 1;
			meshData.Indices32[k + 2] = (i + 1) * mSizeY + j;

			meshData.Indices32[k + 3] = (i + 1) * mSizeY + j;
			meshData.Indices32[k + 4] = i * mSizeY + j + 1;
			meshData.Indices32[k + 5] = (i + 1) * mSizeY + j + 1;

			k += 6; // next quad
		}
	}

	return meshData;
}


void UI::UpdateBuffer() {
	if (mChanged) {
		mChanged = false;
		// Describe the data we want to copy into the default buffer.
		D3D12_SUBRESOURCE_DATA subResourceData = {};
		subResourceData.pData = mVertices.data();
		subResourceData.RowPitch = (UINT)sizeof(GeometryGenerator::Vertex) * (UINT)mVertices.size();
		subResourceData.SlicePitch = subResourceData.RowPitch;
		//subResourceData.SlicePitch = 1;

		// Schedule to copy the data to the default buffer resource.  At a high level, the helper function UpdateSubresources
		// will copy the CPU memory into the intermediate upload heap.  Then, using ID3D12CommandList::CopySubresourceRegion,
		// the intermediate upload heap data will be copied to mBuffer.
		mCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mRI->Geo->VertexBufferGPU.Get(),
			D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST));

		UpdateSubresources<1>(mCmdList.Get(), mRI->Geo->VertexBufferGPU.Get(), mRI->Geo->VertexBufferUploader.Get(), 0, 0, 1, &subResourceData);

		mCmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mRI->Geo->VertexBufferGPU.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));
	}
}

void UI::SetTexture(const int VertexPos, DirectX::XMFLOAT2 texturePos, const DirectX::XMFLOAT2 textureSize) {
	int pos = VertexPos + VertexPos;				//Skip every other colomn
	int row = (VertexPos / (mSizeX / 2));	//Calculate the row number
	row *= mSizeX;					//Multiply the row number by the size of each row
	pos += row;						//Add the x and y positions

	//If there aren't enough spaces, don't set the char
	if (pos >= mVertices.size()) return;

	//Set a square of texture coords
	mVertices[pos].TexC = { texturePos.x, texturePos.y };
	mVertices[pos + 1].TexC = { texturePos.x + textureSize.x, texturePos.y };
	mVertices[pos + mSizeX].TexC = { texturePos.x, texturePos.y + textureSize.y };
	mVertices[pos + mSizeX + 1].TexC = { texturePos.x + textureSize.x, texturePos.y + textureSize.y };
}

void UI::SetWholeTexture(DirectX::XMFLOAT2 texturePos, const DirectX::XMFLOAT2 textureSize) {
	mVertices[0].TexC = { texturePos.x, texturePos.y };
	mVertices[1].TexC = { texturePos.x + textureSize.x, texturePos.y };
	mVertices[2].TexC = { texturePos.x, texturePos.y + textureSize.y };
	mVertices[3].TexC = { texturePos.x + textureSize.x, texturePos.y + textureSize.y };
}