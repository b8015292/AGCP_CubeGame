//***************************************************************************************
// CubeGame.cpp by Frank Luna (C) 2015 All Rights Reserved.
//***************************************************************************************

#include "CubeGame.h"

bool GameData::sRunning = true;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
    PSTR cmdLine, int showCmd)
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    try
    {
        CubeGame theApp(hInstance);
        if(!theApp.Initialize())
            return 0;

        return theApp.Run();
    }
    catch(DxException& e)
    {
        MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
        return 0;
    }
}

CubeGame::CubeGame(HINSTANCE hInstance)
    : D3DApp(hInstance)
{
}

CubeGame::~CubeGame()
{
	//Because each gameobject points to the list of gameobjects, there's an infinate loop of pointing.
	//To prevent a data leak, the pointers a manually destroyed
	for (int i = 0; i < mAllEnts->size(); i++) {
		mAllEnts->at(i).~shared_ptr(); 
	}
	for (int i = 0; i < mAllGObjs->size(); i++) {
		mAllGObjs->at(i).~shared_ptr();
	}

    if(md3dDevice != nullptr)
        FlushCommandQueue();
}

bool CubeGame::Initialize()
{
    if(!D3DApp::Initialize())
        return false;

	GameData::sRunning = true;
	mAllGObjs = std::make_shared<std::vector<std::shared_ptr<GameObject>>>();
	mAllEnts = std::make_shared<std::vector<std::shared_ptr<Entity>>>();

    // Reset the command list to prep for initialization commands.
    ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

    // Get the increment size of a descriptor in this heap type.  This is hardware specific, 
	// so we have to query this information.
    mCbvSrvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    BuildRootSignature();
    BuildShadersAndInputLayout();

	mUI.InitFont();
	LoadTextures();
	BuildDescriptorHeaps();

    BuildShapeGeometry();
	BuildMaterials();
    BuildRenderItems();
    BuildFrameResources();
    BuildPSOs();

	mPlayer->GetCam()->SetLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	mPlayer->GetCam()->SetPosition(0.0f, 2.0f, -15.0f);

	mUI.SetRenderItem(mRitemLayer[(int)RenderLayer::Transparent].at(0));
	mUI.UpdateAspectRatio(mPlayer->GetCam()->GetNearWindowWidth(), mPlayer->GetCam()->GetNearWindowHeight());
	mUI.UpdateRotation(0.0f, 0.0f, mPlayer->GetCam()->GetLook());
	mUI.UpdateUIPos(mPlayer->GetCam()->GetPosition());

	mUI.SetString(mCommandList.Get(), "Hello", 0.0f, 0.0f);

    // Execute the initialization commands.
    ThrowIfFailed(mCommandList->Close());
    ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
    mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    // Wait until initialization is complete.
    FlushCommandQueue();

    return true;
}
 
void CubeGame::LoadTextures() {
	auto fontTex = std::make_unique<Texture>();
	fontTex->Name = "font";
	fontTex->Filename = mUI.GetFont()->filePath;
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), fontTex->Filename.c_str(),
		fontTex->Resource, fontTex->UploadHeap));

	mTextures[fontTex->Name] = std::move(fontTex);
}

void CubeGame::OnResize()
{
    D3DApp::OnResize();

    // The window resized, so update the aspect ratio and recompute the projection matrix.
	//If the player has been set
	if (mPlayer) {
		mPlayer->GetCam()->SetLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
		mUI.UpdateAspectRatio(mPlayer->GetCam()->GetNearWindowWidth(), mPlayer->GetCam()->GetNearWindowHeight());
	}
}

