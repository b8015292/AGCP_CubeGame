#pragma once
#include "Object.h"

#include "Common/d3dApp.h"
#include "FrameResource.h"  //For vertex struct

#include "GameData.h"
#include "Camera.h"
#include "Item.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;

class LivingEntity : public Entity {
public:
    LivingEntity(std::shared_ptr<GameObject> gobj);
    ~LivingEntity();

    void Update(const float dTime) override; //overides entities update
    void Jump();
    void Walk(float d, float dTime);
    void Strafe(float d, float dTime);
    void WalkToBlock(XMFLOAT3 blockLocation);

private:
    bool mJumped = true;
    XMVECTOR direction;
    XMFLOAT3 walkTo;

    const float mJumpOffset = 0.2f;     //This is applied to the Y axis when checking collisions while walking, because the player is alays being pushed into the ground

    float walkSpeed;
    float maxHealth;
    float health;
};

class Enemy : public LivingEntity {
public:

private:

};

class Friendly : public LivingEntity {
public:

private:

};