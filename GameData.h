#pragma once

#include "Common/MathHelper.h"

class GameData {
public:
    static const int sNumFrameResources = 3;

    static void StoreFloat4x4InMatrix(DirectX::XMMATRIX& dest, const DirectX::XMFLOAT4X4 source);
};