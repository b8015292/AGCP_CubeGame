#pragma once

#include "Item.h"
#include "Object.h"

static ToolType WOOD = ToolType(25, 1.5f);
static ToolType STONE = ToolType(25, 2.f);

class ToolType {
public:
    ToolType(short int durability, int speed) { mDurability = durability; mSpeed = speed; }
    int GetDurability() { return mDurability; }
    float GetSpeed() { return mSpeed; }
private:
    short int mDurability;
    float mSpeed;
};

class Block : public Item {
public:
    Block(std::string name, ItemType type, short int maxStackSize, blockType blockType) : mBlockType(blockType), Item(name, type, maxStackSize) {}
protected:
    blockType mBlockType;
};