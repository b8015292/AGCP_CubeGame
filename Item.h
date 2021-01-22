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
    int GetDurability() { return mDurability; }
    int GetAmountInStack() { return mAmountInStack; }
    void decreaseStack(int amountToDecrease) { mAmountInStack -= amountToDecrease; }
    bool fullStack() { return mMaxStackSize == mAmountInStack; }
    void increaseStack(int amountAdded) { if(!fullStack()) mAmountInStack += amountAdded; }
protected:
    Item(std::string name, ItemType type, short int maxStackSize, int durability) : mName(name), mType(type), mMaxStackSize(maxStackSize), mDurability(durability) {}
    std::string mName;
    ItemType mType;
    short int mMaxStackSize;
    short int mAmountInStack;
    int mDurability;
};