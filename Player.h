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

private:
    Camera mCamera;
    bool mJumped = true;

    DirectX::XMMATRIX newWorldMatrix;

    const float mJumpOffset = 0.2f;     //This is applied to the Y axis when checking collisions while walking, because the player is alays being pushed into the ground
    const float mCameraOffsetZ = 2.0f;   //For 3rd person
    const float mCameraOffsetY = 0.6f;  //Height
};