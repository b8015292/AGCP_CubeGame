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
                OutputDebugStringW(L"--------\nactive false\n");
                if (block->GetBoundingBox().Intersects(origin, dir, distance)) {
                    std::wostringstream ss;
                    ss << distance << " " << minDistance << "\n";
                    std::wstring s(ss.str());
                    OutputDebugStringW(s.c_str());
                    OutputDebugStringW(L"intersects");
                    if (distance <= minDistance) {
                        OutputDebugStringW(L"closer than solid block");
                        if (distance > maxInactiveDistance) {
                            OutputDebugStringW(L"further than closest non solid");
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