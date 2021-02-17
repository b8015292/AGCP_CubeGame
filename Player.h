#pragma once

#include "Object.h"

class Player : public Entity {
public:
    Player(std::shared_ptr<GameObject> gobj);

    void Update(const float dTime) override; //overides entities update
    void TranslateCamera(float dTime, float x, float y, float z);
    void SetPosition(XMFLOAT3 newPos);

    //Move
    void SetMovement(float x, float z, bool jumping);
    bool Walk(float d, float dTime);
    bool Strafe(float d, float dTime);
    void MoveInRawDirection(float x, float z, float dTime);
    void Jump();


    //Rotate view
    void SetRotation(float x, float y);
    void Pitch(float d);
    void RotateY(float d);

    //bool MovementCollisionCheck(float d, float dTime);

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
    Camera mCamera;
    bool mJumped = true;
    bool mDiagonal = false;

    //DirectX::XMMATRIX mnewWorldMatrix;

    const float mJumpOffset = 0.2f;      //This is applied to the Y axis when checking collisions while walking, because the player is alays being pushed into the ground
    const float mCameraOffsetZ = 0.0f;   //For 3rd person
    const float mCameraOffsetY = 0.6f;   //Height

    int mPlayerHealth;
    const int mMaxHealth = 100;
    int mPlayerDamage;

    const float mMoveSpeed = 5.0f;
    float mMoveX = 0;
    float mMoveZ = 0;
    bool mSetJump = false;

    float mPitch = 0; //y
    float mYaw = 0;   //X
};