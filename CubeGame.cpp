//***************************************************************************************
// CubeGame.cpp by Frank Luna (C) 2015 All Rights Reserved.
//***************************************************************************************

#include "CubeGame.h"
#include "Raycast.h"

bool GameData::sRunning = true;
const int worldWidthLength = 10;
const int worldHeight = 1;
const int numOfCubes = worldWidthLength * worldWidthLength * worldHeight;

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
	GameData::sRunning = false;
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
	mAllBlocks = std::make_shared<std::vector<std::shared_ptr<Block>>>();

    // Reset the command list to prep for initialization commands.
    ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

    // Get the increment size of a descriptor in this heap type.  This is hardware specific, 
	// so we have to query this information.
    mCbvSrvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    BuildRootSignature();
    BuildShadersAndInputLayout();

	//Load the fonts
	mUI.InitFont();
	LoadTextures();

	BuildDescriptorHeaps();
    BuildShapeGeometry();
	BuildMaterials();
    BuildRenderItems();
    BuildFrameResources();
    BuildPSOs();

	//Initialise the camera
	mPlayer->GetCam()->SetLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	mPlayer->GetCam()->SetPosition(1.0f, 7.0f, 1.0f);

	//Initialise the user interface
	mUI.SetRenderItem(mRitemLayer[(int)RenderLayer::Transparent].at(0));
	mUI.UpdateAspectRatio(mPlayer->GetCam()->GetNearWindowWidth(), mPlayer->GetCam()->GetNearWindowHeight());
	mUI.UpdateRotation(0.0f, 0.0f, mPlayer->GetCam()->GetLook());
	mUI.UpdateUIPos(mPlayer->GetCam()->GetPosition());

	SetUIString("Player X: ", 0, 0);

    // Execute the initialization commands.
    ThrowIfFailed(mCommandList->Close());
    ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
    mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    // Wait until initialization is complete.
    FlushCommandQueue();

    return true;
}

void CubeGame::SetUIString(std::string str, int lineNo, int col) {
	if (lineNo > mUIRows) lineNo = mUIRows;
	if (col > mUICols) col = mUICols;
	float row = (float)lineNo * 1 / mUIRows;
	float colm = (float) col * 1 / mUICols;

	mUI.SetString(mCommandList.Get(), str, colm, row);
}
 
void CubeGame::MakeTexture(std::string name, std::string path) {
	auto tex = std::make_unique<Texture>();
	tex->Name = name;
	tex->Filename = GameData::StringToWString(path);
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), tex->Filename.c_str(),
		tex->Resource, tex->UploadHeap));

	mTextures[tex->Name] = std::move(tex);
}
void CubeGame::MakeTexture(std::string name, std::wstring path) {
	auto tex = std::make_unique<Texture>();
	tex->Name = name;
	tex->Filename = path;
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), tex->Filename.c_str(),
		tex->Resource, tex->UploadHeap));

	mTextures[tex->Name] = std::move(tex);
}

void CubeGame::LoadTextures() {

	MakeTexture("font", mUI.GetFont()->filePath);
	MakeTexture("blocks", L"data/blockMap.dds");
	MakeTexture("skyTex", L"data/sky.dds");
	MakeTexture("blockSelect", L"data/blockBreakMap.dds");

	SplitTextureMapIntoPositions(mBlockTexturePositions, mBlockTexSize, mBlockTexRows, mBlockTexCols, mBlockTexNames);
	SplitTextureMapIntoPositions(mBlockBreakTexturePositions, mBlockTexSize + 2, 1, 7, mBlockBreakTexNames);
}

void CubeGame::SplitTextureMapIntoPositions(std::unordered_map<std::string, DirectX::XMFLOAT2>& out, const int texSize, const int rows, const int cols, const std::string texNames[]) {
	int row = 0;
	int col = 0;

	float sizeX = 1.f / (float)cols;
	float sizeY = 1.f / (float)rows;

	for (int i = 0; i <= (rows * cols) - 1; i++) {

		DirectX::XMFLOAT2 pos = { col * sizeX, row * sizeY };
		out[texNames[i]] = pos;

		col++;
		if (col > cols) {
			col = 0;
			row++;
		}
	}
}

