#pragma once

#include "Item.h"

static ToolType WOOD = ToolType(25, 1.5f);
static ToolType STONE = ToolType(60, 2.f);

class ToolType {
public:
    ToolType(short int durability, int speed) { mDurability = durability; mSpeed = speed; }
    int GetDurability() { return mDurability; }
    float GetSpeed() { return mSpeed; }
private:
    short int mDurability;
    float mSpeed;
};

class Tool : public Item {
public:
    Tool(std::string name, ItemType type, short int maxStackSize, ToolType toolType) : mToolType(toolType), mDurability(toolType.GetDurability()), Item(name, type, maxStackSize) {}
    int GetDurability() { return mDurability; }
protected:
    ToolType mToolType;
    int mDurability;
};