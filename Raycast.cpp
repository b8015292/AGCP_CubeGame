#include "Raycast.h"

//#include "CubeGame.h"
#include <iostream>

std::shared_ptr<Block> Raycast::GetFirstBlockInRay(std::shared_ptr<std::vector<std::shared_ptr<Block>>> blocks, DirectX::XMVECTOR origin, DirectX::XMVECTOR dir) {
    DirectX::XMVECTOR pos = origin;
    for (int i = 0; i < 10; i++) {
        pos += XMVector2Normalize(dir);
        for (std::shared_ptr<Block> block : *blocks) {
            std::wostringstream ss;
            ss << block->GetBoundingBox().Center.x << " " << floorf(pos.m128_f32[0]) << " " << block->GetBoundingBox().Center.y << " " << floorf(pos.m128_f32[1]) << " " << block->GetBoundingBox().Center.z << " " << floorf(pos.m128_f32[2]) << "\n";
            std::wstring s(ss.str());
            OutputDebugStringW(s.c_str());
            if (block->GetBoundingBox().Center.x == floorf(pos.m128_f32[0])
                && block->GetBoundingBox().Center.y == floorf(pos.m128_f32[1])
                && block->GetBoundingBox().Center.z == floorf(pos.m128_f32[2])
                && block->GetActive() == true) {
                OutputDebugString(L"FINISHED");
                return block;
            }
        }
    }
    return nullptr;
}

std::shared_ptr<Block> Raycast::GetBlockInfrontFirstBlockInRay(std::shared_ptr<std::vector<std::shared_ptr<Block>>> blocks, DirectX::XMVECTOR origin, DirectX::XMVECTOR dir) {
    DirectX::XMVECTOR pos = origin;
    for (int i = 0; i < 10; i++) {
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
    }
    return nullptr;
}