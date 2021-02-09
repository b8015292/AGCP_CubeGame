#pragma once

#include <string>
#include <unordered_map>

#include "Common/d3dUtil.h"
#include "Common/MathHelper.h"

enum items
{
    bread,
    meat,
    cookedMeat,

    pickaxe,
    shovel,
    sword,

    dirt,
    stone,
    wood,
    sticks,
    iron,
    gold,
    diamond
};

class GameData {
public:
    static const int sNumFrameResources = 3;
    static const float sGrav;
    static bool sRunning;
    static std::vector<UINT> sAvailableObjCBIndexes;

    //Each render layer is rendered in a different way (using different PSOs)
    enum class RenderLayer : int
    {
        Main = 0,
        UserInterface,
        Sky,
        Count
    };

    static UINT GetObjectCBIndex();
    static void AddNewObjectCBIndex(UINT i);

    static void StoreFloat4x4InMatrix(DirectX::XMMATRIX& dest, const DirectX::XMFLOAT4X4 source);
    static std::wstring StringToWString(std::string s);
    static void Print(std::string message);
};

class GeometryGenerator {
public:
    using uint16 = std::uint16_t;
    using uint32 = std::uint32_t;

    struct Vertex
    {
        Vertex(float pX, float pY, float pZ, float nX, float nY, float nZ, float tU, float tV)
            : Pos(pX, pY, pZ), Normal(nX, nY, nZ), TexC(tU, tV)
        {}

        Vertex() : Pos(0, 0, 0), Normal(0, 0, 0), TexC(0, 0) {}

        Vertex operator=(const Vertex& v) {
            //Vertex ret(Pos.x, Pos.y, Pos.z, Normal.x, Normal.y, Normal.z, TexC.x, TexC.y);
            Pos = v.Pos;
            Normal = v.Normal;
            TexC = v.TexC;
            return *this;
        }

        DirectX::XMFLOAT3 Pos;
        DirectX::XMFLOAT3 Normal;
        DirectX::XMFLOAT2 TexC;
    };

    struct MeshData
    {
        std::vector<Vertex> Vertices;
        std::vector<uint32> Indices32;

        std::vector<uint16>& GetIndices16()
        {
            if (mIndices16.empty())
            {
                mIndices16.resize(Indices32.size());
                for (size_t i = 0; i < Indices32.size(); ++i)
                    mIndices16[i] = static_cast<uint16>(Indices32[i]);
            }

            return mIndices16;
        }

    private:
        std::vector<uint16> mIndices16;
    };

    ///<summary>
    /// Creates a box centered at the origin with the given dimensions, where each
    /// face has m rows and n columns of vertices.
    ///</summary>
    MeshData CreateBox(float width, float height, float depth, uint32 numSubdivisions);

    ///<summary>
    /// Creates a sphere centered at the origin with the given radius.  The
    /// slices and stacks parameters control the degree of tessellation.
    ///</summary>
    MeshData CreateSphere(float radius, uint32 sliceCount, uint32 stackCount);

private:
    void Subdivide(MeshData& meshData);
    Vertex MidPoint(const Vertex& v0, const Vertex& v1);

};

class RenderItemParent
{
public:
    RenderItemParent() = default;
    RenderItemParent(MeshGeometry* meshGeo, std::string meshName, Material* mat);

    bool active = true;

    // Index into GPU constant buffer corresponding to the ObjectCB for this render item.
    UINT ObjCBIndex = -1;

    Material* Mat = nullptr;
    MeshGeometry* Geo = nullptr;
    std::string MeshName = "";

    // Primitive topology.
    D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    // DrawIndexedInstanced parameters.
    UINT IndexCount = 0;
    UINT StartIndexLocation = 0;
    int BaseVertexLocation = 0;
};

class RenderItem : public RenderItemParent {
public:
    static int sCBIndex;

    RenderItem() = default;
    RenderItem(MeshGeometry* meshGeo, std::string meshName, Material* mat, DirectX::XMMATRIX world);

    // World matrix of the shape that describes the object's local space
    // relative to the world space, which defines the position, orientation,
    // and scale of the object in the world.
    DirectX::XMFLOAT4X4 World = MathHelper::Identity4x4();

    DirectX::XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();

    int NumFramesDirty = gNumFrameResources;
};

class InstanceData {
public:
    InstanceData() = default;
    InstanceData(DirectX::XMMATRIX world,UINT materialIndex) {
        XMStoreFloat4x4(&World, world);
        MaterialIndex = materialIndex;
    };
    InstanceData& operator=(const InstanceData& id) {
        World = id.World;
        TexTransform = id.TexTransform;
        MaterialIndex = id.MaterialIndex;
        NumFramesDirty = id.NumFramesDirty;
        return (*this);
    }

    DirectX::XMFLOAT4X4 World = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();
    UINT MaterialIndex = 0;
    UINT NumFramesDirty = gNumFrameResources;
    UINT InstancePad1 = 0;
    bool Active = true;
    bool Pad2 = false;
    bool Pad3 = false;
    bool Pad4 = false;
};

class RenderItemInstance : public RenderItemParent {
public:
    RenderItemInstance() = default;
    RenderItemInstance(MeshGeometry* meshGeo, std::string meshName, Material* mat);
    RenderItemInstance& operator=(const RenderItemInstance& rii);

    UINT InstanceCount = 0;
    std::vector<std::shared_ptr<InstanceData>> Instances;
};


class Font {
public:

    struct myChar {
        float posX;
        float posY;
        float width;
        float height;

        myChar() = default;

        myChar(float pX, float pY, float w, float h) {
            posX = pX;
            posY = pY;
            width = w;
            height = h;
        };
    };

    std::wstring filePath;
    std::unordered_map<char, myChar> chars;

};

