#pragma once

#include <string>

enum class ItemType {
    TOOL,
    MISC,
    BLOCK
};

class ToolType
{
public:
    ToolType(int durability, int damageMultiplier, std::string typeName) : mDurability(durability), mDamageMultiplier(damageMultiplier), mTypeName(typeName) {}
    int getDamageMultiplier() { return mDamageMultiplier; }
    int getDurability() { return mDurability; }
    std::string getTypeName() { return mTypeName; }
private:
    int mDurability;
    int mDamageMultiplier;
    std::string mTypeName;
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

//Block Items

class DirtBlock : public Item
{
public:
    DirtBlock(char texRef) : Item("dirtBlock", ItemType::BLOCK, 64, 1, 1, texRef) {}
};

class StoneBlock : public Item
{
public:
    StoneBlock(char texRef) : Item("stoneBlock", ItemType::BLOCK, 64, 1, 1, texRef) {}
};

class WoodBlock : public Item
{
public:
    WoodBlock(char texRef) : Item("woodBlock", ItemType::BLOCK, 64, 1, 1, texRef) {}
};

class CoalBlock : public Item
{
public:
    CoalBlock(char texRef) : Item("coalBlock", ItemType::BLOCK, 64, 1, 1, texRef) {}
};

class IronBlock : public Item
{
public:
    IronBlock(char texRef) : Item("IronBlock", ItemType::BLOCK, 64, 1, 1, texRef) {}
};

// Misc Items

class Coal : public Item
{
public:
    Coal(char texRef) : Item("coal", ItemType::MISC, 64, 1, 1, texRef) {}
};

class Iron : public Item
{
    Iron(char texRef) : Item("iron", ItemType::MISC, 64, 1, 1, texRef) {}
};

class Stick : public Item
{
public:
    Stick(char texRef) : Item("stick", ItemType::MISC, 64, 1, 1, texRef) {}
};

class Torch : public Item
{
public:
    Torch(char texRef) : Item("torch", ItemType::MISC, 64, 1, 1, texRef) {}
};

// Tool Items

class Pickaxe : public Item
{
public:
    Pickaxe(ToolType toolType, char texRef) : Item(toolType.getTypeName() + "Pickaxe", ItemType::TOOL, 1, toolType.getDamageMultiplier(), toolType.getDurability(), texRef) {}
};

class Shovel : public Item
{
public:
    Shovel(ToolType toolType, char texRef) : Item(toolType.getTypeName() + "Shovel", ItemType::TOOL, 1, toolType.getDamageMultiplier(), toolType.getDurability(), texRef) {}
};

class Sword : public Item
{
public:
    Sword(ToolType toolType, char texRef) : Item(toolType.getTypeName() + "Sword", ItemType::TOOL, 1, 3 * toolType.getDamageMultiplier(), toolType.getDurability(), texRef) {}
};