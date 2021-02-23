#pragma once

#include "Common/d3dApp.h"
#include "FrameResource.h"  //For vertex struct

#include "GameData.h"
#include "Camera.h"
#include "Item.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;

enum EPos { tfl = 0, tfr = 1, tbl = 2, tbr = 3, bfl = 4, bfr = 5, bbl = 6, bbr = 7, size = 8 };
//enum Face { top = 0, bottom = 1, front = 2, back = 3, left = 4, right = 5};

class GameObject {
public:
    static std::shared_ptr<std::vector<std::shared_ptr<GameObject>>> sAllGObjs;
    static int sMaxID;

    //Constructor & Initializer
    GameObject(std::shared_ptr<RenderItem> rI);
    GameObject(std::shared_ptr<GameObject> gobj);
    GameObject();
    void CreateBoundingBox();

    bool operator==(GameObject& obj) { return GetID() == obj.GetID(); };

    //Getters and Setters
    bool GetActive() { return mActive; };
    virtual void SetActive(bool val);
    int GetID() { return mID; };
    bool GetApplyGravity() { return mApplyGravity; };
    virtual std::array<XMFLOAT3, 8> GetCoords();
    virtual BoundingBox GetBoundingBox() { return mBoundingBox; };
    std::shared_ptr<RenderItem> GetRI() { return mRI; };
    virtual void SetPosition(XMFLOAT3 pos);


    bool GetDirty() { return mDirty; };
    //If the object has been changed call this.
    void SetDirtyFlag() { mDirty = true; };
    //At the end of each update, if the object is dirty the render item is made dirty
    virtual void SetRIDirty() { mRI->NumFramesDirty++; mDirty = false; };

    //Mutators
    virtual void Translate(const float dTime, float x, float y, float z);
    virtual void Rotate(const float dTime, XMVECTOR axis, float angle);

protected:
    bool mActive = true;
    int mID = 0;
    bool mApplyGravity = true;
    bool mDirty = false;

    std::shared_ptr<RenderItem> mRI = nullptr;

    BoundingBox mBoundingBox;    //Contains the center point and the size



};

class Entity : public GameObject {
public:
    static std::shared_ptr<std::vector<std::shared_ptr<Entity>>> sAllEntities;

    //Constructors
    Entity(std::shared_ptr<GameObject> gobj);
    void Init();

    //Getters/Setters
    void AddVelocity(float x, float y, float z);
    void SetVelocity(XMFLOAT3 newVel);
    void SetMaxVelocity(XMFLOAT3 newMaxVel);
    XMFLOAT3 GetVelocity() { return mVel; };
    XMFLOAT3 GetMaxVelocity() { return mMaxVel; };

    //Mutators
    virtual void Update(const float dTime);

    //Collision Checks
    std::vector<int> CheckAllCollisionsAtBox(BoundingBox nextPos);
    bool CheckIfCollidingAtBox(BoundingBox nextPos);

protected:
    XMFLOAT3 mVel;
    XMFLOAT3 mMaxVel;

    bool mMoved = false;
};

class ItemEntity : public Entity {
public:
    //Static variables
    static std::shared_ptr<std::vector<std::shared_ptr<ItemEntity>>> sAllItemEntities;

    //Constructor
    ItemEntity(std::shared_ptr<GameObject> gobj);

    void Update(const float dTime) override;

    void Pickup();
};

class Block : public GameObject{
public:
    //Static variables
    static std::shared_ptr<std::vector<std::shared_ptr<Block>>> sAllBlocks;
    static std::shared_ptr<RenderItemInstance> sBlockInstance;

    //Static functions
    static GeometryGenerator::MeshData CreateCubeGeometry(float width, float height, float depth, float texWidth, float texHeight);

    //Constructor
    Block(std::shared_ptr<InstanceData> idata);

    //Getters
    float GetDurability() { return mDurability; };
    int GetInstanceIndex() { return mInstanceIndex; };
    std::shared_ptr<InstanceData> GetInstanceData() { return mInstanceData; };

    void SetActive(bool val)override;
    void SetRIDirty() override { mInstanceData->NumFramesDirty++; mDirty = false; };

    std::array<XMFLOAT3, 8> GetCoords() override;
    void SetPosition(XMFLOAT3 pos) override;
    void Translate(const float dTime, float x, float y, float z) override;


    
private:
    std::shared_ptr<InstanceData> mInstanceData;
    int mInstanceIndex = -1;

    const float blockDimension = 1.0f;
    float worldCoord[3];

    float mDurability = 1.f;

};