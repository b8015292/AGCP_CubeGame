#include "UI.h"

void UI::SetRenderItem(std::shared_ptr<RenderItem> ri) {
	mRI = ri;
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
	char spec[] = { '.', ',', ';', ':', '$', '#', 'X', '!', '"', '/', '?', '%', '&', '(', ')', '@'  };
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
GeometryGenerator::MeshData UI::CreateUIPlane(float width, float depth, int oM, int oN) {
	GeometryGenerator::MeshData meshData;

	int m = oM * 2;
	int n = oN * 2;

	mSizeX = (int)m;
	mSizeZ = (int)n;

	int vertexCount = m * n;
	int faceCount = (m - 1) * (n - 1) * 2;


	float halfWidth = 0.5f * width;
	float halfDepth = 0.5f * depth;

	float dx = width / ((float)n - 1); //Vertices per row
	float dz = depth / ((float)m - 1); //Vertices per column

	float du = 1.0f / ((float)n - 1); //Tex coord increment per vertex
	float dv = 1.0f / ((float)m - 1);

	int decI = 0;
	meshData.Vertices.resize(vertexCount);
	for (int i = 0; i < m; ++i)
	{
		float z = halfDepth - (float)(i - decI) * (dz * 2.f);
		int decJ = 0;
		for (int j = 0; j < n; ++j)
		{

			float x = -halfWidth + (float)(j - decJ) * (dx * 2.f);
			size_t index = i * n + j;

			meshData.Vertices[index].Position = DirectX::XMFLOAT3(x, 0.0f, z);
			meshData.Vertices[index].Normal = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
			meshData.Vertices[index].TangentU = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);

			// Stretch texture over grid.
			meshData.Vertices[index].TexC.x = (float)j * du;
			meshData.Vertices[index].TexC.y = (float)i * dv;

			if (j != 0 && j < n - 1) {
				j++;
				index = i * n + j;

				meshData.Vertices[index].Position = DirectX::XMFLOAT3(x, 0.0f, z);
				meshData.Vertices[index].Normal = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
				meshData.Vertices[index].TangentU = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);

				// Stretch texture over grid.
				meshData.Vertices[index].TexC.x = (float)(j - 1) * du;
				meshData.Vertices[index].TexC.y = (float)(i - 1) * dv;

				decJ++;
			}

		}

		if (i != 0 && i < m - 1) {
			i++;

			int decJ = 0;
			for (int j = 0; j < n; ++j)
			{

				float x = -halfWidth + (float)(j - decJ) * (dx * 2);
				size_t index = i * n + j;

				meshData.Vertices[index].Position = DirectX::XMFLOAT3(x, 0.0f, z);
				meshData.Vertices[index].Normal = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
				meshData.Vertices[index].TangentU = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);

				// Stretch texture over grid.
				meshData.Vertices[index].TexC.x = (float)j * du;
				meshData.Vertices[index].TexC.y = (float)i * dv;

				if (j != 0 && j < n - 1) {
					j++;
					index = i * n + j;

					meshData.Vertices[index].Position = DirectX::XMFLOAT3(x, 0.0f, z);
					meshData.Vertices[index].Normal = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
					meshData.Vertices[index].TangentU = DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f);

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
	for (GeometryGenerator::uint32 i = 0; i < (GeometryGenerator::uint32)m - 1; ++i)
	{
		for (GeometryGenerator::uint32 j = 0; j < (GeometryGenerator::uint32)n - 1; ++j)
		{
			meshData.Indices32[k] = i * n + j;
			meshData.Indices32[k + 1] = i * n + j + 1;
			meshData.Indices32[k + 2] = (i + 1) * n + j;

			meshData.Indices32[k + 3] = (i + 1) * n + j;
			meshData.Indices32[k + 4] = i * n + j + 1;
			meshData.Indices32[k + 5] = (i + 1) * n + j + 1;

			k += 6; // next quad
		}
	}

	return meshData;
}
void UI::UpdateUIPos(DirectX::XMVECTOR camPos) {

	////Scale the plane
	//DirectX::XMMATRIX uiTransform = DirectX::XMMatrixScalingFromVector({ mWindowWidth , mWindowHeight , 1.f});
	DirectX::XMMATRIX uiTransform = DirectX::XMMatrixIdentity();

	//Move the plane from the origin to the camera
	DirectX::XMVECTOR invCamPos = { -camPos.m128_f32[0], -camPos.m128_f32[1], -camPos.m128_f32[2] };
	uiTransform = DirectX::XMMatrixMultiply(uiTransform, DirectX::XMMatrixTranslationFromVector(invCamPos));

	//Rotate the plane
	uiTransform = DirectX::XMMatrixMultiply(DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(-90.f)),
		DirectX::XMMatrixRotationX(mRotationY));
	uiTransform = DirectX::XMMatrixMultiply(uiTransform, DirectX::XMMatrixRotationY(mRotationX));

	//Translate the plane to be just infront of the camera
	float offset = 1.2f;
	DirectX::XMVECTOR translate = {camPos.m128_f32[0] + mLook.m128_f32[0] * offset, camPos.m128_f32[1] + mLook.m128_f32[1] * offset, camPos.m128_f32[2] + mLook.m128_f32[2] * offset };
	uiTransform = XMMatrixMultiply(uiTransform, DirectX::XMMatrixTranslationFromVector(translate));

	//Apply the matrix to the UI plane
	XMStoreFloat4x4(&mRI->World, uiTransform);
	mRI->NumFramesDirty++;
}

