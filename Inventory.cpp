#include <algorithm>

#include "Inventory.h"

void inventory::addItem(Item newItem)
{
	bool complete = false;
	std::string itemName = newItem.GetName();
	int amountToAdd = newItem.GetAmountInStack();
	std::for_each(mInvItems.begin(), mInvItems.end(), [&](std::pair<int, Item*> p)
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
	if (!complete)
	{
		mInvItems.insert(std::make_pair(mInvItems.size() + 1, &newItem));
	}
}


void inventory::removeItem(int spaceToDelete, bool deleteAll)
{
	std::map<int, Item*>::iterator it = mInvItems.find(spaceToDelete);
	int amountToDelete = 1;
	if (deleteAll)
	{
		amountToDelete = it->second->GetAmountInStack();
	}
	it->second->decreaseStack(amountToDelete);

	if (it->second->GetAmountInStack() <= 0)
	{
		mInvItems.erase(spaceToDelete);
		std::for_each(mInvItems.begin(), mInvItems.end(), [spaceToDelete](std::pair<int, Item*> p) { if (p.first > spaceToDelete) p.first - 1; });
	}
}