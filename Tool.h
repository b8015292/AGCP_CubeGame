#pragma once
#include "Item.h"

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

ToolType WOOD(25, 5, "Wood");
ToolType STONE(50, 10, "Stone");
ToolType IRON(100, 15, "Iron");

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