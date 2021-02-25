#pragma once

#include "Common/d3dUtil.h"
#include "Common/MathHelper.h"

#include "GameData.h"

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

    void CreateBoundingBox();
    std::array<DirectX::XMFLOAT3, 8> GetCoords();


    bool active = true;

    // Index into GPU constant buffer corresponding to the ObjectCB for this render item.
    UINT ObjCBIndex = -1;

    Material* Mat = nullptr;
    MeshGeometry* Geo = nullptr;
    std::string MeshName = "";
    DirectX::BoundingBox mBoundingBox;

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

    void UpdateBoundingBox();

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
    InstanceData(DirectX::XMMATRIX world, UINT materialIndex, std::string materialName, DirectX::BoundingBox bb) {
        XMStoreFloat4x4(&World, world);
        MaterialIndex = materialIndex;
        MaterialName = materialName;
        mBoundingBox = bb;
        UpdateBoundingBox();
    };
    InstanceData& operator=(const InstanceData& id) {
        World = id.World;
        TexTransform = id.TexTransform;
        MaterialIndex = id.MaterialIndex;
        NumFramesDirty = id.NumFramesDirty;
        mBoundingBox = id.mBoundingBox;
        return (*this);
    }

    void UpdateBoundingBox();

    DirectX::XMFLOAT4X4 World = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();
    UINT MaterialIndex = 0;
    UINT NumFramesDirty = gNumFrameResources;
    //UINT BufferIndex = -1;
    bool Active = true;
    bool Visible = false;

    std::string MaterialName;
    DirectX::BoundingBox mBoundingBox;

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