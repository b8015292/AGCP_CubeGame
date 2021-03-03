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
	int i(0);

	if (!fullHotbar())
	{
		if (mHotbar.size() != 0)
		{
			while (complete != true && i < mHotbar.size() - 1)
			{
				if ((mHotbar[i].name == mInventory[spaceToMove].name) && (mHotbar[i].full == false))
				{
					int spaceLeft = mHotbar[i].maxStackSize - mHotbar[i].stackSize;

					if (spaceLeft >= mInventory[spaceToMove].stackSize)
					{
						mHotbar[i].stackSize += mInventory[spaceToMove].stackSize;
						removeItemFromInvClick(spaceToMove, true);
						complete = true;

						if (mHotbar[i].stackSize == mHotbar[i].maxStackSize) mHotbar[i].full = true;
					}
					else
					{
						mHotbar[i].stackSize += spaceLeft;
						mHotbar[i].full = true;
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
	int i(0);

	if (!fullHotbar())
	{
		if (mInventory.size() != 0)
		{
			while (complete != true && i < mInventory.size() - 1)
			{
				if ((mInventory[i].name == mHotbar[spaceToMove].name) && (mInventory[i].full == false))
				{
					int spaceLeft = mInventory[i].maxStackSize - mHotbar[i].stackSize;

					if (spaceLeft >= mHotbar[spaceToMove].stackSize)
					{
						mInventory[i].stackSize += mHotbar[spaceToMove].stackSize;
						removeItemFromHotbarClick(spaceToMove, true);
						complete = true;

						if (mInventory[i].stackSize == mInventory[i].maxStackSize) mInventory[i].full = true;
					}
					else
					{
						mInventory[i].stackSize += spaceLeft;
						mInventory[i].full = true;
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
	//Creating a local versino of the item in the structure format to simplify the code
	invItem sNewItem;
	sNewItem.name = newItem.getName();
	sNewItem.stackSize = amount;
	sNewItem.maxStackSize = newItem.getMaxStackSize();
	sNewItem.durability = newItem.getDurability();

	bool complete = false;
	int i(0);
	
	// Try to add new Item to the hotbar first
	if (mHotbar.size() != 0)
	{
		while (complete != true && i < mHotbar.size() - 1)
		{
			if ((mHotbar[i].name == sNewItem.name) && (mHotbar[i].full == false))
			{
				int spaceLeft = mHotbar[i].maxStackSize - mHotbar[i].stackSize;

				if (spaceLeft >= sNewItem.stackSize)
				{
					mHotbar[i].stackSize += sNewItem.stackSize;
					complete = true;

					if (mHotbar[i].stackSize == mHotbar[i].maxStackSize) mHotbar[i].full = true;
				}
				else
				{
					mHotbar[i].stackSize += spaceLeft;
					mHotbar[i].full = true;
					sNewItem.stackSize -= spaceLeft;
				}
			}
			else
			{
				if (!fullHotbar())
				{
					mHotbar.push_back(sNewItem);
					complete = true;
				}
			}

			++i;
		}
	}
	else mHotbar.push_back(sNewItem);

	// If the hotbar cannot take the item, then attempt to move the item to the inventory
	if (mInventory.size() != 0 && !complete)
	{
		while (complete != true && i < mInventory.size() - 1)
		{
			if ((mInventory[i].name == sNewItem.name) && (mInventory[i].full == false))
			{
				int spaceLeft = mInventory[i].maxStackSize - mInventory[i].stackSize;

				if (spaceLeft >= sNewItem.stackSize)
				{
					mInventory[i].stackSize += sNewItem.stackSize;
					complete = true;

					if (mInventory[i].stackSize == mInventory[i].maxStackSize) mInventory[i].full = true;
				}
				else
				{
					mInventory[i].stackSize += spaceLeft;
					mInventory[i].full = true;
					sNewItem.stackSize -= spaceLeft;
				}
			}
			else
			{
				if (!fullInventory())
				{
					mInventory.push_back(sNewItem);
					complete = true;
				}
			}
		}

		++i;
	}
	else if (!complete) mInventory.push_back(sNewItem);
}

void Inventory::removeItemFromInvClick(int spaceToRemove, bool all)
{
	if (all) mInventory.erase(mInventory.begin() + spaceToRemove);
	else
	{
		mInventory[spaceToRemove].stackSize -= 1;
		if (mInventory[spaceToRemove].stackSize < 1) mInventory.erase(mInventory.begin() + spaceToRemove);
	}
}
void Inventory::removeItemFromHotbarClick(int spaceToRemove, bool all)
{
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