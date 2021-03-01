//***************************************************************************************
// CubeGame.cpp by Frank Luna (C) 2015 All Rights Reserved.
//***************************************************************************************

#include <ctime>

#include "CubeGame.h"
#include "Raycast.h"
#include "PerlinNoise.h"

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
		//GetDeviceRemovedReason
		
        return 0;
    }
}

CubeGame::CubeGame(HINSTANCE hInstance)
    : D3DApp(hInstance), currentState(GameStates::STARTUP)
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

	mAllUIs = std::make_shared<std::unordered_map<std::string, std::shared_ptr<UI>>>();
	mGeometries = std::make_shared<std::unordered_map<std::string, std::shared_ptr<MeshGeometry>>>();
	//sMaterials = std::make_shared<std::unordered_map<std::string, std::shared_ptr<Material>>>();
	mMaterialIndexes = std::make_shared<std::unordered_map<std::string, int>>();
	for(int i = 0; i < (int)GameData::RenderLayer::Count; i++)
		mRitemLayer[i] = std::make_shared<std::vector<std::shared_ptr<RenderItem>>>();

	mActiveRItems = std::make_shared<std::vector<std::shared_ptr<RenderItem>>>();
	mRitemIntances = std::make_shared<std::vector<std::shared_ptr<RenderItemInstance>>>();

	//std::srand(std::time(nullptr));
	////noise = PerlinNoise(std::rand());

    // Reset the command list to prep for initialization commands.
    ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

    BuildRootSignature();
    BuildShadersAndInputLayout();

	BuildUserInterfaces();
	LoadTextures();

	BuildDescriptorHeaps();
    BuildShapeGeometry();
	BuildMaterials();
	BuildGameObjects();
    BuildFrameResources();
    BuildPSOs();

	//Initialise the camera
	mPlayer->GetCam()->SetLens(0.4f * MathHelper::Pi, AspectRatio(), mFrontPlane, mBackPlane);
	mPlayer->SetPosition(mSpawnPoint);

	//Push the materials into the frame resources
	for (int i = 0; i < GameData::sNumFrameResources; i++) {
		mCurrFrameResource = mFrameResources.at(i).get();
		UpdateMaterialCBs();
	}

    // Execute the initialization commands.
    ThrowIfFailed(mCommandList->Close());
    ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
    mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

    // Wait until initialization is complete.
    FlushCommandQueue();

    return true;
}

void CubeGame::SetUIString(std::string str, int lineNo, int col) {
	if (lineNo > mGUITextRows) lineNo = mGUITextRows;
	if (col > mGUITextCols) col = mGUITextCols;
	float row = (float)lineNo * 1 / mGUITextRows;
	float colm = (float) col * 1 / mGUITextCols;

	mUI_Text->SetString(str, colm, row);
}
 
