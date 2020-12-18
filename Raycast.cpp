#include "Raycast.h"

//#include "CubeGame.h"
#include <iostream>

std::shared_ptr<Block> Raycast::GetFirstBlockInRay(std::shared_ptr<std::vector<std::shared_ptr<Block>>> blocks, DirectX::XMVECTOR origin, DirectX::XMVECTOR dir) {
    float minDistance = 4;
    std::shared_ptr<Block> closestBlock = nullptr;

    for (std::shared_ptr<Block> block : *blocks) {
        if (block->GetActive() == true) {
            float distance = 0;
            if (block->GetBoundingBox().Intersects(origin, dir, distance)) {
                if (distance < minDistance) {
                    closestBlock = block;
                    minDistance = distance;
                }
            }
        }
    }

    return closestBlock;
}

std::shared_ptr<Block> Raycast::GetBlockInfrontFirstBlockInRay(std::shared_ptr<std::vector<std::shared_ptr<Block>>> blocks, DirectX::XMVECTOR origin, DirectX::XMVECTOR dir) {


    float minDistance = 4;
    std::vector<std::shared_ptr<Block>> blocksInRay = {};
    std::shared_ptr<Block> closestBlock = nullptr;

    for (std::shared_ptr<Block> block : *blocks) {
        float distance = 0;
        if (block->GetBoundingBox().Intersects(origin, dir, distance)) {
            blocksInRay.push_back(block);
            if (distance < minDistance) {
                if (block->GetActive() == true) {
                    closestBlock = block;
                    minDistance = distance;
                }
            }
        }
    }

    std::shared_ptr<Block> closestInactiveBlock = nullptr;
    if (closestBlock != nullptr) {
        float maxInactiveDistance = 0;

        for (std::shared_ptr<Block> block : blocksInRay) {
            float distance = 0;
            if (block->GetActive() == false) {
                if (block->GetBoundingBox().Intersects(origin, dir, distance)) {
                    if (distance <= minDistance) {
                        if (distance > maxInactiveDistance) {
                            closestInactiveBlock = block;
                            maxInactiveDistance = distance;
                        }
                    }
                }
            }
        }
    }

    return closestInactiveBlock;
}