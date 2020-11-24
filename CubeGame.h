#pragma once

#include "Common/MathHelper.h"
#include "Common/UploadBuffer.h"
#include "Common/GeometryGenerator.h"
#include "FrameResource.h"
#include "Camera.h"

#include "GameData.h"
#include "Object.h"

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

    std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();
    void InitFont();
    void LoadTextures();
    void SetString(std::string str, XMFLOAT2 pos);

private:

    enum class RenderLayer : int
    {
        Opaque = 0,
        Transparent,
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

    //ComPtr<ID3D12PipelineState> mOpaquePSO = nullptr;

    // List of all the render items.
    std::shared_ptr<std::vector<std::shared_ptr<GameObject>>> mAllGObjs;
    std::shared_ptr<std::vector<std::shared_ptr<Entity>>> mAllEnts;

    // Render items divided by PSO.
    std::vector<std::shared_ptr<RenderItem>> mRitemLayer[(int)RenderLayer::Count];

    PassConstants mMainPassCB;

    /*old camera stuff
    XMFLOAT3 mEyePos = { 0.0f, 0.0f, 0.0f };
    XMFLOAT4X4 mView = MathHelper::Identity4x4();
    XMFLOAT4X4 mProj = MathHelper::Identity4x4();

    float mTheta = 1.5f * XM_PI;
    float mPhi = 0.2f * XM_PI;
    float mRadius = 15.0f;
    */

    POINT mLastMousePos;

    Camera mCamera;

    Font fnt;


};