void CubeGame::BuildUserInterfaces() {
	mUI_Text = std::make_shared<Text>();
	mUI_Text->InitFont();
	std::pair<std::string, std::shared_ptr<Text>> text("Text", mUI_Text);
	mAllUIs->insert(text);

	mUI_Crosshair = std::make_shared<UI>();
	std::pair<std::string, std::shared_ptr<UI>> cross("Crosshair", mUI_Crosshair);
	mAllUIs->insert(cross);

	mUI_Hotbar = std::make_shared<UI>();
	std::pair<std::string, std::shared_ptr<UI>> hotbar("Hotbar", mUI_Hotbar);
	mAllUIs->insert(hotbar);
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

	MakeTexture("tex_font", mUI_Text->GetFont()->filePath);
	MakeTexture("tex_blocks", L"data/blockMap.dds");
	MakeTexture("tex_skyTex", L"data/sky.dds");
	MakeTexture("tex_blockSelect", L"data/blockBreakMap.dds");
	MakeTexture("tex_gui_elements", L"data/guiElements.dds");
	MakeTexture("tex_gui_menus", L"data/guiMenus.dds");

	SplitTextureMapIntoPositions(mBlockTexturePositions, mBlockTexSize, mBlockTexRows, mBlockTexCols, mBlockTexNames);
	SplitTextureMapIntoPositions(mBlockBreakTexturePositions, mBlockTexSize + 2, 1, 7, mBlockBreakTexNames);
	SplitTextureMapIntoPositions(mGUIElementTexturePositions, mGUIElTexSize, mGUIElTexRows, mGUIElTexCols, mGUIElTexNames);


	mGUIElementTexturePositionsAndSizes["hotbar"] = {0.f, 0.f, 1.f, 33.f/mGUIMenuFileSize.y};

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
		mPlayer->GetCam()->SetLens(0.4f * MathHelper::Pi, AspectRatio(), mFrontPlane, mBackPlane);
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
	if (mCurrFrameResource->Fence != 0 && mFence->GetCompletedValue() < mCurrFrameResource->Fence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		ThrowIfFailed(mFence->SetEventOnCompletion(mCurrFrameResource->Fence, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}

	if (mCursorInUse != mCursorInUsePrev) {
		mCursorInUsePrev = mCursorInUse;
		ShowCursor(!mCursorInUse);

		if(!mCursorInUse)
			ReleaseCapture();
		else
			SetCapture(mhMainWnd);
	}

	//Deciding what to update depending on the current game state

	switch (currentState)
	{
	case GameStates::PLAYGAME:

		if (GameData::sRunning) {
			//Update entities and set dirty flag
			for (int i = 0; i < Entity::sAllEntities->size(); i++) {
				Entity::sAllEntities->at(i)->Update(gt.DeltaTime());
			}

			//Hanlde mouse input - Place and destroy blocks
			if (mLeftMouseDown) {
				if (mPreviousSelectedBlock != nullptr) {
					MineSelectedBlock(gt.DeltaTime());
				}
			}
			if (mRightMouseDown) {
				mRightMouseDownTimer += gt.DeltaTime();
				if (mRightMouseDownTimer >= mRightMouseDownTimerMax) {
					mRightMouseDownTimer = 0.f;

					std::shared_ptr<Block> block = Raycast::GetBlockInfrontFirstBlockInRay(Block::sAllBlocks, mPlayer->GetCam()->GetPosition(), mPlayer->GetCam()->GetLook());
					if (block != nullptr) {
						block->SetActive(true);
						mPlayerChangedView = true;
					}
				}
			}

			//If the player's moved update  the block selector
			if (mPlayerChangedView) {
				UpdateBlockSelector();
				mPlayerChangedView = false;
				//DEBUG
				ShowDebug();
			}

			if (mPlayer->GetUpdateWorldPos()) {
				mWorldMgr.UpdatePlayerPosition(mPlayer->GetBoundingBox().Center);
			}

			//Update the UI
			std::unordered_map<std::string, std::shared_ptr<UI>>::iterator it = mAllUIs->begin();
			while (it != mAllUIs->end()) {
				if (it->second->GetDirty()) {
					it->second->SetRIDirty();
				}
				it++;
			}

			if (mBlockSelector->GetDirty()) {
				mBlockSelector->SetRIDirty();	//Number of frame resources
				mBlockSelector->SetRIDirty();
				mBlockSelector->SetRIDirty();
			}
		}
		break;
	case GameStates::PAUSE:
	{
		//Update the UI
		SetUIString("Press P to leave pause", 10, 0);

		std::unordered_map<std::string, std::shared_ptr<UI>>::iterator it = mAllUIs->begin();
		while (it != mAllUIs->end()) {
			if (it->second->GetDirty()) {
				it->second->SetRIDirty();
			}
			it++;
		}
	}
		break;
	case GameStates::MAINMENU:
		break;
	case GameStates::LOADWORLD:
		break;
	case GameStates::STARTUP:
		//Update the UI
	{
		SetUIString("START GAME", 10, 8);

		std::unordered_map<std::string, std::shared_ptr<UI>>::iterator it = mAllUIs->begin();
		while (it != mAllUIs->end()) {
			if (it->second->GetDirty()) {
				it->second->SetRIDirty();
			}
			it++;
		}
		break;
	}
	}

	//AnimateMaterials(gt);
	UpdateObjectCBs(gt);
	//UpdateMaterialCBs(gt);
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
	ThrowIfFailed(mCommandList->Reset(cmdListAlloc.Get(), mPSOs["pso_main"].Get()));

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

	//Set descriptor heap which holds textures
	ID3D12DescriptorHeap* descriptorHeaps[] = { mSrvDescriptorHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	switch (currentState)
	{
	case GameStates::PLAYGAME:
	case GameStates::PAUSE:
		{
		DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)GameData::RenderLayer::Main]);

		mCommandList->SetPipelineState(mPSOs["pso_userInterface"].Get());
		mUI_Text->UpdateBuffer();
		DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)GameData::RenderLayer::UserInterface]);

		mCommandList->SetPipelineState(mPSOs["pso_sky"].Get());
		DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)GameData::RenderLayer::Sky]);

		//Instance*************
		mCommandList->SetPipelineState(mPSOs["pso_instance"].Get());
		//Set the material buffer
		auto matBuffer = mCurrFrameResource->MaterialCB->Resource();
		mCommandList->SetGraphicsRootShaderResourceView(5, matBuffer->GetGPUVirtualAddress());
		//Set the texture table
		mCommandList->SetGraphicsRootDescriptorTable(3, mSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

		DrawInstanceItems(mCommandList.Get());
		//End of Instance******
		}
		break;
	case GameStates::MAINMENU:
	case GameStates::STARTUP:
	case GameStates::LOADWORLD:

		mCommandList->SetPipelineState(mPSOs["pso_userInterface"].Get());
		mUI_Text->UpdateBuffer();
		DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)GameData::RenderLayer::UserInterface]);

		mCommandList->SetPipelineState(mPSOs["pso_sky"].Get());
		DrawRenderItems(mCommandList.Get(), mRitemLayer[(int)GameData::RenderLayer::Sky]);

		break;
	}

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

	if ((btnState & MK_LBUTTON) != 0) {
		mLeftMouseDown = true;
		mCursorInUse = true;
	}

	if ((btnState & MK_RBUTTON) != 0) {
		mRightMouseDown = true;
		mCursorInUse = true;
	}
}

void CubeGame::OnMouseUp(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) == 0)
		mLeftMouseDown = false;

	if ((btnState & MK_RBUTTON) == 0) {
		mRightMouseDown = false;
		mRightMouseDownTimer = mRightMouseDownTimerMax;
	}
}

void CubeGame::OnMouseMove(WPARAM btnState, int x, int y)
{
    if(mCursorInUse)
    {
        // Make each pixel correspond to a quarter of a degree.
        float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
        float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

		mPlayer->SetRotation(dx, dy);
		mPlayerChangedView = true;

		mLastMousePos.x = x;
		mLastMousePos.y = y;
    }
}

void CubeGame::UpdateBlockSelector() {
	std::shared_ptr<Block> block = Raycast::GetFirstBlockInRay(Block::sAllBlocks, mPlayer->GetCam()->GetPosition(), mPlayer->GetCam()->GetLook());
	//If the block is active
	if (block != nullptr && block->GetActive()) {
		//If the previous block ins't the same as the new block
		if (mPreviousSelectedBlock == nullptr || block->GetID() != mPreviousSelectedBlock->GetID()) {
			mPreviousSelectedBlock = block;

			//Activate and set the position and material of the block selector
			mBlockSelector->SetActive(true);
			mBlockSelector->SetPosition(block->GetBoundingBox().Center);
			mBlockSelector->GetRI()->Mat = GameData::sMaterials->at("mat_blockSelect").get();

			//Set the block breaker visual timers
			mBlockSelectorTimer = 0.f;
			mBlockSelectorTextureCount = 0;
			mCurrentBlockDurability = block->GetDurability();
			mBlockTimerMax = mCurrentBlockDurability / (float)mBlockBreakTexNames->size();
		}
	}
	else {
		mBlockSelector->SetActive(false);
		mPreviousSelectedBlock = nullptr;
	}
}
 