void CubeGame::OnResize()
{
    D3DApp::OnResize();

    // The window resized, so update the aspect ratio and recompute the projection matrix.
	//If the player has been set
	if (mPlayer) {
		mPlayer->GetCam()->SetLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, mBackPlane);
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

		// Should be put in the player VVV
 		mUI.UpdateUIPos(mPlayer->GetCam()->GetPosition());

		//Set render items to dirty if their GO has been updated



		SetUIString("Player X: ", 0, 0);

		for (int i = 0; i < mAllGObjs->size(); i++) {
			if (mAllGObjs->at(i)->GetDirty()) 
				mAllGObjs->at(i)->SetRIDirty();
		}
		if (mUI.GetDirty()) mUI.SetRIDirty();


		//SetUIString("Player Y: ", 1, 0);
		//SetUIString("Player Z: ", 2, 0);
		//SetUIString("Raycast: ", 3, 0);
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

	DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Opaque]);

	mCommandList->SetPipelineState(mPSOs["sky"].Get());
	DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Sky]);

	mCommandList->SetPipelineState(mPSOs["transparent"].Get());
	DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)RenderLayer::Transparent]);


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

		mPlayer->Pitch(dy);
		mPlayer->RotateY(dx);
		mUI.UpdateRotation(dx, dy, mPlayer->GetCam()->GetLook());

		/*std::shared_ptr<Block> block = Raycast::GetFirstBlockInRay(mAllBlocks, mPlayer->GetCam()->GetPosition(), mPlayer->GetCam()->GetLook());
		if (block != nullptr) {
			block->deactivate();
		}*/

    }

    mLastMousePos.x = x;
    mLastMousePos.y = y;
}
 
void CubeGame::OnKeyboardInput(const GameTimer& gt)
{
	const float dt = gt.DeltaTime();

	if (GetAsyncKeyState('W') & 0x8000)
		mPlayer->Walk(5.0f, dt);

	if (GetAsyncKeyState('S') & 0x8000)
		mPlayer->Walk(-5.0f, dt);

	if (GetAsyncKeyState('A') & 0x8000)
		mPlayer->Strafe(-5.0f, dt);

	if (GetAsyncKeyState('D') & 0x8000)
		mPlayer->Strafe(5.0f, dt);

	if (GetAsyncKeyState(VK_SPACE) & 0x8000)
		mPlayer->Jump();

	if (GetAsyncKeyState('E') & 0x8000) {
		std::shared_ptr<Block> block = Raycast::GetFirstBlockInRay(mAllBlocks, mPlayer->GetCam()->GetPosition(), mPlayer->GetCam()->GetLook());
		if (block != nullptr) {
			block->SetActive(false);
		}
	}
	if (GetAsyncKeyState('F') & 0x8000) {
		std::shared_ptr<Block> block = Raycast::GetBlockInfrontFirstBlockInRay(mAllBlocks, mPlayer->GetCam()->GetPosition(), mPlayer->GetCam()->GetLook());
		if (block != nullptr) {
			block->SetActive(true);
		}
	}

	mPlayer->GetCam()->UpdateViewMatrix();
}


void CubeGame::AnimateMaterials(const GameTimer& gt)
{
	
}

