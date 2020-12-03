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