void CubeGame::OnKeyboardInput(const GameTimer& gt)
{
	float playerX = 0.f;
	float playerZ = 0.f;
	bool playerJump = false;

	bool keyWDown = GetAsyncKeyState('W') & 0x8000;
	bool keySDown = GetAsyncKeyState('S') & 0x8000;
	bool keyADown = GetAsyncKeyState('A') & 0x8000;
	bool keyDDown = GetAsyncKeyState('D') & 0x8000;
	bool keyPDown = GetAsyncKeyState('P') & 0x8000;
	bool keySpaceDown = GetAsyncKeyState(VK_SPACE) & 0x8000;
	if (!keyWDown && !keySDown && !keyADown && !keyDDown && !keyPDown && !keySpaceDown)
	{
		actionComplete = false;
	}


	switch (currentState)
	{
	case GameStates::PLAYGAME:
		if (keyWDown || keySDown || keyADown || keyDDown || keySpaceDown) {
			if (keyWDown) {
				playerZ = 1.f;
			}

			if (keySDown) {
				playerZ = -1.f;
			}

			if (keyADown) {
				playerX = -1.f;
			}

			if (keyDDown) {
				playerX = 1.f;
			}

			if (keySpaceDown) {

				playerJump = true;
			}

			mPlayerChangedView = true;
			mPlayer->SetMovement(playerX, playerZ, playerJump);
		}
		if (keyPDown && (actionComplete == false))
		{
			actionComplete = true;
			changeState(GameStates::PAUSE);
		}
		//DEBUG
		if (GetAsyncKeyState('1') & 0x8000) {
			mCursorInUse = false;
		}

		if (GetAsyncKeyState('2') & 0x8000) {
			RespawnPlayer();
		}
		if (GetAsyncKeyState('3') & 0x8000) {
			mShowDebugInfo++;
			if (mShowDebugInfo > 2) mShowDebugInfo = 0;

			mUI_Text->ResetVerticies();
			mPlayerChangedView = true;
		}
		break;
	case GameStates::PAUSE:
		if (keyPDown && (actionComplete == false))
		{
			//while (keyPDown & 0x80);
			actionComplete = true;
			changeState(GameStates::PLAYGAME);
		}
		break;
	case GameStates::STARTUP:
		if (keySpaceDown) {
			changeState(GameStates::LOADWORLD);
			SetUIString("Loading", 10, 8);
		}
		break;
	case GameStates::LOADWORLD:
		SetUIString("Loading...", 10, 8);

		GenerateWorld();
		mRitemLayer[(int)GameData::RenderLayer::UserInterface]->push_back(mUI_Crosshair->GetRI());
		mRitemLayer[(int)GameData::RenderLayer::UserInterface]->push_back(mUI_Hotbar->GetRI());

		//Once everything has loaded, switch to the playstate
		changeState(GameStates::PLAYGAME);

		break;
	default:
		break;
	}
}

void CubeGame::MineSelectedBlock(const float dTime) {
	mCurrentBlockDurability -= dTime;	// dTime * mineingRate; (different tools mine quicker)
	mBlockSelectorTimer += dTime;

	if (mBlockSelectorTimer >= mBlockTimerMax) {
		mBlockSelectorTimer = 0;
		mBlockSelectorTextureCount++;
		switch (mBlockSelectorTextureCount) {
		default:
			mBlockSelector->GetRI()->Mat = GameData::sMaterials->at("mat_blockSelect").get();
			break;
		case(1):
			mBlockSelector->GetRI()->Mat = GameData::sMaterials->at("mat_blockSelect1").get();
			break;
		case(2):
			mBlockSelector->GetRI()->Mat = GameData::sMaterials->at("mat_blockSelect2").get();
			break;
		case(3):
			mBlockSelector->GetRI()->Mat = GameData::sMaterials->at("mat_blockSelect3").get();
			break;
		case(4):
			mBlockSelector->GetRI()->Mat = GameData::sMaterials->at("mat_blockSelect4").get();
			break;
		case(5):
			mBlockSelector->GetRI()->Mat = GameData::sMaterials->at("mat_blockSelect5").get();
			break;
		case(6):
			mBlockSelector->GetRI()->Mat = GameData::sMaterials->at("mat_blockSelect6").get();
			break;
		}
	}
	
	if (mCurrentBlockDurability <= 0)
		DestroySelectedBlock();
}
void CubeGame::DestroySelectedBlock() {

	XMFLOAT3 blockCenter = mPreviousSelectedBlock->GetBoundingBox().Center;

	bool stacked = false;

	for (std::shared_ptr<ItemEntity> entity : *ItemEntity::sAllItemEntities) {
		if (entity->GetRI()->Mat->Name == mPreviousSelectedBlock->GetInstanceData()->MaterialName) {
			XMFLOAT3 entityCenter = entity->GetBoundingBox().Center;
			XMFLOAT3 difference = XMFLOAT3{ blockCenter.x - entityCenter.x, blockCenter.y - entityCenter.y, blockCenter.z - entityCenter.z };
			float distance = sqrtf((difference.x * difference.x) + (difference.y * difference.y) + (difference.z * difference.z));

			if (distance <= mMaxNumberOfItemEntities) {
				//They are close enough to stack
				entity->AddStack();
				entity->SetPosition(blockCenter);
				stacked = true;
				break;
			}
		}
	}

	if (!stacked) {
		auto geo = mGeometries->at("geo_shape").get();
		auto itemEntityRI = std::make_shared<RenderItem>(geo, "mesh_itemEntity", GameData::sMaterials->at(mPreviousSelectedBlock->GetInstanceData()->MaterialName).get(), XMMatrixTranslation(mPreviousSelectedBlock->GetBoundingBox().Center.x, mPreviousSelectedBlock->GetBoundingBox().Center.y, mPreviousSelectedBlock->GetBoundingBox().Center.z));	//Make a render item
		auto itemEntity = std::make_shared<ItemEntity>(std::make_shared<GameObject>(itemEntityRI));
		GameObject::sAllGObjs->push_back(itemEntity);
		ItemEntity::sAllEntities->push_back(itemEntity);
		ItemEntity::sAllItemEntities->push_back(itemEntity);
		mRitemLayer[(int)GameData::RenderLayer::Main]->push_back(itemEntity->GetRI());
	}

	mPreviousSelectedBlock->SetActive(false);
	mPreviousSelectedBlock = Block::sAllBlocks->at(0);
	mBlockSelectorTextureCount = 0;
	mBlockSelector->GetRI()->Mat = GameData::sMaterials->at("mat_blockSelect").get();



	mPlayerChangedView = true;
}

