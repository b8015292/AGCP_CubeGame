#pragma once

#include <string>
#include <unordered_map>
#include <map>

#include "Common/d3dUtil.h"
//#include "Common/MathHelper.h"

class GameData {
public:
    static const int sNumFrameResources = 3;
    static const float sGrav;
    static bool sRunning;
    static std::vector<UINT> sAvailableObjCBIndexes;
    static std::shared_ptr<std::map<std::string, std::shared_ptr<Material>>> sMaterials;

    //Each render layer is rendered in a different way (using different PSOs)
    enum class RenderLayer : int
    {
        Main = 0,
        UserInterface,
        Sky,
        Count
    };

    static UINT GetObjectCBIndex();
    static void AddNewObjectCBIndex(UINT i);

    static void StoreFloat4x4InMatrix(DirectX::XMMATRIX& dest, const DirectX::XMFLOAT4X4 source);
    static std::wstring StringToWString(std::string s);
    static void Print(std::string message);

    static DirectX::XMFLOAT3 AddFloat3AndVector(DirectX::XMFLOAT3, DirectX::XMVECTOR);
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

