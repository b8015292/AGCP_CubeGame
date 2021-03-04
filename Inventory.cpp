#include <algorithm>
#include <assert.h>

#include "Inventory.h"

Inventory::Inventory(int hotbarSize)
{
	mHotbarSize = hotbarSize;

	mInventory.reserve(64);
	mHotbar.reserve(mHotbarSize);
}

void Inventory::invToHotbar(int spaceToMove)
{
	bool complete = false;
	mHotbarDirty = true;
	mInventoryDirty = true;
	int i(0);

	if (!fullHotbar())
	{
		if (mHotbar.size() != 0)
		{
			while (complete != true && i < mHotbar.size() - 1)
			{
				if ((mHotbar[i].name == mInventory[spaceToMove].name) && (!stackIsFull(true, i)))
				{
					int spaceLeft = mHotbar[i].maxStackSize - mHotbar[i].stackSize;

					if (spaceLeft >= mInventory[spaceToMove].stackSize)
					{
						mHotbar[i].stackSize += mInventory[spaceToMove].stackSize;
						removeItemFromInvClick(spaceToMove, true);
						complete = true;
					}
					else
					{
						mHotbar[i].stackSize += spaceLeft;
						mInventory[spaceToMove].stackSize -= spaceLeft;
					}
				}
				else
				{
					if (!fullHotbar())
					{
						mHotbar.push_back(mInventory[spaceToMove]);
						removeItemFromInvClick(spaceToMove, true);
						complete = true;
					}
				}

				++i;
			}
		}
		else 
		{
			mHotbar.push_back(mInventory[spaceToMove]);
			removeItemFromInvClick(spaceToMove, true);
		}
	}
}
void Inventory::hotbarToInv(int spaceToMove)
{
	bool complete = false;
	mHotbarDirty = true;
	mInventoryDirty = true;
	int i(0);

	if (!fullHotbar())
	{
		if (mInventory.size() != 0)
		{
			while (complete != true && i < mInventory.size() - 1)
			{
				if ((mInventory[i].name == mHotbar[spaceToMove].name) && (!stackIsFull(false, i)))
				{
					int spaceLeft = mInventory[i].maxStackSize - mHotbar[i].stackSize;

					if (spaceLeft >= mHotbar[spaceToMove].stackSize)
					{
						mInventory[i].stackSize += mHotbar[spaceToMove].stackSize;
						removeItemFromHotbarClick(spaceToMove, true);
						complete = true;
					}
					else
					{
						mInventory[i].stackSize += spaceLeft;
						mHotbar[spaceToMove].stackSize -= spaceLeft;
					}
				}
				else
				{
					if (!fullHotbar())
					{
						mInventory.push_back(mHotbar[spaceToMove]);
						removeItemFromHotbarClick(spaceToMove, true);
						complete = true;
					}
				}

				++i;
			}
		}
		else
		{
			mInventory.push_back(mHotbar[spaceToMove]);
			removeItemFromHotbarClick(spaceToMove, true);
		}
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
		std::for_each(mHotbar.begin(), mHotbar.end(), [&](invItem& item) 
			{
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
			});
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

		std::for_each(mInventory.begin(), mInventory.end(), [&](invItem& item) 
			{
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
			});
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
	int i = mInventory.size() - 1;

	while (i >= 0 && complete == false)
	{
		if (mInventory[i].name == itemName)
		{
			mInventoryDirty = true;
			if (amount >= mInventory[i].stackSize)
			{
				amount -= mInventory[i].stackSize;
				removeItemFromInvClick(i, true);
			}
			if (amount != 0)
			{
				mInventory[i].stackSize -= amount;
				amount = 0;
				complete = true;
			}
		}

		--i;
	}

	i = mHotbar.size() - 1;

	while (i >= 0 && complete == false)
	{
		if (mHotbar[i].name == itemName)
		{
			mHotbarDirty = true;
			if (amount >= mHotbar[i].stackSize)
			{
				amount -= mHotbar[i].stackSize;
				removeItemFromHotbarClick(i, true);
			}
			if (amount != 0)
			{
				mHotbar[i].stackSize -= amount;
				amount = 0;
				complete = true;
			}
		}

		--i;
	}

	assert(amount != 0);
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