void CubeGame::RespawnPlayer() {
	mWorldMgr.RelocatePlayer(mSpawnPoint);
	mPlayer->SetPosition(mSpawnPoint);
	mPlayerChangedView = true;
}

void CubeGame::ShowDebug() {
	
	if (mShowDebugInfo == 1) {
		XMFLOAT3 pos = mPlayer->GetBoundingBox().Center;
		SetUIString(("x:" + std::to_string(pos.x)), 0, 0);
		SetUIString(("y:" + std::to_string(pos.y)), 1, 0);
		SetUIString(("z:" + std::to_string(pos.z)), 2, 0);

		std::shared_ptr<WorldManager::Chunk> c = mWorldMgr.GetChunkFromWorldCoords(pos);
		WorldManager::Pos cPos = c->GetPos();
		SetUIString(("chunk x:" + std::to_string(cPos.x)), 4, 0);
		SetUIString(("chunk y:" + std::to_string(cPos.y)), 5, 0);
		SetUIString(("chunk z:" + std::to_string(cPos.z)), 6, 0);
		int i = c->GetID();
		if (i < 10)
			SetUIString(("chunk id:" + std::to_string(i) + "x"), 7, 0);
		else
			SetUIString(("chunk id:" + std::to_string(i)), 7, 0);
	}
	else if (mShowDebugInfo == 2) {
		SetUIString("1 to unfocus mouse", 0, 0);
		SetUIString("2 to respawn", 1, 0);
		SetUIString("3 to toggle debug text", 2, 0);
	}
}

void CubeGame::AnimateMaterials(const GameTimer& gt)
{
	
}

void CubeGame::UpdateObjectCBs(const GameTimer& gt)
{
	auto currObjectCB = mCurrFrameResource->ObjectCB.get();
	
	for (int j = 0; j < (int)GameData::RenderLayer::Count; j++) {
		for (int i = 0; i < mRitemLayer[j]->size(); i++) {
			if (mRitemLayer[j]->at(i)->NumFramesDirty > 0) {
				XMMATRIX world = XMLoadFloat4x4(&mRitemLayer[j]->at(i)->World);
				XMMATRIX texTransform = XMLoadFloat4x4(&mRitemLayer[j]->at(i)->TexTransform);

				ObjectConstants objConstants;
				XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
				XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));

				currObjectCB->CopyData(mRitemLayer[j]->at(i)->ObjCBIndex, objConstants);

				// Next FrameResource need to be updated too.
				mRitemLayer[j]->at(i)->NumFramesDirty--;
			}
		}
	}

	auto currInstanceCB = mCurrFrameResource->InstanceCB.get();
	BoundingFrustum cameraFrust = mPlayer->GetCam()->GetFrustum();
	


	//Loop through each render item
	for (int i = 0; i < mRitemIntances->size(); i++) {
		auto rItem = mRitemIntances->at(i);
		rItem->InstanceCount = 0;
		//Loop through each instance of the render item
//#pragma omp parallel for
		for (int j = 0; j < (int)rItem->Instances.size(); j++) {
			std::shared_ptr<InstanceData> idata = rItem->Instances.at(j);

			if (idata->Active) {

				//if (cameraFrust.Contains(idata->mBoundingBox) == DirectX::DISJOINT) {

					XMMATRIX world = XMLoadFloat4x4(&idata->World);

					InstanceConstants newIData;
					XMStoreFloat4x4(&newIData.World, XMMatrixTranspose(world));
					newIData.MaterialIndex = idata->MaterialIndex;

					currInstanceCB->CopyData(rItem->InstanceCount, newIData);
					rItem->InstanceCount++;

					idata->NumFramesDirty--;
				//}
			}
		}
	}
}

