#include <algorithm>
#include <assert.h>

#include "Inventory.h"

Inventory::Inventory(int hotbarSize)
{
	mHotbarSize = hotbarSize;

	mInventory.reserve(64);
	mHotbar.reserve(mHotbarSize);
}

void Inventory::invToHotbar(int spaceToMove, bool all)
{
	bool complete = false;

	if (mHotbar.size() != 0 && mInventory.size() != 0)
	{
		mHotbarDirty = true;
		mInventoryDirty = true;
		for (int i = 0; i < mHotbar.size(); i++)
		{
			invItem item = mHotbar.at(i);
				if ((item.name == mInventory[spaceToMove].name) && (item.maxStackSize != item.stackSize) && !complete)
				{
					mHotbarDirty = true;
					mInventoryDirty = true;
					int spaceLeft = item.maxStackSize - item.stackSize;
					if (all)
					{
						if (spaceLeft >= mInventory[spaceToMove].stackSize)
						{
							item.stackSize += mInventory[spaceToMove].stackSize;
							complete = true;
							removeItemFromInvClick(spaceToMove, true);
						}
						else
						{
							item.stackSize += spaceLeft;
							mInventory[spaceToMove].stackSize -= spaceLeft;
						}
					}
					else
					{
						if (spaceLeft >= 1)
						{
							++item.stackSize;
							--mInventory[spaceToMove].stackSize;
							complete = true;
						}
					}
				}
		}

		if (!fullHotbar() && !complete)
		{
			mHotbarDirty = true;
			mInventoryDirty = true;
			mHotbar.push_back(mInventory[spaceToMove]);
			if (!all) mHotbar[mHotbar.size()].stackSize = 1;
			removeItemFromInvClick(spaceToMove, all);
			complete = true;
		}
	}
	else if (mInventory.size() != 0)
	{
		mHotbarDirty = true;
		mInventoryDirty = true;
		mHotbarDirty = true;
		mInventoryDirty = true;
		mHotbar.push_back(mInventory[spaceToMove]);
		if (!all) mHotbar[mHotbar.size()].stackSize = 1;
		removeItemFromInvClick(spaceToMove, all);
		complete = true;
	}
}
void Inventory::hotbarToInv(int spaceToMove, bool all)
{
	bool complete = false;

	//check if space is null

	if (mHotbar.size() != 0 && mInventory.size() != 0)
	{
		mHotbarDirty = true;
		mInventoryDirty = true;
		for (int i = 0; i < mInventory.size(); ++i)
		{
			invItem item = mInventory.at(i);
			if ((item.name == mHotbar[spaceToMove].name) && (item.maxStackSize != item.stackSize) && !complete)
			{
				mHotbarDirty = true;
				mInventoryDirty = true;
				int spaceLeft = item.maxStackSize - item.stackSize;
				if (all)
				{
					if (spaceLeft >= mHotbar[spaceToMove].stackSize)
					{
						item.stackSize += mHotbar[spaceToMove].stackSize;
						complete = true;
						removeItemFromHotbarClick(spaceToMove, all);
					}
					else
					{
						item.stackSize += spaceLeft;
						mHotbar[spaceToMove].stackSize -= spaceLeft;
					}
				}
				else
				{
					if (spaceLeft >= 1)
					{
						++item.stackSize;
						--mHotbar[spaceToMove].stackSize;
						complete = true;
					}
				}
			}
		}
		if (!fullInventory() && !complete)
		{
			mHotbarDirty = true;
			mInventoryDirty = true;
			mInventory.push_back(mHotbar[spaceToMove]);
			removeItemFromHotbarClick(spaceToMove, all);
			if (!all) mInventory[mInventory.size()].stackSize = 1;
			complete = true;
		}
	}
	else if(mHotbar.size() != 0)
	{
		mHotbarDirty = true;
		mInventoryDirty = true;
		mInventory.push_back(mHotbar[spaceToMove]);
		removeItemFromHotbarClick(spaceToMove, all);
		if (!all) mInventory[mInventory.size()].stackSize = 1;
		complete = true;
	}
}
void Inventory::addItem(Item newItem, int &amount)
{
	//Creating a local version of the item in the structure format to simplify the code
	invItem sNewItem;
	sNewItem.name = newItem.getName();
	sNewItem.stackSize = amount;
	sNewItem.maxStackSize = newItem.getMaxStackSize();
	sNewItem.durability = newItem.getDurability();
	sNewItem.mTextureReference = newItem.GetTextureChar();

	bool complete = false;
	
	// Try to add new Item to the hotbar first
	if (mHotbar.size() != 0)
	{
		for (int i = 0; i < mHotbar.size(); ++i)
		{
			invItem item = mHotbar.at(i);
			if ((item.name == sNewItem.name) && (item.maxStackSize != item.stackSize) && !complete)
			{
				mHotbarDirty = true;
				int spaceLeft = item.maxStackSize - item.stackSize;

				if (spaceLeft >= sNewItem.stackSize)
				{
					item.stackSize += sNewItem.stackSize;
					complete = true;
				}
				else
				{
					item.stackSize += spaceLeft;
					sNewItem.stackSize -= spaceLeft;
				}
			}
		}
		if (!fullHotbar() && !complete)
		{
			mHotbarDirty = true;
			mHotbar.push_back(sNewItem);
			complete = true;
		}
	}
	else if (!fullHotbar()) {
		mHotbar.push_back(sNewItem);
		complete = true;
		mHotbarDirty = true;
	}
	// If the hotbar cannot take the item, then attempt to move the item to the inventory
	if (mInventory.size() != 0 && !complete && !fullInventory())
	{
		for (int i = 0; i < mInventory.size(); ++i)
		{
			invItem item = mInventory.at(i);
			if ((item.name == sNewItem.name) && (item.maxStackSize != item.stackSize) && !complete)
			{
				mInventoryDirty = true;
				int spaceLeft = item.maxStackSize - item.stackSize;

				if (spaceLeft >= sNewItem.stackSize)
				{
					item.stackSize += sNewItem.stackSize;
					complete = true;
				}
				else
				{
					item.stackSize += spaceLeft;
					sNewItem.stackSize -= spaceLeft;
				}
			}
		}
		if (!fullInventory() && !complete)
		{
			mInventoryDirty = true;
			mInventory.push_back(sNewItem);
			complete = true;
		}
	}
	else if (!complete && !fullInventory()) {
		mInventory.push_back(sNewItem);
		complete = true;
		mInventoryDirty = true;
	}
}