void CubeGame::Update(const GameTimer& gt)
{
    OnKeyboardInput(gt);

    // Cycle through the circular frame resource array.
    mCurrFrameResourceIndex = (mCurrFrameResourceIndex + 1) % GameData::sNumFrameResources;
    mCurrFrameResource = mFrameResources[mCurrFrameResourceIndex].get();

    // Has the GPU finished processing the commands of the current frame resource?
    // If not, wait until the GPU has completed commands up to this fence point.
    if(mCurrFrameResource->Fence != 0 && mFence->GetCompletedValue() < mCurrFrameResource->Fence)
    {
        HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
        ThrowIfFailed(mFence->SetEventOnCompletion(mCurrFrameResource->Fence, eventHandle));
        WaitForSingleObject(eventHandle, INFINITE);
        CloseHandle(eventHandle);
    }



	if(GameData::sRunning){ 
		for (int i = 0; i < mAllEnts->size(); i++) {
			mAllEnts->at(i)->Update(gt.DeltaTime());
		}

		mUI.UpdateUIPos(mPlayer->GetCam()->GetPosition());
	}



	AnimateMaterials(gt);
	UpdateObjectCBs(gt);
	UpdateMaterialCBs(gt);
	UpdateMainPassCB(gt);
}

void CubeGame::Draw(const GameTimer& gt)
{
    auto cmdListAlloc = mCurrFrameResource->CmdListAlloc;

    // Reuse the memory associated with command recording.
    // We can only reset when the associated command lists have finished execution on the GPU.
    ThrowIfFailed(cmdListAlloc->Reset());

    // A command list can be reset after it has been added to the command queue via ExecuteCommandList.
    // Reusing the command list reuses memory.
	ThrowIfFailed(mCommandList->Reset(cmdListAlloc.Get(), mPSOs["opaque"].Get()));

    mCommandList->RSSetViewports(1, &mScreenViewport);
    mCommandList->RSSetScissorRects(1, &mScissorRect);

    // Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

    // Clear the back buffer and depth buffer.
    mCommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
    mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    // Specify the buffers we are going to render to.
    mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

	//Set Pass Data
	auto passCB = mCurrFrameResource->PassCB->Resource();
	mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

	//Set Font Data
	ID3D12DescriptorHeap* descriptorHeaps[] = { mSrvDescriptorHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
	CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
	mCommandList->SetGraphicsRootDescriptorTable(3, tex);

	DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Opaque]);

	mCommandList->SetPipelineState(mPSOs["transparent"].Get());
    DrawUI(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Transparent]);

    // Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

    // Done recording commands.
    ThrowIfFailed(mCommandList->Close());

    // Add the command list to the queue for execution.
    ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
    mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    // Swap the back and front buffers
    ThrowIfFailed(mSwapChain->Present(0, 0));
	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

    // Advance the fence value to mark commands up to this fence point.
    mCurrFrameResource->Fence = ++mCurrentFence;

    // Add an instruction to the command queue to set a new fence point. 
    // Because we are on the GPU timeline, the new fence point won't be 
    // set until the GPU finishes processing all the commands prior to this Signal().
    mCommandQueue->Signal(mFence.Get(), mCurrentFence);
}

void CubeGame::OnMouseDown(WPARAM btnState, int x, int y)
{
    mLastMousePos.x = x;
    mLastMousePos.y = y;

    SetCapture(mhMainWnd);
}

void CubeGame::OnMouseUp(WPARAM btnState, int x, int y)
{
    ReleaseCapture();
}

void CubeGame::OnMouseMove(WPARAM btnState, int x, int y)
{
    if((btnState & MK_LBUTTON) != 0)
    {
        // Make each pixel correspond to a quarter of a degree.
        float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
        float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

		mPlayer->GetCam()->Pitch(dy);
		mPlayer->GetCam()->RotateY(dx);
		mUI.UpdateRotation(dx, dy, mPlayer->GetCam()->GetLook());

    }

    mLastMousePos.x = x;
    mLastMousePos.y = y;
}
 