void CubeGame::UpdateMaterialCBs()
{
	auto currMaterialCB = mCurrFrameResource->MaterialCB.get();

	for(std::map<std::string, std::shared_ptr<Material>>::iterator it = GameData::sMaterials->begin(); it != GameData::sMaterials->end(); it++)
	{
		// Only update the cbuffer data if the constants have changed.  If the cbuffer
		// data changes, it needs to be updated for each FrameResource.
		std::shared_ptr<Material> mat = it->second;
		if(mat->NumFramesDirty > 0)
		{
			XMMATRIX matTransform = XMLoadFloat4x4(&mat->MatTransform);
			XMMATRIX matTransformTop = XMLoadFloat4x4(&mat->MatTransformTop);
			XMMATRIX matTransformBottom = XMLoadFloat4x4(&mat->MatTransformBottom);

			MaterialConstants matConstants;
			matConstants.DiffuseAlbedo = mat->DiffuseAlbedo;
			matConstants.FresnelR0 = mat->FresnelR0;
			matConstants.Roughness = mat->Roughness;
			matConstants.DiffuseMapIndex = mat->DiffuseSrvHeapIndex;
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
	mMainPassCB.NearZ = mFrontPlane;
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
		7,  // number of descriptors
		0,  // register t0
		0); 

	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[6];

	// Create root CBV.
	slotRootParameter[0].InitAsConstantBufferView(0);	//Object
	slotRootParameter[1].InitAsConstantBufferView(1);	//Material
	slotRootParameter[2].InitAsConstantBufferView(2);	//MainPass
	slotRootParameter[3].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);	//Texture
	slotRootParameter[4].InitAsShaderResourceView(0, 1);	//Instance data
	slotRootParameter[5].InitAsShaderResourceView(1, 1);	//Instance material data

	auto staticSamplers = GetStaticSamplers();


	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(6, slotRootParameter, (UINT)staticSamplers.size(), staticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if(errorBlob != nullptr)
	{
		OutputDebugStringA((char*)errorBlob->GetBufferPointer());
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

	//Standard
	mShaders["shader_standardVS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", nullptr, "VS", "vs_5_1");

	mShaders["shader_opaquePS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", nullptr, "PS", "ps_5_1");
	//UI / 2D
	mShaders["shader_2DPS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", nullptr, "TwoDPS", "ps_5_1");
	mShaders["shader_2DVS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", nullptr, "TwoDVS", "vs_5_1");
	//Sky
	mShaders["shader_SkyVS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", nullptr, "SkyVS", "vs_5_1");
	mShaders["shader_SkyPS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", nullptr, "SkyPS", "ps_5_1");
	//Instance
	mShaders["shader_InstanceVS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", nullptr, "InstanceVS", "vs_5_1");
	mShaders["shader_InstancePS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", nullptr, "InstancePS", "ps_5_1");

	//Main
	mInputLayout->push_back({
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    });

	//GUI
	mInputLayout->push_back({
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	});

	//Sky
	mInputLayout->push_back({{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }, });

	//Instance	//Same as main
	mInputLayout->push_back({
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	});
}

void CubeGame::CreateTextureSRV(std::string textureName, CD3DX12_CPU_DESCRIPTOR_HANDLE handle) {
	//Gets a local version of the texture resource
	auto fontTex = mTextures[textureName]->Resource;

	//Describes the SRV
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = fontTex->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = fontTex->GetDesc().MipLevels;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	//Creates the SRV
	md3dDevice->CreateShaderResourceView(fontTex.Get(), &srvDesc, handle);
}

void CubeGame::BuildDescriptorHeaps() {
	//Describe the descriptor heap and create it. Its big enough to hold all the textures.
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = (UINT)mTextures.size();
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mSrvDescriptorHeap)));
	mSrvDescriptorHeap.Get()->SetName(LPCWSTR(L"textureHeap"));

	//The start handle (location) of the descriptor heap
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	//The size of one texture. Used to offset the handle
	UINT cbvSrvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	CreateTextureSRV("tex_font", hDescriptor);

	//Offset the handle before making a new texture
	hDescriptor.Offset(1, cbvSrvDescriptorSize);
	CreateTextureSRV("tex_blocks", hDescriptor);

	//Since the sky is a cube map, it has a different View Dimension
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	hDescriptor.Offset(1, cbvSrvDescriptorSize);
	auto skyTex = mTextures["tex_skyTex"]->Resource;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = skyTex->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = skyTex->GetDesc().MipLevels;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	md3dDevice->CreateShaderResourceView(skyTex.Get(), &srvDesc, hDescriptor);

	hDescriptor.Offset(1, cbvSrvDescriptorSize);
	CreateTextureSRV("tex_blockSelect", hDescriptor);

	hDescriptor.Offset(1, cbvSrvDescriptorSize);
	CreateTextureSRV("tex_gui_elements", hDescriptor);
	
	hDescriptor.Offset(1, cbvSrvDescriptorSize);
	CreateTextureSRV("tex_gui_menus", hDescriptor);
}

void CubeGame::BuildShapeGeometry()
{
	GeometryGenerator geoGen;

	const int numb = 4;
	std::string geoHolderNames[numb] = { "geo_shape", "geo_ui_text1" , "geo_ui_crosshair" , "geo_ui_hotbar" };
	std::vector<GeometryGenerator::MeshData> meshDatas[numb];
	std::vector<std::string> meshNames[numb];

	//Shape Geos
	meshDatas[0].push_back(Block::CreateCubeGeometry(0.5f, 1.5f, 0.5f, mBlockTexturePositions["dirt"].x, 1.0f));
	meshNames[0].push_back("mesh_player");
	meshDatas[0].push_back(Block::CreateCubeGeometry(1.0f, 1.0f, 1.0f, mBlockTexturePositions["dirt"].x, 1.0f));
	meshNames[0].push_back("mesh_cube");
	meshDatas[0].push_back(Block::CreateCubeGeometry(0.2f, 0.2f, 0.2f, mBlockTexturePositions["dirt"].x, 1.0f));
	meshNames[0].push_back("mesh_itemEntity");
	meshDatas[0].push_back(Block::CreateCubeGeometry(1.05f, 1.05f, 1.05f, mBlockBreakTexturePositions[mBlockBreakTexNames[1]].x, 1.0f));
	meshNames[0].push_back("mesh_blockSelector");
	meshDatas[0].push_back(geoGen.CreateSphere(0.5f, 20, 20));
	meshNames[0].push_back("mesh_sky");

	//UI Geos
	meshDatas[1].push_back(mUI_Text->CreateUIPlane2D(1.95f, 1.95f, mGUITextRows, mGUITextCols));
	meshNames[1].push_back("mesh_gui_text1");
	meshDatas[2].push_back(mUI_Crosshair->CreateUIPlane2D(1.95f, 1.95f, mGUIElementRows, mGUIElementCols));
	meshNames[2].push_back("mesh_gui_crosshair");
	meshDatas[3].push_back(mUI_Hotbar->CreateUIPlane2D(0.7f, 0.15f, 1, 1));
	meshNames[3].push_back("mesh_gui_hotbar");

	for (int md = 0; md < numb; md++) {
		//Get the total number of vertices
		size_t totalVertexCount = 0;
		for each (GeometryGenerator::MeshData mds in meshDatas[md]) {
			totalVertexCount += mds.Vertices.size();
		}

		//Get a vector of each vertex
		std::vector<GeometryGenerator::Vertex> vertices(totalVertexCount);
		UINT k = 0;
		for each (GeometryGenerator::MeshData mds in meshDatas[md]) {
			for (size_t i = 0; i < mds.Vertices.size(); ++i, ++k) {
				vertices[k].Pos = mds.Vertices[i].Pos;
				vertices[k].Normal = mds.Vertices[i].Normal;
				vertices[k].TexC = mds.Vertices[i].TexC;
			}
		}

		//Get a vector of each index
		std::vector<std::uint16_t> indices;
		for each (GeometryGenerator::MeshData mds in meshDatas[md]) {
			indices.insert(indices.end(), std::begin(mds.GetIndices16()), std::end(mds.GetIndices16()));
		}

		//Get the total byte size of each vector
		const UINT vbByteSize = (UINT)vertices.size() * sizeof(GeometryGenerator::Vertex);
		const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

		//Make a MeshGeometry to hold all the data
		std::shared_ptr<MeshGeometry> geo = std::make_unique<MeshGeometry>();
		geo->Name = geoHolderNames[md];

		ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
		CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

		ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
		CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

		geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
			mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

		geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
			mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

		geo->VertexByteStride = sizeof(GeometryGenerator::Vertex);
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

		std::pair<std::string, std::shared_ptr<MeshGeometry>> pair(geo->Name, geo);
		mGeometries->insert(pair);

	}
}

