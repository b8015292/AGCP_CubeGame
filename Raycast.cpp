#include "Raycast.h"

//#include "CubeGame.h"
#include <iostream>

std::shared_ptr<Block> Raycast::GetFirstBlockInRay(std::shared_ptr<std::vector<std::shared_ptr<Block>>> blocks, DirectX::XMVECTOR origin, DirectX::XMVECTOR dir) {
    //This minDistance is used so we don't check blocks that are really far away
    float minDistance = 4;
    std::shared_ptr<Block> closestBlock = nullptr;

    //Loop through all blocks. TODO: loop through only the blocks in nearby chunks to save performance
    for (std::shared_ptr<Block> block : *blocks) {
        if (block->GetActive() == true) {
            float distance = 0;
            //If this block is not air and if our "ray" intersects the block, then it's in their line of sight
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

    //We must use the same code as the GetFirstBlockInRay function here, because we need to modify it to keep a record of all the blocks in their line of sight (including air blocks)
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

    //Once we have all the blocks in their line of sight, and the closest block, we can then find the closest air block in this list of blocks
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