void CubeGame::OnKeyboardInput(const GameTimer& gt)
{
	const float dt = gt.DeltaTime();

	if (GetAsyncKeyState('W') & 0x8000)
		mPlayer->GetCam()->Walk(5.0f * dt);

	if (GetAsyncKeyState('S') & 0x8000)
		mPlayer->GetCam()->Walk(-5.0f * dt);

	if (GetAsyncKeyState('A') & 0x8000)
		mPlayer->GetCam()->Strafe(-5.0f * dt);

	if (GetAsyncKeyState('D') & 0x8000)
		mPlayer->GetCam()->Strafe(5.0f * dt);

	if (GetAsyncKeyState('E') & 0x8000) {
		float dist = 0;
		bool intersects = mAllEnts->at(0)->boundingBox.Intersects(mPlayer->GetCam()->GetPosition(), mPlayer->GetCam()->GetLook(), dist);
		if(intersects)
			OutputDebugStringW(L"intersected\n");
		else
			OutputDebugStringW(L"didnt intersect\n");

	}

	mPlayer->GetCam()->UpdateViewMatrix();
}


void CubeGame::AnimateMaterials(const GameTimer& gt)
{
	
}

void CubeGame::UpdateObjectCBs(const GameTimer& gt)
{
	auto currObjectCB = mCurrFrameResource->ObjectCB.get();
	
	for (int i = 0; i < mAllGObjs->size(); i++) {
		if (mAllGObjs->at(i)->mRI->NumFramesDirty > 0) {
			XMMATRIX world = XMLoadFloat4x4(&mAllGObjs->at(i)->mRI->World);
			XMMATRIX texTransform = XMLoadFloat4x4(&mAllGObjs->at(i)->mRI->TexTransform);

			ObjectConstants objConstants;
			XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
			XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));

			currObjectCB->CopyData(mAllGObjs->at(i)->mRI->ObjCBIndex, objConstants);

			// Next FrameResource need to be updated too.
			mAllGObjs->at(i)->mRI->NumFramesDirty--;
		}
	}

	for (int i = 0; i < mRitemLayer[(int)RenderLayer::Transparent].size(); i++) {
		if (mRitemLayer[(int)RenderLayer::Transparent].at(i)->NumFramesDirty > 0) {
			XMMATRIX world = XMLoadFloat4x4(&mRitemLayer[(int)RenderLayer::Transparent].at(i)->World);
			XMMATRIX texTransform = XMLoadFloat4x4(&mRitemLayer[(int)RenderLayer::Transparent].at(i)->TexTransform);

			ObjectConstants objConstants;
			XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
			XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));

			currObjectCB->CopyData(mRitemLayer[(int)RenderLayer::Transparent].at(i)->ObjCBIndex, objConstants);

			// Next FrameResource need to be updated too.
			mRitemLayer[(int)RenderLayer::Transparent].at(i)->NumFramesDirty--;
		}
	}

}

void CubeGame::UpdateMaterialCBs(const GameTimer& gt)
{
	auto currMaterialCB = mCurrFrameResource->MaterialCB.get();
	for(auto& e : mMaterials)
	{
		// Only update the cbuffer data if the constants have changed.  If the cbuffer
		// data changes, it needs to be updated for each FrameResource.
		Material* mat = e.second.get();
		if(mat->NumFramesDirty > 0)
		{
			XMMATRIX matTransform = XMLoadFloat4x4(&mat->MatTransform);

			MaterialConstants matConstants;
			matConstants.DiffuseAlbedo = mat->DiffuseAlbedo;
			matConstants.FresnelR0 = mat->FresnelR0;
			matConstants.Roughness = mat->Roughness;
			XMStoreFloat4x4(&matConstants.MatTransform, XMMatrixTranspose(matTransform));

			currMaterialCB->CopyData(mat->MatCBIndex, matConstants);

			// Next FrameResource need to be updated too.
			mat->NumFramesDirty--;
		}
	}
}

