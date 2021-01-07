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
	XMStoreFloat4x4(&mRI->World, DirectX::XMMatrixTranslation(-0.02f, 0.02, 0));
	SetDirtyFlag();
}

void UI::InitFont() {
	mFnt.filePath = L"data/font.dds";
	//1 / number of rows or columns
	float size = 1.f / 9.f;

	int rows = 3;
	int cols = 9;
	int row = 0;
	int col = 0;

	//Capitals
	for (int i = 65; i <= 65 + 25; i++) {
		char c = (char)i;

		Font::myChar temp(col * size, row * size, size, size);
		mFnt.chars[c] = temp;

		col++;
		if (col >= cols) {
			col = 0;
			row++;
		}
	}

	//Lower case
	row = 3;
	col = 0;
	for (int i = 97; i <= 97 + 25; i++) {
		char c = (char)i;

		Font::myChar temp(col * size, row * size, size, size);
		mFnt.chars[c] = temp;

		col++;
		if (col >= cols) {
			col = 0;
			row++;
		}
	}

	//numbers
	row = 6;
	col = 0;
	for (int i = 48; i <= 48 + 9; i++) {
		char c = (char)i;

		Font::myChar temp(col * size, row * size, size, size);
		mFnt.chars[c] = temp;

		col++;
		if (col >= cols) {
			col = 0;
			row++;
		}
	}

	//Specials
	row = 7;
	col = 1;
	char spec[] = { '.', ',', ';', ':', '$', '#', 'X', '!', '"', '/', '?', '%', '&', '(', ')', '@', ' '  };
	for (int i = 0; i <= 16 + 9; i++) {

		Font::myChar temp(col * size, row * size, size, size);
		mFnt.chars[spec[i]] = temp;

		col++;
		if (col >= cols) {
			col = 0;
			row++;
		}
	}
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
			meshData.Vertices[index].TexC.x = (float)j * du;
			meshData.Vertices[index].TexC.y = (float)i * dv;

			if (j != 0 && j < mSizeY - 1) {
				j++;
				index = i * mSizeY + j;

				meshData.Vertices[index].Pos = DirectX::XMFLOAT3(x, y, 0.0f);
				meshData.Vertices[index].Normal = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);

				// Stretch texture over grid.
				meshData.Vertices[index].TexC.x = (float)(j - 1) * du;
				meshData.Vertices[index].TexC.y = (float)(i - 1) * dv;

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
				meshData.Vertices[index].TexC.x = (float)j * du;
				meshData.Vertices[index].TexC.y = (float)i * dv;

				if (j != 0 && j < mSizeY - 1) {
					j++;
					index = i * mSizeY + j;

					meshData.Vertices[index].Pos = DirectX::XMFLOAT3(x, y, 0.0f);
					meshData.Vertices[index].Normal = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);

					// Stretch texture over grid.
					meshData.Vertices[index].TexC.x = (float)(j - 1) * du;
					meshData.Vertices[index].TexC.y = (float)(i - 1) * dv;

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

void UI::SetChar(char c, int p, std::vector<GeometryGenerator::Vertex>& vs) {
	//Skip every other position
	int pos = p + p;				//Skip every other colomn
	int row = (p / (mSizeX / 2));	//Calculate the row number
	row *= mSizeX;					//Multiply the row number by the size of each row
	pos += row;						//Add the x and y positions

	//If there aren't enough spaces, don't set the char
	if (pos >= vs.size()) return;

	//Set a square of texture coords
	vs[pos].TexC = { mFnt.chars.at(c).posX, mFnt.chars.at(c).posY };
	vs[pos + 1].TexC = { mFnt.chars.at(c).posX + mFnt.chars.at(c).width, mFnt.chars.at(c).posY };
	vs[pos + mSizeX].TexC = { mFnt.chars.at(c).posX, mFnt.chars.at(c).posY + mFnt.chars.at(c).height };
	vs[pos + mSizeX + 1].TexC = { mFnt.chars.at(c).posX + mFnt.chars.at(c).width, mFnt.chars.at(c).posY + mFnt.chars.at(c).height };
}

void UI::SetString(std::string str, float posX, float posY) {
	//Make sure the position isn't outside plane
	if (posX > 1.f) posX = 1.f;
	if (posY > 1.f) posY = 1.f;

	//Calculate the starting position
	posX *= mSizeX * 0.5f;
	posY *= mSizeY * 0.5f;
	posY *= mSizeX * 0.5f;

 	int pos = (int)(posX + posY);

	//Change the texture coords of each sub-square on the UI plane to match those in the font sprite map
	for each (char c in str) {
		SetChar(c, pos, mVertices);
		pos++;
	}

	//Update the CPUs version of the verticies
	CopyMemory(mRI->Geo->VertexBufferCPU->GetBufferPointer(), mVertices.data(), mVbByteSize);

	//Make sure the render item is updated on each Frame Resource	
	SetDirtyFlag();
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