#pragma once
#include "Object.h"
#include "Player.h"
#include "WorldManager.h"
#include "Pathfinding.h"

class LivingEntity : public Entity {
public:
    static std::shared_ptr<std::vector<std::shared_ptr<LivingEntity>>> sAllLivingEntities;

    LivingEntity(std::shared_ptr<GameObject> gobj, std::shared_ptr<Player> player, std::shared_ptr<WorldManager> worldManager);
    ~LivingEntity();

    void Update(const float dTime) override; //overides entities update
    void Walk(float d, float dTime);
    void Strafe(float d, float dTime);
    void WalkToBlock(XMFLOAT3 blockLocation);

    //Move
    void Jump();
    void CheckCollisions(float dTime);
    void MoveInLook(float dTime);
    void MoveInAxis(float dTime);

    void DebugPath();

private:
    bool mJumped = true;
    XMVECTOR direction;
    XMFLOAT3 walkTo = { -1, -1, -1 };
    XMFLOAT3 nextWalkTo = { -1, -1, -1 };
    std::shared_ptr<Player> mPlayer;
    std::shared_ptr<WorldManager> mWorldManager;

    const float mJumpOffset = 0.2f;     //This is applied to the Y axis when checking collisions while walking, because the player is alays being pushed into the ground

    float walkSpeed = 1;
    float maxHealth = 10;
    float health = 10;

    //Movement
    class Dir {
    public:
        const static int look = 0;

        //const static int axisStrafe = 5;

        const static int count = 1;
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
    const float mMoveSpeed = 1.f;
    XMMATRIX mRotate90;

    bool debugging = false;

    std::vector<Node> path;

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