void UI::UpdateAspectRatio(float camNearWindowWidth, float camNearWindowHeight) {
	mWindowWidth = camNearWindowWidth;
	mWindowHeight = camNearWindowHeight;
}

void UI::UpdateRotation(float rotX, float rotY, DirectX::XMVECTOR look) {
	mRotationX += rotX;
	mRotationY += rotY;
	mLook = look;
}

void UI::SetChar(char c, int p, std::vector<Vertex>& vs) {
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

void UI::SetString(ID3D12GraphicsCommandList* cmdList, std::string str, float posX, float posY) {
	//Define constants
	const UINT vertsPerObj = mSizeX * mSizeZ;
	const UINT vbByteSize = vertsPerObj * sizeof(Vertex);

	//Where the verticies for this item start in the buffer
	const int vertStart = mRI->BaseVertexLocation;

	//A pointer to the buffer of vertices
	ComPtr<ID3DBlob> verticesBlob = mRI->Geo->VertexBufferCPU;

	//Move the data from the buffer onto a vector we can view/manipulate
	std::vector<Vertex> vs(vertsPerObj);
	CopyMemory(vs.data(), verticesBlob->GetBufferPointer(), vbByteSize);

	//Make sure the position isn't outside plane
	if (posX > 1.f) posX = 1.f;
	if (posY > 1.f) posY = 1.f;

	//Calculate the starting position
	posX *= mSizeX * 0.5f;
	posY *= mSizeZ * 0.5f;

	posY *= mSizeX * 0.5f;

 	int pos = (int)(posX + posY);

	//Change the texture coords of each sub-square on the UI plane to match those in the font sprite map
	for each (char c in str) {
		SetChar(c, pos, vs);
		pos++;
	}

	//Update the CPUs version of the verticies
	CopyMemory(verticesBlob->GetBufferPointer(), vs.data(), vbByteSize);

	//Update the GPUs version of the verticies
	UpdateBuffer(cmdList, vs);
}

void UI::UpdateBuffer(ID3D12GraphicsCommandList* cmdList, std::vector<Vertex> vertices) {
	// Describe the data we want to copy into the default buffer.
	D3D12_SUBRESOURCE_DATA subResourceData = {};
	subResourceData.pData = vertices.data();
	subResourceData.RowPitch = (UINT)sizeof(Vertex) * (UINT)vertices.size();
	subResourceData.SlicePitch = subResourceData.RowPitch;

	// Schedule to copy the data to the default buffer resource.  At a high level, the helper function UpdateSubresources
	// will copy the CPU memory into the intermediate upload heap.  Then, using ID3D12CommandList::CopySubresourceRegion,
	// the intermediate upload heap data will be copied to mBuffer.
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mRI->Geo->VertexBufferGPU.Get(),
		D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST));

	UpdateSubresources<1>(cmdList, mRI->Geo->VertexBufferGPU.Get(), mRI->Geo->VertexBufferUploader.Get(), 0, 0, 1, &subResourceData);

	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(mRI->Geo->VertexBufferGPU.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));
}

//void UI::DrawUI(ID3D12GraphicsCommandList* cmdList, const std::vector<std::shared_ptr<RenderItem>> ritems) {
//
//}