void Inventory::removeItemFromInvClick(int spaceToRemove, bool all)
{
	mInventoryDirty = true;
	if (all) mInventory.erase(mInventory.begin() + spaceToRemove);
	else
	{
		mInventory[spaceToRemove].stackSize -= 1;
		if (mInventory[spaceToRemove].stackSize < 1) mInventory.erase(mInventory.begin() + spaceToRemove);
	}
}
void Inventory::removeItemFromHotbarClick(int spaceToRemove, bool all)
{
	mHotbarDirty = true;
	if (all) mHotbar.erase(mHotbar.begin() + spaceToRemove);
	else
	{
		mHotbar[spaceToRemove].stackSize -= 1;
		if (mHotbar[spaceToRemove].stackSize < 1) mHotbar.erase(mHotbar.begin() + spaceToRemove);
	}
}
void Inventory::removeItemCraft(std::string itemName, int amount)
{
	bool complete = false;

	for (int i = mInventory.size(); i > 0; --i)
	{
		invItem item = mInventory.at(i);
		if ((item.name == itemName) && (amount != 0))
		{
			mInventoryDirty = true;
			int temp = item.stackSize;
			item.stackSize -= amount;
			if (item.stackSize <= 0)
			{
				amount -= temp;
				removeItemFromInvClick(i, true);
			}
			else amount = 0;
		}
	}

	for (int i = mHotbar.size(); i > 0; --i)
	{
		invItem item = mHotbar.at(i);
		if ((item.name == itemName) && (amount != 0))
		{
			mHotbarDirty = true;
			int temp = item.stackSize;
			item.stackSize -= amount;
			if (item.stackSize <= 0)
			{
				amount -= temp;
				removeItemFromHotbarClick(i, true);
			}
			else amount = 0;
		}
	}
}

bool Inventory::stackIsFull(bool hotbar, int invSpace)
{
	bool full = false;
	if (hotbar)
	{
		if (mHotbar[invSpace].maxStackSize == mHotbar[invSpace].stackSize) full = true;
	}
	else
	{
		if (mInventory[invSpace].maxStackSize == mInventory[invSpace].stackSize) full = true;
	}

	return full;
}

bool Inventory::GetHotbarDirty() {
	if (mHotbarDirty) {
		mHotbarDirty = false;
		return true;
	}
	return false;
}
bool Inventory::GetInventoryDirty() {
	if (mInventoryDirty) {
		mInventoryDirty = false;
		return true;
	}
	return false;
}