void CubeGame::BuildPSOs()
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;

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

	CD3DX12_BLEND_DESC blendDesc;
	blendDesc.AlphaToCoverageEnable = true;				//Must be true to view transparent objects through a transparent object!
	blendDesc.IndependentBlendEnable = false;
	blendDesc.RenderTarget[0] = transparencyBlendDesc;


    ZeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	opaquePsoDesc.InputLayout = { mInputLayout->at((int)GameData::RenderLayer::Main).data(), (UINT)mInputLayout->at((int)GameData::RenderLayer::Main).size() };
	opaquePsoDesc.pRootSignature = mRootSignature.Get();
	opaquePsoDesc.VS = 
	{ 
		reinterpret_cast<BYTE*>(mShaders["shader_standardVS"]->GetBufferPointer()), 
		mShaders["shader_standardVS"]->GetBufferSize()
	};
	opaquePsoDesc.PS = 
	{ 
		reinterpret_cast<BYTE*>(mShaders["shader_opaquePS"]->GetBufferPointer()),
		mShaders["shader_opaquePS"]->GetBufferSize()
	};
	opaquePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	//opaquePsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	opaquePsoDesc.BlendState = blendDesc;
	opaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	opaquePsoDesc.SampleMask = UINT_MAX;
	opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	opaquePsoDesc.NumRenderTargets = 1;
	opaquePsoDesc.RTVFormats[0] = mBackBufferFormat;
	opaquePsoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	opaquePsoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	opaquePsoDesc.DSVFormat = mDepthStencilFormat;

    ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&mPSOs["pso_main"])));
	mPSOs["pso_main"].Get()->SetName(LPCWSTR(L"main"));
	//**********************************
	//Instance

	D3D12_GRAPHICS_PIPELINE_STATE_DESC InstancePso = opaquePsoDesc;
	InstancePso.InputLayout = { mInputLayout->at(3).data(), (UINT)mInputLayout->at(3).size() };

	InstancePso.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["shader_InstanceVS"]->GetBufferPointer()),
		mShaders["shader_InstanceVS"]->GetBufferSize()
	};
	InstancePso.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["shader_InstancePS"]->GetBufferPointer()),
		mShaders["shader_InstancePS"]->GetBufferSize()
	};

	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&InstancePso, IID_PPV_ARGS(&mPSOs["pso_instance"])));
	mPSOs["pso_instance"].Get()->SetName(LPCWSTR(L"instance"));
	//***********************************
	//UI

	D3D12_GRAPHICS_PIPELINE_STATE_DESC UserInterface = opaquePsoDesc;

	//UserInterface.InputLayout = { mInputLayout->at((int)GameData::RenderLayer::UserInterface).data(), (UINT)mInputLayout->at((int)GameData::RenderLayer::UserInterface).size() };

	UserInterface.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["shader_2DPS"]->GetBufferPointer()),
		mShaders["shader_2DPS"]->GetBufferSize()
	};

	UserInterface.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["shader_2DVS"]->GetBufferPointer()),
		mShaders["shader_2DVS"]->GetBufferSize()
	};

	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&UserInterface, IID_PPV_ARGS(&mPSOs["pso_userInterface"])));
	mPSOs["pso_userInterface"].Get()->SetName(LPCWSTR(L"userinterface"));

	//***********************************
	//Sky

	D3D12_GRAPHICS_PIPELINE_STATE_DESC skyPsoDesc = opaquePsoDesc;
	// The camera is inside the sky sphere, so just turnoff culling.
	skyPsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	// Make sure the depth function is LESS_EQUAL and not just LESS.
	// Otherwise, the normalized depth values at z = 1 (NDC)will
	// fail the depth test if the depth buffer was cleared to 1.
	skyPsoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	skyPsoDesc.pRootSignature = mRootSignature.Get();
	skyPsoDesc.InputLayout = { mInputLayout->at((int)GameData::RenderLayer::Sky).data(), (UINT)mInputLayout->at((int)GameData::RenderLayer::Sky).size() };
	skyPsoDesc.VS =
	{
	reinterpret_cast<BYTE*>(mShaders["shader_SkyVS"] -> GetBufferPointer()), mShaders["shader_SkyVS"]->GetBufferSize()
	};
	skyPsoDesc.PS =
	{
	reinterpret_cast<BYTE*>(mShaders["shader_SkyPS"] -> GetBufferPointer()), mShaders["shader_SkyPS"]->GetBufferSize()
	};
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(
		&skyPsoDesc, IID_PPV_ARGS(&mPSOs["pso_sky"])));
	mPSOs["pso_sky"].Get()->SetName(LPCWSTR(L"sky"));
}

