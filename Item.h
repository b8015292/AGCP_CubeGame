#pragma once

#include <string>

enum class ItemType {
    TOOL,
    MISC,
    BLOCK
};

class Item {
public:
    Item(std::string Name, ItemType Type, int MaxStackSize, int MeleeDamage, int Durability, char textureRef) 
        : mName(Name), mType(Type), mMaxStackSize(MaxStackSize), mMeleeDamage(MeleeDamage), mDurability(Durability), mTextureReference(textureRef) {};
    ~Item() {}
    std::string getName() { return mName; }
    ItemType getType() { return mType; }
    int getMaxStackSize() { return mMaxStackSize; }
    int getMeleeDamage() { return mMeleeDamage; }
    int getDurability() { return mDurability; }
    char GetTextureChar() { return mTextureReference; };

protected:
    std::string mName;
    ItemType mType;
    int mMaxStackSize;
    int mMeleeDamage;
    int mDurability;
    char mTextureReference;
};