void CubeGame::UpdateMainPassCB(const GameTimer& gt)
{
	XMMATRIX view = mPlayer->GetCam()->GetView();
	XMMATRIX proj = mPlayer->GetCam()->GetProj();

	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

	XMStoreFloat4x4(&mMainPassCB.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&mMainPassCB.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&mMainPassCB.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&mMainPassCB.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&mMainPassCB.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&mMainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
	mMainPassCB.EyePosW = mPlayer->GetCam()->GetPosition3f();
	mMainPassCB.RenderTargetSize = XMFLOAT2((float)mClientWidth, (float)mClientHeight);
	mMainPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / mClientWidth, 1.0f / mClientHeight);
	mMainPassCB.NearZ = 1.0f;
	mMainPassCB.FarZ = 1000.0f;
	mMainPassCB.TotalTime = gt.TotalTime();
	mMainPassCB.DeltaTime = gt.DeltaTime();
	mMainPassCB.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };

	//mMainPassCB.Lights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };
	//mMainPassCB.Lights[0].Strength = { 0.6f, 0.6f, 0.6f };
	//mMainPassCB.Lights[0].FalloffEnd = 10.0f;
	//mMainPassCB.Lights[0].FalloffStart = 1.0f;
	//mMainPassCB.Lights[0].Position = { 0.0f, 5.5f, 0.0f };

	mMainPassCB.Lights[1].Direction = { -0.57735f, -0.57735f, 0.57735f };
	mMainPassCB.Lights[1].Strength = { 0.3f, 0.3f, 0.3f };
	mMainPassCB.Lights[2].Direction = { 0.0f, -0.707f, -0.707f };
	mMainPassCB.Lights[2].Strength = { 0.15f, 0.15f, 0.15f };

	auto currPassCB = mCurrFrameResource->PassCB.get();
	currPassCB->CopyData(0, mMainPassCB);
}

void CubeGame::BuildRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE texTable;
	texTable.Init(
		D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
		1,  // number of descriptors
		0); // register t0

	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[4];

	// Create root CBV.
	slotRootParameter[0].InitAsConstantBufferView(0);
	slotRootParameter[1].InitAsConstantBufferView(1);
	slotRootParameter[2].InitAsConstantBufferView(2);
	slotRootParameter[3].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);

	auto staticSamplers = GetStaticSamplers();


	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(4, slotRootParameter, (UINT)staticSamplers.size(), staticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if(errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(md3dDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(mRootSignature.GetAddressOf())));
}

void CubeGame::BuildShadersAndInputLayout()
{
	const D3D_SHADER_MACRO alphaTestDefines[] =
	{
		"ALPHA_TEST", "1",
		NULL, NULL
	};

	mShaders["standardVS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", nullptr, "VS", "vs_5_1");
	mShaders["opaquePS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", nullptr, "PS", "ps_5_1");
	mShaders["UIPS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", nullptr, "UIPS", "ps_5_1");
	
    mInputLayout =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };
}

void CubeGame::BuildDescriptorHeaps() {
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 1;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mSrvDescriptorHeap)));

	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	auto fontTex = mTextures["font"]->Resource;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = fontTex->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = fontTex->GetDesc().MipLevels;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	md3dDevice->CreateShaderResourceView(fontTex.Get(), &srvDesc, hDescriptor);
	//hDescriptor.Offset(1, mCbvSrvDescriptorSize);
}