void CubeGame::UpdateObjectCBs(const GameTimer& gt)
{
	auto currObjectCB = mCurrFrameResource->ObjectCB.get();
	
	for (int j = 0; j < (int)RenderLayer::Count; j++) {
		for (int i = 0; i < mRitemLayer[j].size(); i++) {
			if (mRitemLayer[j].at(i)->NumFramesDirty > 0) {
				XMMATRIX world = XMLoadFloat4x4(&mRitemLayer[j].at(i)->World);
				XMMATRIX texTransform = XMLoadFloat4x4(&mRitemLayer[j].at(i)->TexTransform);

				ObjectConstants objConstants;
				XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
				XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));

				currObjectCB->CopyData(mRitemLayer[j].at(i)->ObjCBIndex, objConstants);

				// Next FrameResource need to be updated too.
				mRitemLayer[j].at(i)->NumFramesDirty--;
			}
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
			XMMATRIX matTransformTop = XMLoadFloat4x4(&mat->MatTransformTop);
			XMMATRIX matTransformBottom = XMLoadFloat4x4(&mat->MatTransformBottom);

			MaterialConstants matConstants;
			matConstants.DiffuseAlbedo = mat->DiffuseAlbedo;
			matConstants.FresnelR0 = mat->FresnelR0;
			matConstants.Roughness = mat->Roughness;
			XMStoreFloat4x4(&matConstants.MatTransform, XMMatrixTranspose(matTransform));
			XMStoreFloat4x4(&matConstants.MatTransformTop, XMMatrixTranspose(matTransformTop));
			XMStoreFloat4x4(&matConstants.MatTransformBottom, XMMatrixTranspose(matTransformBottom));

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
	mMainPassCB.FarZ = mBackPlane;
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
	slotRootParameter[0].InitAsConstantBufferView(0);	//Object
	slotRootParameter[1].InitAsConstantBufferView(1);	//Material
	slotRootParameter[2].InitAsConstantBufferView(2);	//MainPass
	slotRootParameter[3].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);	//Texture

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
	mShaders["SkyVS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", nullptr, "SkyVS", "vs_5_1");
	mShaders["SkyPS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", nullptr, "SkyPS", "ps_5_1");

	
    mInputLayout =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };
}

void CubeGame::BuildDescriptorHeaps() {
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 4;	//Number of textures
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

	hDescriptor.Offset(1, mCbvSrvDescriptorSize);

	auto blockTex = mTextures["blocks"]->Resource;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = blockTex->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = blockTex->GetDesc().MipLevels;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	md3dDevice->CreateShaderResourceView(blockTex.Get(), &srvDesc, hDescriptor);

	hDescriptor.Offset(1, mCbvSrvDescriptorSize);

	auto skyTex = mTextures["skyTex"]->Resource;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = skyTex->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = skyTex->GetDesc().MipLevels;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	md3dDevice->CreateShaderResourceView(skyTex.Get(), &srvDesc, hDescriptor);

	hDescriptor.Offset(1, mCbvSrvDescriptorSize);

	auto blockBreakTex = mTextures["blockSelect"]->Resource;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = blockBreakTex->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = blockBreakTex->GetDesc().MipLevels;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	md3dDevice->CreateShaderResourceView(blockBreakTex.Get(), &srvDesc, hDescriptor);


}

