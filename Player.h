#pragma once

#include "Object.h"

class Player : public Entity {
public:
    Player(std::shared_ptr<GameObject> gobj);

    void Update(const float dTime) override; //overides entities update
    void TranslateCamera(float dTime, float x, float y, float z);
    void Jump();
    void Walk(float d, float dTime);
    void Strafe(float d, float dTime);
    void Pitch(float d);
    void RotateY(float d);
    bool MovementCollisionCheck(float d, float dTime);

    Camera* GetCam() { return &mCamera; };

    bool* getDiagonal() { return &mDiagonal; };

    int getPlayerHealth() { return mPlayerHealth; }
    int getPlayerDamage() { return mPlayerDamage; }

    void increasePlayerHealth(int amount);
    void decreasePlayerHealth(int amount);
    void increasePlayerDamage(int weaponBonus) { mPlayerDamage += weaponBonus; }
    void decreasePlayerDamage(int weaponBonus) { mPlayerDamage -= weaponBonus; }

private:
    Camera mCamera;
    bool mJumped = true;
    bool mDiagonal = false;

    DirectX::XMMATRIX mnewWorldMatrix;

    const float mJumpOffset = 0.2f;      //This is applied to the Y axis when checking collisions while walking, because the player is alays being pushed into the ground
    const float mCameraOffsetZ = 2.0f;   //For 3rd person
    const float mCameraOffsetY = 0.6f;   //Height

    int mPlayerHealth;
    const int mMaxHealth = 100;
    int mPlayerDamage;
};