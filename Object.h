#pragma once

#include "Common/d3dApp.h"
#include "FrameResource.h"  //For vertex struct

#include "GameData.h"
#include "Camera.h"


using Microsoft::WRL::ComPtr;
using namespace DirectX;

enum EPos { tfl = 0, tfr = 1, tbl = 2, tbr = 3, bfl = 4, bfr = 5, bbl = 6, bbr = 7, size = 8 };

//enum Face { top = 0, bottom = 1, front = 2, back = 3, left = 4, right = 5};

enum blockType {
    type_Default = 0,
    type_Dirt,
    type_Grass,
    type_Stone,
    type_Wood,
    type_Count
};

class GameObject {
public:
    static int sMaxID;

    //Constructor & Initializer
    GameObject(std::shared_ptr<std::vector<std::shared_ptr<GameObject>>> allGObjs, std::shared_ptr<RenderItem> rI);
    GameObject(std::shared_ptr<GameObject> gobj);
    ~GameObject();
    void CreateBoundingBox();

    //Getters and Setters
    bool GetActive() { return mActive; };
    void SetActive(bool val);
    int GetID() { return mID; };
    bool GetApplyGravity() { return mApplyGravity; }
    std::array<XMFLOAT3, 8> GetCoords();
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

    //Collision Checks
    std::vector<int> CheckAllCollisionsAtBox(BoundingBox nextPos);
    bool CheckIfCollidingAtBox(BoundingBox nextPos);

protected:
    XMFLOAT3 mVel;
    XMFLOAT3 mMaxVel;

    bool mOnGround = false;
};

class Player : public Entity {
public:
    Player(std::shared_ptr<GameObject> gobj);
    ~Player();

    void Update(const float dTime) override; //overides entities update
    void TranslateCamera(float dTime, float x, float y, float z);
    void Jump();
    void Walk(float d, float dTime);
    void Strafe(float d, float dTime);
    void Pitch(float d);
    void RotateY(float d);

    Camera* GetCam() { return &mCamera; };

private:
    Camera mCamera;
    bool mJumped = false;
};


class Block : public GameObject {
public:
    Block(std::shared_ptr<GameObject> GObj);
    ~Block();

    void Init();
    void activate(blockType newType);
    void deactivate();
    
    //static void SetTexturePositions(const int mBlockTexSize, const int mBlockTexRows,const int mBlockTexCols, const std::string mBlockTexNames[]);

    float *getWorldCoords();
private:
    const float blockDimension = 1.0f;
    blockType type;
    float worldCoord[3];


    //static std::unordered_map<std::string, DirectX::XMFLOAT2> mBlockTexturePositions;
    //void SetTexture(blockType type);

};

class Item : protected GameObject {

};