void CubeGame::BuildShapeGeometry()
{
	GeometryGenerator geoGen;

	const int numb = 3;
	std::string geoHolderNames[numb] = { "shapeGeo", "uiGeo", "skyGeo" };
	std::vector<GeometryGenerator::MeshData> meshDatas[numb];
	std::vector<std::string> meshNames[numb];

	//Shape Geos
	meshDatas[0].push_back(geoGen.CreateBox(0.5f, 0.75f, 0.5f, 0));
	meshNames[0].push_back("player");
	meshDatas[0].push_back(geoGen.CreateBox(1.0f, 1.0f, 1.0f, 0));
	meshNames[0].push_back("cube");
	meshDatas[0].push_back(geoGen.CreateBox(1.1f, 1.1f, 1.1f, 0));
	meshNames[0].push_back("blockSelector");

	//UI Geos
	meshDatas[1].push_back(mUI.CreateUIPlane(1.f, 1.f, mUIRows, mUICols));
	meshNames[1].push_back("mainGUI");

	//Sky
	meshDatas[2].push_back(geoGen.CreateSphere(0.5f, 20, 20));
	meshNames[2].push_back("sky");

	for (int md = 0; md < numb; md++) {
		//Get the total number of vertices
		size_t totalVertexCount = 0;
		for each (GeometryGenerator::MeshData mds in meshDatas[md]) {
			totalVertexCount += mds.Vertices.size();
		}

		//Get a vector of each vertex
		std::vector<Vertex> vertices(totalVertexCount);
		UINT k = 0;
		int vert = 0;
		int face = 0;
		const float width = mBlockTexturePositions["dirt"].x;
		for each (GeometryGenerator::MeshData mds in meshDatas[md]) {
			for (size_t i = 0; i < mds.Vertices.size(); ++i, ++k) {
				vertices[k].Pos = mds.Vertices[i].Position;
				vertices[k].Normal = mds.Vertices[i].Normal;

				if (md == 1) vertices[k].TexC = { 0.1f, 0.1f }; //UI transparent section
				else { //Block coords
					//Face: 0 - camSide, 1 - back, 2 - top, 3 - bottom, 4 - left, 5 - right
					if (face != 1) {
						//Sets the texture coordinates of each vertex in a face
						if (vert == 0)
							vertices[k].TexC = { 0.f, 1.f };
						else if (vert == 1)
							vertices[k].TexC = { 0.f, 0.f };
						else if (vert == 2)
							vertices[k].TexC = { width, 0.f };
						else
							vertices[k].TexC = { width, 1.f };
					}
					else {	//Because the backside's verticies are in clockwise, they are set differently
						if (vert == 1)
							vertices[k].TexC = { 0.f, 1.f };
						else if (vert == 2)
							vertices[k].TexC = { 0.f, 0.f };
						else if (vert == 3)
							vertices[k].TexC = { width, 0.f };
						else
							vertices[k].TexC = { width, 1.f };
					}

					vert++;
					if (vert >= 4) {
						vert = 0;
						face++;
						if (face >= 6) face = 0;
					}
					
				}
			}
		}

		//Get a vector of each index
		std::vector<std::uint16_t> indices;
		for each (GeometryGenerator::MeshData mds in meshDatas[md]) {
			indices.insert(indices.end(), std::begin(mds.GetIndices16()), std::end(mds.GetIndices16()));
		}

		//Get the total byte size of each vector
		const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
		const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

		//Make a MeshGeometry to hold all the data
		auto geo = std::make_unique<MeshGeometry>();
		geo->Name = geoHolderNames[md];

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
		for (int i = 0; i < meshDatas[md].size(); i++) {
			SubmeshGeometry temp;
			temp.IndexCount = (UINT)meshDatas[md].at(i).Indices32.size();
			temp.StartIndexLocation = indexOffset;
			temp.BaseVertexLocation = vertexOffset;

			geo->DrawArgs[meshNames[md].at(i)] = temp;

			indexOffset += (UINT)meshDatas[md].at(i).Indices32.size();
			vertexOffset += (UINT)meshDatas[md].at(i).Vertices.size();
		}

		mGeometries[geo->Name] = std::move(geo);
	}
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


	D3D12_GRAPHICS_PIPELINE_STATE_DESC skyPsoDesc = opaquePsoDesc;
	// The camera is inside the sky sphere, so just turnoff culling.
	skyPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	// Make sure the depth function is LESS_EQUAL and not just LESS.
	// Otherwise, the normalized depth values at z = 1 (NDC)will
	// fail the depth test if the depth buffer was cleared to 1.
	skyPsoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	skyPsoDesc.pRootSignature = mRootSignature.Get();
	skyPsoDesc.VS =
	{
	reinterpret_cast<BYTE*>(mShaders["SkyVS"] -> GetBufferPointer()), mShaders["SkyVS"]->GetBufferSize()
	};
	skyPsoDesc.PS =
	{
	reinterpret_cast<BYTE*>(mShaders["SkyPS"] -> GetBufferPointer()), mShaders["SkyPS"]->GetBufferSize()
	};
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(
		&skyPsoDesc, IID_PPV_ARGS(&mPSOs["sky"])));

}

