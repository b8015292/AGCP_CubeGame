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
    DirectX::XMVECTOR pos = origin;
    /*for (int i = 0; i < 10; i++) {
        pos += dir * 0.5;
        std::shared_ptr<Block> foundBlock = nullptr;
        for (std::shared_ptr<Block> block : *blocks) {
            std::wostringstream ss;
            ss << block->GetBoundingBox().Center.x << " " << ((int)pos.m128_f32[0]) << " " << block->GetBoundingBox().Center.y << " " << ((int)pos.m128_f32[0]) << " " << block->GetBoundingBox().Center.z << " " << ((int)pos.m128_f32[0]) << "\n";
            std::wstring s(ss.str());
            OutputDebugString(s.c_str());
            if (block->GetBoundingBox().Center.x == (int)pos.m128_f32[0]
                && block->GetBoundingBox().Center.y == (int)pos.m128_f32[1]
                && block->GetBoundingBox().Center.z == (int)pos.m128_f32[2]
                && block->GetActive() == true) {
                OutputDebugString(L"FINISHED");
                foundBlock = block;
                break;
            }
        }
        if (foundBlock != nullptr) {
            pos -= dir * 0.5;
            for (std::shared_ptr<Block> block : *blocks) {
                if (block->GetBoundingBox().Center.x == (int)pos.m128_f32[0]
                    && block->GetBoundingBox().Center.y == (int)pos.m128_f32[1]
                    && block->GetBoundingBox().Center.z == (int)pos.m128_f32[2]
                    && block->GetActive() == true) {
                    return block;
                }
            }
            //block doesnt exist in frontx
        }
    }*/

    float minDistance = 4;
    std::vector<std::shared_ptr<Block>> blocksInRay = {};
    std::shared_ptr<Block> closestBlock = nullptr;

    for (std::shared_ptr<Block> block : *blocks) {
        float distance = 0;
        if (block->GetBoundingBox().Intersects(origin, dir, distance)) {
            blocksInRay.push_back(block);
            if (distance < minDistance) {
                closestBlock = block;
                minDistance = distance;
            }
        }
    }

    //for()
    return nullptr;
}