void CubeGame::BuildFrameResources()
{
	UINT totalExtraRI = mMaxNumberOfItemEntities;// +mMaxUICount;; // maxEntityCount		//Render items which have not yet been created
	UINT totalRI = (UINT)(mRitemLayer[(int)GameData::RenderLayer::Main]->size() 
		+ mRitemLayer[(int)GameData::RenderLayer::UserInterface]->size() 
		+ mRitemLayer[(int)GameData::RenderLayer::Sky]->size()
		+ totalExtraRI
		);

	UINT totalBlocks = mWorldMgr.GetTotalAmountOfBlocks();

    for(int i = 0; i < GameData::sNumFrameResources; ++i)
    {
		mFrameResources.push_back(std::make_unique<FrameResource>(md3dDevice.Get(),
			1, totalRI, (UINT)GameData::sMaterials->size(), totalBlocks));
    }

	for (UINT i = totalExtraRI; i > 0; i--) {
		GameData::sAvailableObjCBIndexes.push_back(totalRI - i);
	}
}

void CubeGame::BuildMaterials()
{
	float x = mBlockTexturePositions["dirt"].x;
	float x2 = mBlockBreakTexturePositions["b0"].x;

	CreateMaterial("mat_font", 0, { 1.0f, 1.0f, 1.0f , 0.5f}, { 0.f, 0.f });

	CreateMaterial("mat_player", 1, DirectX::Colors::Black, { 0,0 });
	CreateMaterial("mat_dirt", 1, {0.4311f, 0.1955f, 0.1288f, 1.f }, { x,0 });
	CreateMaterial("mat_grass", 1, { 0.4311f, 0.1955f, 0.1288f, 1.f }, { x * 2.f,0 }, { x * 3.f,0 }, { x,0 });
	CreateMaterial("mat_stone", 1, { 0.4311f, 0.1955f, 0.1288f, 1.f }, { x * 4.f,0 });
	CreateMaterial("mat_coal_ore", 1, { 0.4311f, 0.1955f, 0.1288f, 1.f }, { x * 5.f,0 });
	CreateMaterial("mat_iron_ore", 1, { 0.4311f, 0.1955f, 0.1288f, 1.f }, { x * 6.f,0 });
	CreateMaterial("mat_oak_log", 1, { 0.4311f, 0.1955f, 0.1288f, 1.f }, { x * 7.f,0 }, { x * 8.f,0 }, { x * 8.f,0 });
	CreateMaterial("mat_oak_leaf", 1, { 0.4311f, 0.1955f, 0.1288f, 1.f }, { x * 9.f,0 });

	CreateMaterial("mat_sky", 2, { 1.0f, 1.0f, 1.0f }, { 0.f, 0.f });

	CreateMaterial("mat_blockSelect", 3, { 1.0f, 1.0f, 1.0f, 1.0f }, { 0.f, 0.f });
	CreateMaterial("mat_blockSelect1", 3, { 1.0f, 1.0f, 1.0f, 1.0f }, { x2, 0.f });
	CreateMaterial("mat_blockSelect2", 3, { 1.0f, 1.0f, 1.0f, 1.0f }, { x2 * 2, 0.f });
	CreateMaterial("mat_blockSelect3", 3, { 1.0f, 1.0f, 1.0f, 1.0f }, { x2 * 3, 0.f });
	CreateMaterial("mat_blockSelect4", 3, { 1.0f, 1.0f, 1.0f, 1.0f }, { x2 * 4, 0.f });
	CreateMaterial("mat_blockSelect5", 3, { 1.0f, 1.0f, 1.0f, 1.0f }, { x2 * 5, 0.f });
	CreateMaterial("mat_blockSelect6", 3, { 1.0f, 1.0f, 1.0f, 1.0f }, { x2 * 6, 0.f });

	CreateMaterial("mat_gui_elements", 4, { 1.0f, 1.0f, 1.0f , 0.5f }, { 0.f, 0.f });
	CreateMaterial("mat_gui_menus", 5, { 1.0f, 1.0f, 1.0f , 0.5f }, { 0.f, 0.f });
}

void CubeGame::CreateMaterial(std::string name, int textureIndex, DirectX::XMVECTORF32 color, DirectX::XMFLOAT2 texTransform) {
	auto mat = std::make_shared<Material>();
	mat->Name = name;
	mat->MatCBIndex = (int)GameData::sMaterials->size();
	mat->DiffuseSrvHeapIndex = textureIndex;
	mat->DiffuseAlbedo = XMFLOAT4(color);
	mat->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	mat->Roughness = 0.2f;
	XMMATRIX trans = DirectX::XMMatrixTranslation(texTransform.x, texTransform.y, 0.f);
	DirectX::XMStoreFloat4x4(&mat->MatTransform, trans);
	DirectX::XMStoreFloat4x4(&mat->MatTransformTop, trans);
	DirectX::XMStoreFloat4x4(&mat->MatTransformBottom, trans);

	std::pair<std::string, std::shared_ptr<Material>> materialPair(name, mat);
	std::pair<std::string, int> indexPair(name, (int)GameData::sMaterials->size());

	GameData::sMaterials->insert(materialPair);
	mMaterialIndexes->insert(indexPair);
}

void CubeGame::CreateMaterial(std::string name, int textureIndex, DirectX::XMVECTORF32 color, DirectX::XMFLOAT2 texTransform, DirectX::XMFLOAT2 texTransformTop, DirectX::XMFLOAT2 texTransformBottom) {
	CreateMaterial(name, textureIndex, color, texTransform);
	DirectX::XMStoreFloat4x4(&GameData::sMaterials->at(name)->MatTransformTop, DirectX::XMMatrixTranslation(texTransformTop.x, texTransformTop.y, 0.f));
	DirectX::XMStoreFloat4x4(&GameData::sMaterials->at(name)->MatTransformBottom, DirectX::XMMatrixTranslation(texTransformBottom.x, texTransformBottom.y, 0.f));
}

