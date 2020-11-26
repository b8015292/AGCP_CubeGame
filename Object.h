#pragma once

#include "Common/d3dApp.h"
#include "FrameResource.h"  //For vertex struct

#include "GameData.h"
#include "Collision.h"
#include "Camera.h"


using Microsoft::WRL::ComPtr;
using namespace DirectX;

struct RenderItem
{
    RenderItem() = default;

    // World matrix of the shape that describes the object's local space
    // relative to the world space, which defines the position, orientation,
    // and scale of the object in the world.
    XMFLOAT4X4 World = MathHelper::Identity4x4();

    XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();

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

class GameObject {
public:
    //Variables
    std::shared_ptr<RenderItem> mRI;
    std::shared_ptr<std::vector<std::shared_ptr<GameObject>>> mAllGObjs;

    static int sMaxID;
    int mID = 0;

    bool active = true;
    bool applyGravity = true;

    BoundingBox boundingBox;
    
    //Functions
    GameObject(std::shared_ptr<std::vector<std::shared_ptr<GameObject>>> allGObjs);
    GameObject(std::shared_ptr<GameObject> gobj);
    Collision::ColCube GetCoords();
    void Translate(const float dTime, float x, float y, float z);
    void CreateBoundingBox();
};

class Entity : public GameObject {
public:
    XMFLOAT3 mVel;
    XMFLOAT3 mMaxVel;

    bool mOnGround = false;
    Collision::ColPoints mColPoints;

    Entity(std::shared_ptr<std::vector<std::shared_ptr<GameObject>>> allGObjs);
    Entity(std::shared_ptr<GameObject> gobj);
    void Init();
    virtual void Update(const float dTime);
    std::vector<int> CheckAllCollisions(Collision::ColCube thisCube);
    Collision::ColPoints GetAllCollisionPoints(Collision::ColCube coordinates);
    bool IsPointColliding(const XMFLOAT3 point);

    void AddVelocity(float x, float y, float z);

    bool temp = true;

};

class Player : public Entity {
public:
    Player(std::shared_ptr<std::vector<std::shared_ptr<GameObject>>> allGObjs);
    Player(std::shared_ptr<GameObject> gobj);

    void Update(const float dTime) override; //overides entities update
    void TranslateCamera(float dTime, float x, float y, float z);

    Camera* GetCam() { return &mCamera; };

private:
    Camera mCamera;
};

class Block : protected GameObject {
    int type;
};

class Item : protected GameObject {

};