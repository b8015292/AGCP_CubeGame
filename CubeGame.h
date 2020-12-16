#pragma once

#include "Common/MathHelper.h"
#include "Common/UploadBuffer.h"
#include "FrameResource.h"

#include "GameData.h"
#include "Object.h"
#include "UI.h"

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
    virtual void OnResize()override;
    virtual void Update(const GameTimer& gt)override;
    virtual void Draw(const GameTimer& gt)override;

    virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
    virtual void OnMouseUp(WPARAM btnState, int x, int y)override;
    virtual void OnMouseMove(WPARAM btnState, int x, int y)override;

    void OnKeyboardInput(const GameTimer& gt);
    void AnimateMaterials(const GameTimer& gt);
    void UpdateObjectCBs(const GameTimer& gt);
    void UpdateMaterialCBs(const GameTimer& gt);
    void UpdateMainPassCB(const GameTimer& gt);

    void BuildRootSignature();
    void BuildShadersAndInputLayout();
    void BuildDescriptorHeaps();
    void BuildShapeGeometry();
    void BuildPSOs();
    void BuildFrameResources();
    void BuildMaterials();
    void BuildRenderItems();
    void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<std::shared_ptr<RenderItem>> ritems);
    void DrawUI(ID3D12GraphicsCommandList* cmdList, const std::vector<std::shared_ptr<RenderItem>> ritems);

    void MakeTexture(std::string name, std::string path);
    void MakeTexture(std::string name, std::wstring path);
    void LoadTextures();
    std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();
    void SetBlockTexturePositions(const int blockTexSize, const int blockTexRows, const int blockTexCols, const std::string blockTexNames[]);
    void CreateMaterial(std::string name, int textureIndex, DirectX::XMVECTORF32 color, DirectX::XMFLOAT2 texTransform);
    void CreateMaterial(std::string name, int textureIndex, DirectX::XMVECTORF32 color, DirectX::XMFLOAT2 texTransform, DirectX::XMFLOAT2 texTransformTop, DirectX::XMFLOAT2 texTransformBottom);

private:

    enum class RenderLayer : int
    {
        Opaque = 0,
        Transparent,
        Sky,
        Count
    };

    std::vector<std::unique_ptr<FrameResource>> mFrameResources;
    FrameResource* mCurrFrameResource = nullptr;
    int mCurrFrameResourceIndex = 0;

    UINT mCbvSrvDescriptorSize = 0;

    ComPtr<ID3D12RootSignature> mRootSignature = nullptr;

    ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap = nullptr;

    std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> mGeometries;
    std::unordered_map<std::string, std::unique_ptr<Material>> mMaterials;
    std::unordered_map<std::string, std::unique_ptr<Texture>> mTextures;
    std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;
    std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;

    std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;

    // List of all the render items.
    std::shared_ptr<std::vector<std::shared_ptr<GameObject>>> mAllGObjs;
    std::shared_ptr<std::vector<std::shared_ptr<Entity>>> mAllEnts;
    std::shared_ptr<std::vector<std::shared_ptr<Block>>> mAllBlocks;

    // Render items divided by PSO.
    std::vector<std::shared_ptr<RenderItem>> mRitemLayer[(int)RenderLayer::Count];

    PassConstants mMainPassCB;


    POINT mLastMousePos;

    std::shared_ptr<Player> mPlayer;
    UI mUI;

    Font fnt;

    const int mBlockTexSize = 32;
    const int mBlockTexRows = 1;
    const int mBlockTexCols = 7;
    const std::string mBlockTexNames[7] = { "null", "dirt", "grassSide", "grass", "stone", "null", "null"};
    std::unordered_map<std::string, DirectX::XMFLOAT2> mBlockTexturePositions;
};