void CubeGame::BuildShapeGeometry()
{
    GeometryGenerator geoGen;

	//Create the mesh
	std::vector<GeometryGenerator::MeshData> meshData;

	meshData.push_back(geoGen.CreateBox(0.5f, 0.75f, 0.5f, 0));
	meshData.push_back(geoGen.CreateBox(1.f, 1.f, 1.f, 0));

	//Create a name for each mesh
	std::vector<std::string> meshNames;

	meshNames.push_back("player");
	meshNames.push_back("block");

	//Get the total number of vertices
	size_t totalVertexCount = 0;
	for each (GeometryGenerator::MeshData md in meshData) {
		totalVertexCount += md.Vertices.size();
	}

	//Get a vector of each vertex
	std::vector<Vertex> vertices(totalVertexCount);
	UINT k = 0;
	for each (GeometryGenerator::MeshData md in meshData) {
		for (size_t i = 0; i < md.Vertices.size(); ++i, ++k) {
			vertices[k].Pos = md.Vertices[i].Position;
			vertices[k].Normal = md.Vertices[i].Normal;
			vertices[k].TexC = md.Vertices[i].TexC;
		}
	}

	//Get a vector of each index
	std::vector<std::uint16_t> indices;
	for each (GeometryGenerator::MeshData md in meshData) {
		indices.insert(indices.end(), std::begin(md.GetIndices16()), std::end(md.GetIndices16()));
	}

	//Get the total byte size of each vector
    const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
    const UINT ibByteSize = (UINT)indices.size()  * sizeof(std::uint16_t);

	//Make a MeshGeometry to hold all the data
	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "shapeGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);


	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;


	//Tell MeshGemotry the location of each mesh
	UINT indexOffset = 0;
	UINT vertexOffset = 0;
	for (int i = 0; i < meshData.size(); i++) {
		SubmeshGeometry temp;
		temp.IndexCount = (UINT)meshData.at(i).Indices32.size();
		temp.StartIndexLocation = indexOffset;
		temp.BaseVertexLocation = vertexOffset;

		geo->DrawArgs[meshNames.at(i)] = temp;

		indexOffset += (UINT)meshData.at(i).Indices32.size();
		vertexOffset += (UINT)meshData.at(i).Vertices.size();
	}

	mGeometries[geo->Name] = std::move(geo);


	//UI stuff------------------------------------------------------------------
	std::vector<GeometryGenerator::MeshData>uiData;

	//uiData.push_back(geoGen.CreateGrid(10.f, 10.f, 2, 2));
	uiData.push_back(mUI.CreateUIPlane(1.f, 1.f, 10, 10));

	auto ui = std::make_unique<MeshGeometry>();
	ui->Name = "uiGeo";


	//Get the total number of vertices
	size_t totalVertexCountu = 0;

	for each (GeometryGenerator::MeshData md in uiData) {
		totalVertexCountu += md.Vertices.size();
	}

	//Get a vector of each vertex
	std::vector<Vertex> verticesu(totalVertexCountu);
	UINT l = 0;
	for each (GeometryGenerator::MeshData md in uiData) {
		int rows = 9;
		int cols = 3;
		int row = 0;
		int col = 0;
		float r = (float)rows / 10.f;
		float c = (float)cols / 10.f;

		float val = 0.0001f;

		for (size_t i = 0; i < md.Vertices.size(); ++i, ++l) {
			verticesu[l].Pos = md.Vertices[i].Position;
			verticesu[l].Normal = md.Vertices[i].Normal;
			//verticesu[l].TexC = md.Vertices[i].TexC;
			verticesu[l].TexC = { 0.1f, 0.1f };
		}
	}

	//Get a vector of each index
	std::vector<std::uint16_t> indicesu;
	for each (GeometryGenerator::MeshData md in uiData) {
		indicesu.insert(indicesu.end(), std::begin(md.GetIndices16()), std::end(md.GetIndices16()));
	}

	//Get the total byte size of each vector
	const UINT vbByteSizeu = (UINT)verticesu.size() * sizeof(Vertex);
	const UINT ibByteSizeu = (UINT)indicesu.size() * sizeof(std::uint16_t);

	ThrowIfFailed(D3DCreateBlob(vbByteSizeu, &ui->VertexBufferCPU));
	CopyMemory(ui->VertexBufferCPU->GetBufferPointer(), verticesu.data(), vbByteSizeu);

	ThrowIfFailed(D3DCreateBlob(ibByteSizeu, &ui->IndexBufferCPU));
	CopyMemory(ui->IndexBufferCPU->GetBufferPointer(), indicesu.data(), ibByteSizeu);

	ui->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), verticesu.data(), vbByteSizeu, ui->VertexBufferUploader);

	ui->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indicesu.data(), ibByteSizeu, ui->IndexBufferUploader);


	ui->VertexByteStride = sizeof(Vertex);
	ui->VertexBufferByteSize = vbByteSizeu;
	ui->IndexFormat = DXGI_FORMAT_R16_UINT;
	ui->IndexBufferByteSize = ibByteSizeu;

	//Tell MeshGemotry the location of each mesh
	UINT indexOffsetu = 0;
	UINT vertexOffsetu = 0;
	for (int i = 0; i < uiData.size(); i++) {
		SubmeshGeometry temp;
		temp.IndexCount = (UINT)uiData.at(i).Indices32.size();
		temp.StartIndexLocation = indexOffsetu;
		temp.BaseVertexLocation = vertexOffsetu;

		ui->DrawArgs["tempString"] = temp;

		indexOffsetu += (UINT)uiData.at(i).Indices32.size();
		vertexOffsetu += (UINT)uiData.at(i).Vertices.size();
	}

	mGeometries[ui->Name] = std::move(ui);
}

