#pragma once
#include <vector>

#include "Item.h"

struct invItem {
	std::string name;
	int maxStackSize;
	int stackSize;
	int durability;
	bool full = false;
};

class Inventory
{
public:
	Inventory();

	std::vector<invItem> getInventory() { return mInventory; }
	std::vector<invItem> getHotbar() { return mHotbar; }

	bool fullInventory() { return mInventory.size() == 64; }
	bool fullHotbar() { return mHotbar.size() == 8; }

	void invToHotbar(int spaceToMove);
	void hotbarToInv(int spaceToMove);
	void addItem(Item newItem, int &amount);
	void removeItemCraft(std::string itemName, int name);
	void removeItemFromInvClick(int spaceToRemove, bool all);
	void removeItemFromHotbarClick(int spaceToRemove, bool all);

private:
	std::vector<invItem> mInventory;
	std::vector<invItem> mHotbar;
};