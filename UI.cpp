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
	XMStoreFloat4x4(&mRI->World, DirectX::XMMatrixTranslation(-0.02f, 0.02f, 0));
	SetDirtyFlag();
}

GeometryGenerator::MeshData UI::CreateUIPlane2D(float width, float height, int oM, int oN) {
	GeometryGenerator::MeshData meshData;

	//Multiplies the rows and colomns by 2
	mSizeX = (int)oM * 2;
	mSizeY = (int)oN * 2;

	//Set these for use in later functions
	mVertsPerObj = mSizeX * mSizeY;
	mVbByteSize = mVertsPerObj * sizeof(GeometryGenerator::Vertex);
	mVertices.resize(mVertsPerObj);


	int faceCount = (mSizeX - 1) * (mSizeY - 1) * 2;
	float halfWidth = 0.5f * width;
	float halfHeight = 0.5f * height;


	float dx = width / ((float)mSizeY - 1); //Vertices per row
	float dz = height / ((float)mSizeX - 1); //Vertices per column

	//Normalized tex coord increment per vertex
	float du = 1.0f / ((float)mSizeY - 1); 
	float dv = 1.0f / ((float)mSizeX - 1);

	int decI = 0;
	meshData.Vertices.resize(mVertsPerObj);
	for (int i = 0; i < mSizeX; ++i)
	{
		float y = halfHeight - (float)(i - decI) * (dz * 2.f);
		int decJ = 0;
		for (int j = 0; j < mSizeY; ++j)
		{

			float x = -halfWidth + (float)(j - decJ) * (dx * 2.f);
			size_t index = i * mSizeY + j;

			meshData.Vertices[index].Pos = DirectX::XMFLOAT3(x, y, 0.0f);
			meshData.Vertices[index].Normal = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);

			// Stretch texture over grid.
			//meshData.Vertices[index].TexC.x = (float)j * du;
			//meshData.Vertices[index].TexC.y = (float)i * dv;
			meshData.Vertices[index].TexC = { 0.f, 0.f };

			if (j != 0 && j < mSizeY - 1) {
				j++;
				index = i * mSizeY + j;

				meshData.Vertices[index].Pos = DirectX::XMFLOAT3(x, y, 0.0f);
				meshData.Vertices[index].Normal = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);

				// Stretch texture over grid.
				//meshData.Vertices[index].TexC.x = (float)(j - 1) * du;
				//meshData.Vertices[index].TexC.y = (float)(i - 1) * dv;
				meshData.Vertices[index].TexC = { 0.f, 0.f };

				decJ++;
			}

		}

		if (i != 0 && i < mSizeX - 1) {
			i++;

			int decJ = 0;
			for (int j = 0; j < mSizeY; ++j)
			{

				float x = -halfWidth + (float)(j - decJ) * (dx * 2);
				size_t index = i * mSizeY + j;

				meshData.Vertices[index].Pos = DirectX::XMFLOAT3(x, y, 0.0f);
				meshData.Vertices[index].Normal = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);

				// Stretch texture over grid.
				//meshData.Vertices[index].TexC.x = (float)j * du;
				//meshData.Vertices[index].TexC.y = (float)i * dv;
				meshData.Vertices[index].TexC = { 0.f, 0.f };

				if (j != 0 && j < mSizeY - 1) {
					j++;
					index = i * mSizeY + j;

					meshData.Vertices[index].Pos = DirectX::XMFLOAT3(x, y, 0.0f);
					meshData.Vertices[index].Normal = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);

					// Stretch texture over grid.
					//meshData.Vertices[index].TexC.x = (float)(j - 1) * du;
					//meshData.Vertices[index].TexC.y = (float)(i - 1) * dv;
					meshData.Vertices[index].TexC = { 0.f, 0.f };

					decJ++;
				}

			}

			decI++;
		}
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

void UI::UpdateBuffer() {
	if (mChanged) {
		mChanged = false;
		// Describe the data we want to copy into the default buffer.
		D3D12_SUBRESOURCE_DATA subResourceData = {};
		subResourceData.pData = mVertices.data();
		subResourceData.RowPitch = (UINT)sizeof(GeometryGenerator::Vertex) * (UINT)mVertices.size();
		subResourceData.SlicePitch = subResourceData.RowPitch;

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