void CubeGame::BuildFrameResources()
{
    for(int i = 0; i < GameData::sNumFrameResources; ++i)
    {
        mFrameResources.push_back(std::make_unique<FrameResource>(md3dDevice.Get(),
            1, (UINT)mAllGObjs->size() + (UINT)mRitemLayer[(int)RenderLayer::Transparent].size() + (UINT)mRitemLayer[(int)RenderLayer::Sky].size(), (UINT)mMaterials.size()));
    }
}

void CubeGame::BuildMaterials()
{
	float x = mBlockTexturePositions["dirt"].x;
	float x2 = mBlockBreakTexturePositions["b0"].x;

	CreateMaterial("player", 1, DirectX::Colors::Black, { 0,0 });
	CreateMaterial("dirt", 1, {0.4311f, 0.1955f, 0.1288f, 1.f }, { x,0 });
	CreateMaterial("grass", 1, { 0.4311f, 0.1955f, 0.1288f, 1.f }, { x * 2.f,0 }, { x * 3.f,0 }, { x,0 });

	CreateMaterial("sky", 2, { 1.0f, 1.0f, 1.0f }, { 0.f, 0.f });
	CreateMaterial("font", 0, { 1.0f, 1.0f, 1.0f }, { 0.f, 0.f });
	CreateMaterial("blockSelect", 3, { 1.0f, 1.0f, 1.0f }, { x2, 0.f });

}

void CubeGame::CreateMaterial(std::string name, int textureIndex, DirectX::XMVECTORF32 color, DirectX::XMFLOAT2 texTransform) {
	auto mat = std::make_unique<Material>();
	mat->Name = name;
	mat->MatCBIndex = mMaterials.size();
	mat->DiffuseSrvHeapIndex = textureIndex;
	mat->DiffuseAlbedo = XMFLOAT4(color);
	mat->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	mat->Roughness = 0.2f;
	DirectX::XMStoreFloat4x4(&mat->MatTransform, DirectX::XMMatrixTranslation(texTransform.x, texTransform.y, 0.f));

	mMaterials[name] = std::move(mat);
}

void CubeGame::CreateMaterial(std::string name, int textureIndex, DirectX::XMVECTORF32 color, DirectX::XMFLOAT2 texTransform, DirectX::XMFLOAT2 texTransformTop, DirectX::XMFLOAT2 texTransformBottom) {
	CreateMaterial(name, textureIndex, color, texTransform);
	DirectX::XMStoreFloat4x4(&mMaterials[name]->MatTransformTop, DirectX::XMMatrixTranslation(texTransformTop.x, texTransformTop.y, 0.f));
	DirectX::XMStoreFloat4x4(&mMaterials[name]->MatTransformBottom, DirectX::XMMatrixTranslation(texTransformBottom.x, texTransformBottom.y, 0.f));
}

