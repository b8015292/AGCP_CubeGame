#pragma once

#include "Common/d3dApp.h"
#include "FrameResource.h"  //For vertex struct

#include "GameData.h"
#include "Collision.h"
#include "Camera.h"
//#include "CubeGame.h"


using Microsoft::WRL::ComPtr;
using namespace DirectX;

enum blockType {
    type_Default = 0,
    type_Grass,
    type_Dirt,
    type_Stone,
    type_Wood,
};

class GameObject {
public:
    static int sMaxID;

    //Constructor & Initializer
    GameObject(std::shared_ptr<std::vector<std::shared_ptr<GameObject>>> allGObjs);
    GameObject(std::shared_ptr<std::vector<std::shared_ptr<GameObject>>> allGObjs, std::shared_ptr<RenderItem> rI);
    GameObject(std::shared_ptr<GameObject> gobj);
    ~GameObject();
    void CreateBoundingBox();

    //Getters and Setters
    bool GetActive() { return mActive; };
    void SetActive(bool val);
    int GetID() { return mID; };
    bool GetApplyGravity() { return mApplyGravity; }
    Collision::ColCube GetCoords();
    BoundingBox GetBoundingBox() { return mBoundingBox; };
    std::shared_ptr<RenderItem> GetRI() { return mRI; };

    //Mutators
    void Translate(const float dTime, float x, float y, float z);

protected:
    int mID = 0;
    bool mApplyGravity = true;

    std::shared_ptr<RenderItem> mRI;
    std::shared_ptr<std::vector<std::shared_ptr<GameObject>>> mAllGObjs;

    BoundingBox mBoundingBox;    //Contains the center point and the size

private:
    bool mActive = true;

};

class Entity : public GameObject {
public:

    //Constructors
    Entity(std::shared_ptr<std::vector<std::shared_ptr<GameObject>>> allGObjs);
    Entity(std::shared_ptr<GameObject> gobj);
    ~Entity();
    void Init();

    //Getters/Setters
    void AddVelocity(float x, float y, float z);
    void SetVelocity(XMFLOAT3 newVel);
    void SetMaxVelocity(XMFLOAT3 newMaxVel);
    XMFLOAT3 GetVelocity() { return mVel; };
    XMFLOAT3 GetMaxVelocity() { return mMaxVel; };

    //Mutators
    virtual void Update(const float dTime);

    std::vector<int> CheckAllCollisions(Collision::ColCube thisCube);
    Collision::ColPoints GetAllCollisionPoints(Collision::ColCube coordinates);
    bool IsPointColliding(const XMFLOAT3 point);

protected:
    XMFLOAT3 mVel;
    XMFLOAT3 mMaxVel;

    bool mOnGround = false;
    Collision::ColPoints mColPoints;
};

class Player : public Entity {
public:
    Player(std::shared_ptr<std::vector<std::shared_ptr<GameObject>>> allGObjs);
    Player(std::shared_ptr<GameObject> gobj);
    ~Player();

    void Update(const float dTime) override;
    void TranslateCamera(float dTime, float x, float y, float z);

    Camera* GetCam() { return &mCamera; };

private:
    Camera mCamera;
};


class Block : public GameObject {
public:
    Block(std::shared_ptr<std::vector<std::shared_ptr<GameObject>>> allGObjs, std::shared_ptr<RenderItem> ri);
    Block(std::shared_ptr<GameObject> GObj);
    ~Block();

    void Init();
    void activate(blockType newType);
    void deactivate();
    
private:
    const float blockDimension = 1.0f;
    blockType type;
    float worldCoord[3];
};

class Item : protected GameObject {

};