#pragma once

#include "Common/MathHelper.h"
#include <string>
#include <unordered_map>

#include "Common/GeometryGenerator.h"
#include "Common/d3dUtil.h"

class GameData {
public:
    static const int sNumFrameResources = 3;
    static const float sGrav;
    static bool sRunning;

    static void StoreFloat4x4InMatrix(DirectX::XMMATRIX& dest, const DirectX::XMFLOAT4X4 source);
};

struct RenderItem
{
    RenderItem() = default;

    bool active = true;

    // World matrix of the shape that describes the object's local space
    // relative to the world space, which defines the position, orientation,
    // and scale of the object in the world.
    DirectX::XMFLOAT4X4 World = MathHelper::Identity4x4();

    DirectX::XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();

    // Dirty flag indicating the object data has changed and we need to update the constant buffer.
    // Because we have an object cbuffer for each FrameResource, we have to apply the
    // update to each FrameResource.  Thus, when we modify obect data we should set 
    // NumFramesDirty = GameData.sNumFrameResources so that each frame resource gets the update.
    int NumFramesDirty = gNumFrameResources;

    // Index into GPU constant buffer corresponding to the ObjectCB for this render item.
    UINT ObjCBIndex = -1;

    Material* Mat = nullptr;
    MeshGeometry* Geo = nullptr;

    // Primitive topology.
    D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

    // DrawIndexedInstanced parameters.
    UINT IndexCount = 0;
    UINT StartIndexLocation = 0;
    int BaseVertexLocation = 0;
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

