#pragma once

#include "Item.h"

static ToolType WOOD = ToolType(25, 1.5f);
static ToolType STONE = ToolType(60, 2.f);

class ToolType {
public:
    ToolType(short int durability, int speedMultiplier) { mDurability = durability; mSpeedMultiplier = speedMultiplier; }
    float GetSpeedMultiplier() { return mSpeedMultiplier; }
    int GetDurability() { return mDurability; }
private:
    short int mDurability;
    float mSpeedMultiplier;
};

class Tool : public Item {
public:
    Tool(std::string name, ItemType type, short int maxStackSize, ToolType toolType) : mToolType(toolType), Item(name, type, maxStackSize, toolType.GetDurability()) {}
protected:
    ToolType mToolType;
};