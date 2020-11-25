#pragma once

#include "Common/MathHelper.h"
#include <string>
#include <unordered_map>

class GameData {
public:
    static const int sNumFrameResources = 3;
    static const float sGrav;
    static bool sRunning;

    static void StoreFloat4x4InMatrix(DirectX::XMMATRIX& dest, const DirectX::XMFLOAT4X4 source);
};

class Font {
public:

    struct myChar {
        float posX;
        float posY;
        float width;
        float height;

        myChar() = default;

        myChar(float pX, float pY, float w, float h) {
            posX = pX;
            posY = pY;
            width = w;
            height = h;
        };
    };

    std::wstring filePath;
    std::unordered_map<char, myChar> chars;

};

