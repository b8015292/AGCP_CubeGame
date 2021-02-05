#pragma once
#include "Common/d3dApp.h"
#include "FrameResource.h"  //For vertex struct

#include "Object.h"
#include "GameData.h"
#include "Camera.h"
#include "Item.h"

class Player : public Entity 
{
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

	int getPlayerHealth() { return mPlayerHealth; }
	int getPlayerDamage() { return mPlayerDamage; }

	void increasePlayerDamage(int itemBonus) { mPlayerDamage += itemBonus; }
	void decreasePlayerDamage(int itemBonus) { mPlayerDamage -= itemBonus; }
	void increasePlayerHealth(int amount);
	void decreasePlayerHealth(int amount);

	int getDistanceTravelled() { return mDistanceTravelled; }
	int getEnemiesKilled() { return mEnemiesKilled; }
	int getFriendliesKilled() { return mFriendliesKilled; }

	void increaseDistanceTravelled() { ++mDistanceTravelled; }
	void increaseFriendliesKilled() { ++mFriendliesKilled; }
	void increaseEnemiesKilled() { ++mEnemiesKilled; }

private:
	Camera mCamera;
	bool mJumped = true;

	DirectX::XMMATRIX newWorldMatrix;

	const float mJumpOffset = 0.2f;     //This is applied to the Y axis when checking collisions while walking, because the player is alays being pushed into the ground
	const float mCameraOffsetZ = 0.f;   //For 3rd person
	const float mCameraOffsetY = 0.6f;  //Height

	//Stats that effect the game
	int mPlayerHealth;
	const int mPlayerMaxHealth;
	int mPlayerDamage;

	//Overall stats
	int mDistanceTravelled;
	int mEnemiesKilled;
	int mFriendliesKilled;
};