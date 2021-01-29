#include <algorithm>

#include "Inventory.h"

void inventory::addItem(Item &newItem, const bool mainInv)
{
	std::map<int, Item*> tempInv;
	if (mainInv) tempInv = mInvItems;
	else tempInv = mHotbarItems;

	bool complete = false;

	std::string itemName = newItem.GetName();
	int amountToAdd = newItem.GetAmountInStack();
	std::for_each(tempInv.begin(), tempInv.end(), [&](std::pair<int, Item*> p)
	{
		if ((p.second->GetName() == itemName) && !complete)
		{
			if (!p.second->fullStack())
			{
				int spaceInInv = (p.second->GetMaxStackSize() - p.second->GetAmountInStack());
				if (amountToAdd < spaceInInv)
				{
					p.second->increaseStack(amountToAdd);
					complete = true;
					
				}
				else
				{
					amountToAdd = spaceInInv;
					newItem.decreaseStack(spaceInInv);
				}
			}
		}
	});
	if (!complete && (tempInv.end()->first < 64))
	{
		tempInv.insert(std::make_pair(tempInv.size() + 1, &newItem));
	}
}


void inventory::removeItem(const int spaceToDelete, const bool deleteAll, const bool mainInv)
{
	std::map<int, Item*> tempInv;
	if (mainInv) tempInv = mInvItems; 
	else tempInv = mHotbarItems;

	std::map<int, Item*>::iterator it = tempInv.find(spaceToDelete);
	int amountToDelete = 1;
	if (deleteAll)
	{
		amountToDelete = it->second->GetAmountInStack();
	}
	it->second->decreaseStack(amountToDelete);

	if (it->second->GetAmountInStack() <= 0)
	{
		tempInv.erase(spaceToDelete);
		std::for_each(tempInv.begin(), tempInv.end(), [spaceToDelete](std::pair<int, Item*> p) { if (p.first > spaceToDelete) p.first -= 1; });
	}
}

void inventory::moveItemHotbarToInv(const int pos, const bool movAll)
{
	if (mInvItems.end()->first < 64)
	{
		auto it = mHotbarItems.find(pos);
		mInvItems.insert(std::make_pair(mInvItems.end()->first + 1, it->second));
		removeItem(pos, movAll, false);
	}

}

void inventory::moveItemInvToHotbar(const int pos, const bool movAll)
{
	if (mHotbarItems.end()->first < 8)
	{
		auto it = mInvItems.find(pos);
		mInvItems.insert(std::make_pair(mHotbarItems.end()->first + 1, it->second));
		removeItem(pos, movAll, true);
	}
}