void CubeGame::BuildRenderItems()
{
	auto geo = mGeometries["shapeGeo"].get();

	//Player
	auto playerRI = std::make_shared<RenderItem>(geo, "player", mMaterials["player"].get(), XMMatrixTranslation(1.0f, 6.0f, 1.0f));	//Make a render item
	//mAllGObjs->push_back(std::make_shared<GameObject>(mAllGObjs, playerRI));	//Make a gameobject from the RI and add it to the list
	mPlayer = std::make_shared<Player>(std::make_shared<GameObject>(mAllGObjs, playerRI));						//Make the Player
	mAllGObjs->push_back(mPlayer);
	mAllEnts->push_back(mPlayer);												//Add the player to the enities list
	mRitemLayer[(int)RenderLayer::Opaque].push_back(playerRI);					//Add the players render item to the opaque list

	//Blocks
	for (int worldX = 0; worldX < worldWidthLength; ++worldX)
	{
		for (int worldY = 0; worldY < worldHeight; ++worldY)
		{
			for (int worldZ = 0; worldZ < worldWidthLength; ++worldZ)
			{
				auto temp = std::make_shared<RenderItem>(geo, "cube", mMaterials["grass"].get(), XMMatrixTranslation(1.0f * worldX, 1.0f * worldY, 1.0f * worldZ));
				auto tempGO = std::make_shared<GameObject>(mAllGObjs, temp);
				mAllGObjs->push_back(tempGO);
				mAllBlocks->push_back(std::make_shared<Block>(tempGO));

				//Add the blocks render item to the opaque items list
				mRitemLayer[(int)RenderLayer::Opaque].push_back(temp);
			}
		}
	}


	//DEBUG

	//auto temp1 = std::make_shared<RenderItem>(geo, "cube", mMaterials["grass"].get(), XMMatrixTranslation(3.0f, 2.0f, 3.0f));
	//auto tempGO1 = std::make_shared<GameObject>(mAllGObjs, temp1);
	//mAllGObjs->push_back(tempGO1);
	//mAllBlocks->push_back(std::make_shared<Block>(tempGO1));

	////Add the blocks render item to the opaque items list
	//mRitemLayer[(int)RenderLayer::Opaque].push_back(temp1);

	//auto temp2 = std::make_shared<RenderItem>(geo, "cube", mMaterials["grass"].get(), XMMatrixTranslation(3.0f, 1.0f, 3.0f));
	//auto tempGO2 = std::make_shared<GameObject>(mAllGObjs, temp2);
	//mAllGObjs->push_back(tempGO2);
	//mAllBlocks->push_back(std::make_shared<Block>(tempGO2));

	////Add the blocks render item to the opaque items list
	//mRitemLayer[(int)RenderLayer::Opaque].push_back(temp2);

	//auto temp3 = std::make_shared<RenderItem>(geo, "cube", mMaterials["grass"].get(), XMMatrixTranslation(3.0f, 2.0f, 4.0f));
	//auto tempGO3 = std::make_shared<GameObject>(mAllGObjs, temp3);
	//mAllGObjs->push_back(tempGO3);
	//mAllBlocks->push_back(std::make_shared<Block>(tempGO3));

	////Add the blocks render item to the opaque items list
	//mRitemLayer[(int)RenderLayer::Opaque].push_back(temp3);

	//DEBUG


	//Sky----------------------------
	
	auto sky = mGeometries["skyGeo"].get();

	auto skyRI = std::make_shared<RenderItem>(sky, "sky", mMaterials["sky"].get(), XMMatrixScaling(5000.0f, 5000.0f, 5000.0f));
	mRitemLayer[(int)RenderLayer::Sky].push_back(skyRI);


	//UI---------------------------
	auto ui = mGeometries["uiGeo"].get();

	auto gui1 = std::make_shared<RenderItem>(ui, "mainGUI", mMaterials["font"].get(), XMMatrixIdentity());
	gui1->active = false;
	mRitemLayer[(int)RenderLayer::Transparent].push_back(gui1);

	//Block selector
	auto selectorRI = std::make_shared<RenderItem>(geo, "blockSelector", mMaterials["blockSelect"].get(), XMMatrixTranslation(0.f, 0.f, 0.f));
	mBlockSelector = std::make_shared<GameObject>(mAllGObjs, selectorRI);
	mRitemLayer[(int)RenderLayer::Transparent].push_back(selectorRI);



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
		//if (ritems[i]->GetActive()) {
		if(ritems[i]->active){
			auto ri = ritems[i];

			cmdList->IASetVertexBuffers(0, 1, &ri->Geo->VertexBufferView());
			cmdList->IASetIndexBuffer(&ri->Geo->IndexBufferView());
			cmdList->IASetPrimitiveTopology(ri->PrimitiveType);

			D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
			cmdList->SetGraphicsRootConstantBufferView((UINT)0, objCBAddress);

			D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * matCBByteSize;
			cmdList->SetGraphicsRootConstantBufferView((UINT)1, matCBAddress);

			CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
			tex.Offset(ri->Mat->DiffuseSrvHeapIndex, mCbvSrvUavDescriptorSize);
			mCommandList->SetGraphicsRootDescriptorTable(3, tex);

			cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, 0);
		}
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