void CubeGame::BuildPSOs()
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;

	D3D12_RENDER_TARGET_BLEND_DESC rtb;
	rtb.BlendEnable = true;
	rtb.LogicOpEnable = false;
	rtb.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	rtb.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	rtb.BlendOp = D3D12_BLEND_OP_ADD;
	rtb.SrcBlendAlpha = D3D12_BLEND_ONE;
	rtb.DestBlendAlpha = D3D12_BLEND_ZERO;
	rtb.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	rtb.LogicOp = D3D12_LOGIC_OP_NOOP;
	rtb.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	CD3DX12_BLEND_DESC blendDesc;
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;
	blendDesc.RenderTarget[0] = rtb;

    ZeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	opaquePsoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
	opaquePsoDesc.pRootSignature = mRootSignature.Get();
	opaquePsoDesc.VS = 
	{ 
		reinterpret_cast<BYTE*>(mShaders["standardVS"]->GetBufferPointer()), 
		mShaders["standardVS"]->GetBufferSize()
	};
	opaquePsoDesc.PS = 
	{ 
		reinterpret_cast<BYTE*>(mShaders["opaquePS"]->GetBufferPointer()),
		mShaders["opaquePS"]->GetBufferSize()
	};
	opaquePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	opaquePsoDesc.BlendState = blendDesc;
	opaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	opaquePsoDesc.SampleMask = UINT_MAX;
	opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	opaquePsoDesc.NumRenderTargets = 1;
	opaquePsoDesc.RTVFormats[0] = mBackBufferFormat;
	opaquePsoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	opaquePsoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	opaquePsoDesc.DSVFormat = mDepthStencilFormat;
    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&mPSOs["opaque"])));

	D3D12_GRAPHICS_PIPELINE_STATE_DESC transparentPsoDesc = opaquePsoDesc;

	D3D12_RENDER_TARGET_BLEND_DESC transparencyBlendDesc;
	transparencyBlendDesc.BlendEnable = true;
	transparencyBlendDesc.LogicOpEnable = false;
	transparencyBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	transparencyBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	transparencyBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	transparencyBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	transparencyBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	transparencyBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	transparencyBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
	transparencyBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	transparentPsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["UIPS"]->GetBufferPointer()),
		mShaders["UIPS"]->GetBufferSize()
	};

	transparentPsoDesc.BlendState.RenderTarget[0] = transparencyBlendDesc;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&transparentPsoDesc, IID_PPV_ARGS(&mPSOs["transparent"])));

}

void CubeGame::BuildFrameResources()
{
    for(int i = 0; i < GameData::sNumFrameResources; ++i)
    {
        mFrameResources.push_back(std::make_unique<FrameResource>(md3dDevice.Get(),
            1, (UINT)mAllGObjs->size() + (UINT)mRitemLayer[(int)RenderLayer::Transparent].size(), (UINT)mMaterials.size()));
    }
}