void CubeGame::BuildGameObjects()
{
	//Get geometries
	auto geo = mGeometries->at("geo_shape").get();

	//Player-------------------------
	auto playerRI = std::make_shared<RenderItem>(geo, "mesh_player", GameData::sMaterials->at("mat_player").get(), XMMatrixTranslation(1.0f, 200.0f, 1.0f));	//Make a render item
	mPlayer = std::make_shared<Player>(std::make_shared<GameObject>(playerRI));						//Make the Player
	GameObject::sAllGObjs->push_back(mPlayer);
	Entity::sAllEntities->push_back(mPlayer);												//Add the player to the enities list
	mRitemLayer[(int)GameData::RenderLayer::Main]->push_back(mPlayer->GetRI());			//Add the players render item to the main list

	//Sky----------------------------
	auto skyRI = std::make_shared<RenderItem>(geo, "mesh_sky", GameData::sMaterials->at("mat_sky").get(), XMMatrixScaling(5000.0f, 5000.0f, 5000.0f));
	mRitemLayer[(int)GameData::RenderLayer::Sky]->push_back(skyRI);

	//UI----------------------------
	//Text
	auto text = std::make_shared<RenderItem>(mGeometries->at("geo_ui_text1").get(), "mesh_gui_text1", GameData::sMaterials->at("mat_font").get(), XMMatrixIdentity());
	mUI_Text->Init(text, mCommandList);
	mRitemLayer[(int)GameData::RenderLayer::UserInterface]->push_back(mUI_Text->GetRI());

	//Crosshair
	std::shared_ptr<RenderItem> crosshair = std::make_shared<RenderItem>(mGeometries->at("geo_ui_crosshair").get(), "mesh_gui_crosshair", GameData::sMaterials->at("mat_gui_elements").get(), XMMatrixIdentity());
	mUI_Crosshair->Init(crosshair, mCommandList);
	mUI_Crosshair->SetTexture((mGUIElementCols * (mGUIElementRows / 2) + (mGUIElementCols / 2)), mGUIElementTexturePositions["crosshair"], mGUIElementTextureSize);
	mUI_Crosshair->UpdateBuffer();

	//Hotbar
	std::shared_ptr<RenderItem> hotbar = std::make_shared<RenderItem>(mGeometries->at("geo_ui_hotbar").get(), "mesh_gui_hotbar", GameData::sMaterials->at("mat_gui_menus").get(), XMMatrixTranslation(-0.33f, -0.8f, 0.f));
	mUI_Hotbar->Init(hotbar, mCommandList);
	DirectX::XMFLOAT4 p = mGUIElementTexturePositionsAndSizes["hotbar"];
	mUI_Hotbar->SetWholeTexture({ p.x, p.y }, {p.z, p.w});
	mUI_Hotbar->UpdateBuffer();


	//Block selector---------------
	auto selectorRI = std::make_shared<RenderItem>(geo, "mesh_blockSelector", GameData::sMaterials->at("mat_blockSelect").get(), XMMatrixTranslation(0.f, 0.f, 0.f));
	mBlockSelector = std::make_shared<GameObject>(selectorRI);
	GameObject::sAllGObjs->push_back(mBlockSelector);
	mRitemLayer[(int)GameData::RenderLayer::Main]->push_back(mBlockSelector->GetRI());

	//World-------------------------
	auto blockRI = std::make_shared<RenderItemInstance>(geo, "mesh_cube", GameData::sMaterials->at("mat_grass").get());
	mRitemIntances->push_back(blockRI);
	Block::sBlockInstance = blockRI;
	mWorldMgr.Init(mMaterialIndexes);
}

void CubeGame::GenerateWorld() {
	mWorldMgr.CreateWorld();
	mWorldMgr.LoadFirstChunks(mSpawnPoint);
}

void CubeGame::GenerateListOfActiveItems() {
	//#pragma omp parallel for
	for (int i = 0; i < (int)mRitemLayer[(int)GameData::RenderLayer::Main]->size(); ++i)
	{
		if (mRitemLayer[(int)GameData::RenderLayer::Main]->at((size_t)i)->active) {
			mActiveRItems->push_back(mRitemLayer[(int)GameData::RenderLayer::Main]->at((size_t)i));
		}
	}
}

void CubeGame::DrawRenderItems(ID3D12GraphicsCommandList* cmdList, std::shared_ptr<std::vector<std::shared_ptr<RenderItem>>> ritems)
{
	auto objectCB = mCurrFrameResource->ObjectCB->Resource();
	auto matCB = mCurrFrameResource->MaterialCB->Resource();

    // For each render item...
    for(int i = 0; i < (int)ritems->size(); ++i)
    {
		if(ritems->at((size_t)i)->active){
			auto ri = ritems->at((size_t)i);

			cmdList->IASetVertexBuffers(0, 1, &ri->Geo->VertexBufferView());
			cmdList->IASetIndexBuffer(&ri->Geo->IndexBufferView());
			cmdList->IASetPrimitiveTopology(ri->PrimitiveType);

			D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * mObjCBByteSize;
			cmdList->SetGraphicsRootConstantBufferView((UINT)0, objCBAddress);

			D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex * mMatCBByteSize;
			cmdList->SetGraphicsRootConstantBufferView((UINT)1, matCBAddress);

			CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
			tex.Offset(ri->Mat->DiffuseSrvHeapIndex, mCbvSrvUavDescriptorSize);
			mCommandList->SetGraphicsRootDescriptorTable(3, tex);

			cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, 0);
		}
	}
}

void CubeGame::DrawInstanceItems(ID3D12GraphicsCommandList* cmdList) {
	for (size_t i = 0; i < mRitemIntances->size(); ++i)
	{
		auto ri = mRitemIntances->at(i);

		cmdList->IASetVertexBuffers(0, 1, &ri->Geo->VertexBufferView());
		cmdList->IASetIndexBuffer(&ri->Geo->IndexBufferView());
		cmdList->IASetPrimitiveTopology(ri->PrimitiveType);

		// Set the instance buffer to use for this render-item.  For structured buffers, we can bypass 
		// the heap and set as a root descriptor.
		auto instanceBuffer = mCurrFrameResource->InstanceCB->Resource();
		mCommandList->SetGraphicsRootShaderResourceView(4, instanceBuffer->GetGPUVirtualAddress());

		cmdList->DrawIndexedInstanced(ri->IndexCount, ri->InstanceCount, ri->StartIndexLocation, ri->BaseVertexLocation, 0);
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

void CubeGame::changeState(GameStates newState)
{
	currentState = newState;
	mUI_Text->ResetVerticies();
	mUI_Text->SetDirtyFlag();
}