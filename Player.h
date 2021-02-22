#pragma once

#include "Object.h"

class Player : public Entity {
public:
    Player(std::shared_ptr<GameObject> gobj);

    void Update(const float dTime) override; //overides entities update
    void TranslateCamera(float dTime, float x, float y, float z);
    void SetPosition(XMFLOAT3 newPos);

    //Input
    void SetMovement(float x, float z, bool jumping);
    void SetRotation(float x, float y);

    //Move
    void Jump();
    void CheckCollisions(float dTime);
    void MoveInLook(float dTime);
    void MoveInAxis(float dTime);

    bool Walk(float d, float dTime);

    //Rotate view

    void Pitch(float d);
    void RotateY(float d);

    //Getters
    Camera* GetCam() { return &mCamera; };
    bool* getDiagonal() { return &mDiagonal; };

    //Setters
    void SetRIDirty() override;

    //Get stats
    int getPlayerHealth() { return mPlayerHealth; }
    int getPlayerDamage() { return mPlayerDamage; }

    //Change stats
    void increasePlayerHealth(int amount);
    void decreasePlayerHealth(int amount);
    void increasePlayerDamage(int weaponBonus) { mPlayerDamage += weaponBonus; }
    void decreasePlayerDamage(int weaponBonus) { mPlayerDamage -= weaponBonus; }

private:
    //Camera
    Camera mCamera;
    const float mJumpOffset = 0.2f;      //This is applied to the Y axis when checking collisions while walking, because the player is alays being pushed into the ground
    const float mCameraOffsetZ = 0.0f;   //For 3rd person
    const float mCameraOffsetY = 0.6f;   //Height
    DirectX::XMMATRIX mNewWorldMatrix;

    //Health & damange stats
    int mPlayerHealth;
    const int mMaxHealth = 100;
    int mPlayerDamage;

    //Mouse look 
    float mPitch = 0; //y
    float mYaw = 0;   //X

    //Keyboard input
    bool mSetJump = false;
    float mStrafing = 0;
    float mWalking = 0; 

    //Jump
    bool mJumped = true;
    const float mJumpHeight = 15.f;

    //Movement
    class Dir {
    public:
        const static int look = 0;
        const static int lookStrafe = 1;
        const static int lookBoth = 2;

        const static int axisForward = 3;
        const static int axisSide = 4;
        //const static int axisStrafe = 5;

        const static int count = 5;
    };
    class Axis {
    public:
        const static int posZ = 0;
        const static int posX = 1;
        const static int negZ = 2;
        const static int negX = 3;

        const static int count = 4;
    };

    bool mDiagonal = false;
    const float mMoveSpeed = 5.f;
    XMMATRIX mRotate90;

    bool mCanMove[Dir::count];
    BoundingBox mNextBoxes[Dir::count];
    XMVECTOR mMoveVectors[Dir::count];
    const XMVECTOR mAxisVectors[Axis::count] = {
        { 0,   0,  1.f, 0},
        { 1.f, 0,  0,   0},
        { 0,   0, -1.f, 0},
        {-1.f, 0,  0,   0},
    };
};