void CubeGame::BuildMaterials()
{
	auto bricks0 = std::make_unique<Material>();
	bricks0->Name = "bricks0";
	bricks0->MatCBIndex = 0;
	bricks0->DiffuseSrvHeapIndex = 0;
	bricks0->DiffuseAlbedo = XMFLOAT4(Colors::ForestGreen);
	bricks0->FresnelR0 = XMFLOAT3(0.02f, 0.02f, 0.02f);
	bricks0->Roughness = 0.1f;

	auto stone0 = std::make_unique<Material>();
	stone0->Name = "stone0";
	stone0->MatCBIndex = 1;
	stone0->DiffuseSrvHeapIndex = 1;
	stone0->DiffuseAlbedo = XMFLOAT4(Colors::LightSteelBlue);
	stone0->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	stone0->Roughness = 0.3f;
 
	auto tile0 = std::make_unique<Material>();
	tile0->Name = "tile0";
	tile0->MatCBIndex = 2;
	tile0->DiffuseSrvHeapIndex = 2;
	tile0->DiffuseAlbedo = XMFLOAT4(Colors::LightGray);
	tile0->FresnelR0 = XMFLOAT3(0.02f, 0.02f, 0.02f);
	tile0->Roughness = 0.2f;

	auto skullMat = std::make_unique<Material>();
	skullMat->Name = "skullMat";
	skullMat->MatCBIndex = 3;
	skullMat->DiffuseSrvHeapIndex = 3;
	skullMat->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	skullMat->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	skullMat->Roughness = 0.3f;
	
	mMaterials["bricks0"] = std::move(bricks0);
	mMaterials["stone0"] = std::move(stone0);
	mMaterials["tile0"] = std::move(tile0);
	mMaterials["skullMat"] = std::move(skullMat);
}

void CubeGame::BuildRenderItems()
{
	auto geo = mGeometries["shapeGeo"].get();

	//Player
	auto temp = std::make_shared<RenderItem>();
	XMStoreFloat4x4(&temp->TexTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
	temp->ObjCBIndex = (UINT)0;
	temp->Mat = mMaterials["stone0"].get();
	temp->Geo = geo;
	temp->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	temp->IndexCount = temp->Geo->DrawArgs["player"].IndexCount;
	temp->StartIndexLocation = temp->Geo->DrawArgs["player"].StartIndexLocation;
	temp->BaseVertexLocation = temp->Geo->DrawArgs["player"].BaseVertexLocation;
	auto tempGO = std::make_shared<GameObject>(mAllGObjs);
	tempGO->mRI = temp;
	mAllGObjs->push_back(tempGO);
	auto player = std::make_shared<Player>(mAllGObjs->at(0));
	mPlayer = player;
	mAllEnts->push_back(player);
	XMStoreFloat4x4(&mAllEnts->at(0)->mRI->World, XMMatrixTranslation(0.0f, 7.0f, 0.0f));

	//Blocks
	const float size = 6;
	float row = 0;
	float col = 0;
	for (int i = 0; i < size * size; ) {
		//Initialise render item
		auto temp = std::make_shared<RenderItem>();
		XMStoreFloat4x4(&temp->TexTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
		temp->ObjCBIndex = (UINT)++i;
		temp->Mat = mMaterials["stone0"].get();
		temp->Geo = geo;
		temp->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		temp->IndexCount = temp->Geo->DrawArgs["block"].IndexCount;
		temp->StartIndexLocation = temp->Geo->DrawArgs["block"].StartIndexLocation;
		temp->BaseVertexLocation = temp->Geo->DrawArgs["block"].BaseVertexLocation;

		//Create a game object from the render item
		auto tempGO = std::make_shared<GameObject>(mAllGObjs);
		tempGO->mRI = temp;

		//Position
		XMStoreFloat4x4(&tempGO->mRI->World, XMMatrixTranslation(row, 0.0f, col));
		col++;
		if (col >= size) {
			col = 0;
			row++;
		}

		//Add to the game object list
		mAllGObjs->push_back(tempGO);
	}

	//TEMP
	auto temp2 = std::make_shared<RenderItem>();
	XMStoreFloat4x4(&temp2->TexTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
	temp2->ObjCBIndex = (UINT)size * size + 1;
	temp2->Mat = mMaterials["stone0"].get();
	temp2->Geo = geo;
	temp2->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	temp2->IndexCount = temp2->Geo->DrawArgs["block"].IndexCount;
	temp2->StartIndexLocation = temp2->Geo->DrawArgs["block"].StartIndexLocation;
	temp2->BaseVertexLocation = temp2->Geo->DrawArgs["block"].BaseVertexLocation;
	auto tempGO2 = std::make_shared<GameObject>(mAllGObjs);
	tempGO2->mRI = temp2;
	XMStoreFloat4x4(&tempGO2->mRI->World, XMMatrixTranslation(size-1, 1.f, size-1));
	mAllGObjs->push_back(tempGO2);
	//TEMP END


	for (int i = 0; i < mAllGObjs->size(); i++) {
		mAllGObjs->at(i)->CreateBoundingBox();
	}

	// All the render items are opaque.
	for (int i = 0; i < mAllGObjs->size(); i++) {
		mRitemLayer[(int)RenderLayer::Opaque].push_back(mAllGObjs->at(i)->mRI);
	}	

	//UI---------------------------
	auto ui = mGeometries["uiGeo"].get();


	//Make each mesh a render item
	int j = 0;
	for (std::pair<std::string, SubmeshGeometry> el : ui->DrawArgs) {
		auto temp = std::make_shared<RenderItem>();
		XMStoreFloat4x4(&temp->TexTransform, XMMatrixScaling(1.0f, 1.0f, 1.0f));
		temp->ObjCBIndex = (UINT)mAllGObjs->size() + (UINT)j;
		temp->Mat = mMaterials["stone0"].get();
		temp->Geo = ui;
		temp->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		temp->IndexCount = temp->Geo->DrawArgs[el.first].IndexCount;
		temp->StartIndexLocation = temp->Geo->DrawArgs[el.first].StartIndexLocation;
		temp->BaseVertexLocation = temp->Geo->DrawArgs[el.first].BaseVertexLocation;
		j++;

		mRitemLayer[(int)RenderLayer::Transparent].push_back(temp);
	}
}

void CubeGame::DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<std::shared_ptr<RenderItem>> ritems)
{
    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));
 
	auto objectCB = mCurrFrameResource->ObjectCB->Resource();
	auto matCB = mCurrFrameResource->MaterialCB->Resource();

    // For each render item...
    for(size_t i = 0; i < ritems.size(); ++i)
    {
        auto ri = ritems[i];

        cmdList->IASetVertexBuffers(0, 1, &ri->Geo->VertexBufferView());
        cmdList->IASetIndexBuffer(&ri->Geo->IndexBufferView());
        cmdList->IASetPrimitiveTopology(ri->PrimitiveType);

        D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex*objCBByteSize;
        cmdList->SetGraphicsRootConstantBufferView((UINT)0, objCBAddress);

		D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex*matCBByteSize;
		cmdList->SetGraphicsRootConstantBufferView((UINT)1, matCBAddress);

        cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, 0);
    }
}

