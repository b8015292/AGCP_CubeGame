#pragma once

#include "Object.h"
#include "Common/d3dApp.h"

class Raycast {
public:
    static std::shared_ptr<Block> GetFirstBlockInRay(std::shared_ptr<std::vector<std::shared_ptr<Block>>> blocks, DirectX::XMVECTOR origin, DirectX::XMVECTOR dir);
    static std::shared_ptr<Block> GetBlockInfrontFirstBlockInRay(std::shared_ptr<std::vector<std::shared_ptr<Block>>> blocks, DirectX::XMVECTOR origin, DirectX::XMVECTOR dir);
};