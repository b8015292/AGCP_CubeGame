#pragma once

#include <string>

enum class ItemType {
    ITEM,
    BLOCK
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