void CubeGame::DrawUI(ID3D12GraphicsCommandList* cmdList, const std::vector<std::shared_ptr<RenderItem>> ritems) {
	
	UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
	UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

	auto objectCB = mCurrFrameResource->ObjectCB->Resource();
	auto matCB = mCurrFrameResource->MaterialCB->Resource();

	// For each render item...
	for (size_t i = 0; i < ritems.size(); ++i)
	{
		auto ri = ritems[i];
		
		cmdList->IASetVertexBuffers(0, 1, &ri->Geo->VertexBufferView());
		cmdList->IASetIndexBuffer(&ri->Geo->IndexBufferView());
		cmdList->IASetPrimitiveTopology(ri->PrimitiveType);

		D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
		cmdList->SetGraphicsRootConstantBufferView(0, objCBAddress);

		D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;
		cmdList->SetGraphicsRootConstantBufferView(1, matCBAddress);

		cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, 0);
	}
}

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> CubeGame::GetStaticSamplers()
{
	// Applications usually only need a handful of samplers.  So just define them all up front
	// and keep them available as part of the root signature.  

	const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
		0, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
		1, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
		2, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
		3, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
		4, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
		0.0f,                             // mipLODBias
		8);                               // maxAnisotropy

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
		5, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
		0.0f,                              // mipLODBias
		8);                                // maxAnisotropy

	return {
		pointWrap, pointClamp,
		linearWrap, linearClamp,
		anisotropicWrap, anisotropicClamp };
}
