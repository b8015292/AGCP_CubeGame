#pragma once

#include "Common/MathHelper.h"
#include "Common/UploadBuffer.h"
#include "FrameResource.h"

#include "GameData.h"
#include "Object.h"
#include "Text.h"
#include "WorldManager.h"
#include "Inventory.h"
#include "Player.h"
#include "LivingEntity.h"

#include "omp.h"

using Microsoft::WRL::ComPtr;
//using namespace DirectX;
using namespace DirectX::PackedVector;

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")

const int gNumFrameResources = GameData::sNumFrameResources;

class CubeGame : public D3DApp
{
public:
    CubeGame(HINSTANCE hInstance);
    CubeGame(const CubeGame& rhs) = delete;
    CubeGame& operator=(const CubeGame& rhs) = delete;
    ~CubeGame();

    virtual bool Initialize()override;

private:
    //Initialization
    void BuildRootSignature();              //Tells the GPU which registers to expect to use   
    void BuildShadersAndInputLayout();      //Compiles shaders and defines their input parameters
    void BuildDescriptorHeaps();            //Creates a heap to hold the textures 
    void BuildShapeGeometry();              //Builds the geometric shapes
    void BuildPSOs();                       //Makes all the Pipeline State Objects - they tell the GPU how to render objects differently (by use different shaders)
    void BuildFrameResources();             //Each frame resource has its own version of the Pass Constant, materials and objects
    void BuildMaterials();
    void BuildGameObjects();                //Creates all the gameobjects

    //Input handling
    virtual void OnResize()override;
    virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
    virtual void OnMouseUp(WPARAM btnState, int x, int y)override;
    virtual void OnMouseMove(WPARAM btnState, int x, int y)override;
    void OnKeyboardInput(const GameTimer& gt);

    //Updateing
    virtual void Update(const GameTimer& gt)override;
    void AnimateMaterials(const GameTimer& gt);
    void UpdateObjectCBs(const GameTimer& gt);
    void UpdateMaterialCBs(const GameTimer& gt);
    void UpdateMainPassCB(const GameTimer& gt);

    //Drawing
    virtual void Draw(const GameTimer& gt)override;
    void GenerateListOfActiveItems();
    void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, std::shared_ptr<std::vector<std::shared_ptr<RenderItem>>> ritems);
    void DrawInstanceItems(ID3D12GraphicsCommandList* cmdList);
    std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();   //Creates the static samples - different ways to interperate textures


    //More init stuff
    //Makes individual textures from a file specified by the path
    void MakeTexture(std::string name, std::string path);
    void MakeTexture(std::string name, std::wstring path);
    void CreateTextureSRV(std::string textureName, CD3DX12_CPU_DESCRIPTOR_HANDLE handle);
    //Loads all the textures and splits some textures into texel location maps
    void LoadTextures();                
    //Splits textures map up into texel location maps.
    void SplitTextureMapIntoPositions(std::unordered_map<std::string, DirectX::XMFLOAT2>& out, const int texSize, const int rows, const int cols, const std::string texNames[]);
    //Creates all the UI objects
    void BuildUserInterfaces();
    //Makes a material, using default values for roughness and fresnel
    void CreateMaterial(std::string name, int textureIndex, DirectX::XMVECTORF32 color, DirectX::XMFLOAT2 texTransform);
    //Makes a material with a different top and bottom texture
    void CreateMaterial(std::string name, int textureIndex, DirectX::XMVECTORF32 color, DirectX::XMFLOAT2 texTransform, DirectX::XMFLOAT2 texTransformTop, DirectX::XMFLOAT2 texTransformBottom);
    //Makes a cube
    //void CreateCube(std::string materialName, XMFLOAT3 pos);

    //Sets a string on the GUI
    void SetUIString(std::string str, int lineNo, int col); 

    //Block stuff
    void UpdateBlockSelector();
    void MineSelectedBlock(const float dTime);
    void DestroySelectedBlock();

private:
    //Each frame resource has its own copy of the pass constant, materials and objects
    std::vector<std::unique_ptr<FrameResource>> mFrameResources;
    FrameResource* mCurrFrameResource = nullptr;
    int mCurrFrameResourceIndex = 0;

    ComPtr<ID3D12RootSignature> mRootSignature = nullptr;

    ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap = nullptr;

    std::shared_ptr<std::unordered_map<std::string, std::shared_ptr<MeshGeometry>>> mGeometries;
    std::shared_ptr<std::unordered_map<std::string, std::shared_ptr<Material>>> mMaterials;
    std::shared_ptr<std::unordered_map<std::string, int>> mMaterialIndexes;
    std::unordered_map<std::string, std::unique_ptr<Texture>> mTextures;
    std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;
    std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;

    // List of all the render items.
    std::vector<std::vector<D3D12_INPUT_ELEMENT_DESC>> mInputLayout[(int)GameData::RenderLayer::Count];
    std::shared_ptr<std::vector<std::shared_ptr<RenderItem>>> mActiveRItems;
    std::shared_ptr<std::unordered_map<std::string, std::shared_ptr<UI>>> mAllUIs;

    UINT mObjCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
    UINT mMatCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

    // Render items divided by PSO.
    std::shared_ptr<std::vector<std::shared_ptr<RenderItem>>> mRitemLayer[(int)GameData::RenderLayer::Count];
    std::shared_ptr<std::vector<std::shared_ptr<RenderItemInstance>>> mRitemIntances;

    PassConstants mMainPassCB;

    WorldManager mWorldMgr;

    std::shared_ptr<Player> mPlayer;
    float mPlayerSpawnX = 12;
    float mPlayerSpawnY = 31;
    float mPlayerSpawnZ = 12;
    bool mPlayerChangedView = false;
    bool mPlayerMoved = false;

    //Camera variables
    const float mBackPlane = 1000.0f;
    const float mFrontPlane = 0.0001f;

    //User interface
    std::shared_ptr<Text> mUI_Text;
    Font fnt;
    const int mUIRows = 26;
    const int mUICols = 26;

    //Mouse input
    bool mCursorInUse = false;
    bool mCursorInUsePrev = false;
    POINT mLastMousePos;
    bool mLeftMouseDown = false;
    bool mRightMouseDown = false;
    float mRightMouseDownTimer = 0.f;
    float mRightMouseDownTimerMax = 0.3f;

    //Block textures
    const int mBlockTexSize = 32;
    const int mBlockTexRows = 1;
    const int mBlockTexCols = 7;
    const std::string mBlockTexNames[7] = { "null", "dirt", "grassSide", "grass", "stone", "null", "null"};
    std::unordered_map<std::string, DirectX::XMFLOAT2> mBlockTexturePositions;

    //Block Break
    std::shared_ptr<GameObject> mBlockSelector;
    const std::string mBlockBreakTexNames[7] = { "select", "b0", "b1", "b2", "b3", "b4", "b5" };
    std::unordered_map<std::string, DirectX::XMFLOAT2> mBlockBreakTexturePositions;
    std::shared_ptr<Block> mPreviousSelectedBlock = nullptr;
    //Used to determine when to break the block
    float mCurrentBlockDurability = -1;
    //Used to determine the texture of the block selector
    float mBlockTimerMax = -1;
    float mBlockSelectorTimer = 0;
    int mBlockSelectorTextureCount = 0;

};