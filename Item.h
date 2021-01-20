#pragma once

#include <string>

enum class ItemType {
    BLOCK,
    TOOL
};

enum class blockType {
    type_Default = 0,
    type_Dirt,
    type_Grass,
    type_Stone,
    type_Wood,
    type_Count
};

class Item {
public:
    std::string GetName() { return mName; }
    ItemType GetType() { return mType; }
    short int GetMaxStackSize() { return mMaxStackSize; }
protected:
    Item(std::string name, ItemType type, short int maxStackSize) : mName(name), mType(type), mMaxStackSize(maxStackSize) {}
    std::string mName;
    ItemType mType;
    short int mMaxStackSize;
};