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
        int posX;
        int posY;
        int width;
        int height;

        myChar() = default;

        myChar(int pX, int pY, int w, int h) {
            posX = pX;
            posY = pY;
            width = w;
            height = h;
        };
    };

    std::wstring filePath;
    std::unordered_map<char, myChar> chars;

};

