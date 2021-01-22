//#include "Crafting.h"
//#include "CubeGame.h"
//
//void crafting::craftSticks(inventory& inv)
//{
//	bool wood = inv.mInvItems.find(Item)->second >= 2;
//
//	if (wood)
//	{
//		inv.addItem(Item, 6);
//		inv.removeItem(Item, 2);
//	}
//}
//
//void crafting::craftIronPick(inventory& inv)
//{
//	bool iron = inv.mInvItems.find(Item)->second >= 3;
//	bool sticks = inv.mInvItems.find(Item)->second >= 2;
//
//	if (iron && sticks)
//	{
//		inv.addItem(Item, 1);
//		inv.removeItem(Item, 3);
//		inv.removeItem(Item, 2);
//	}
//}
//
//void crafting::craftIronShovel(inventory& inv)
//{
//	bool iron = inv.mInvItems.find(Item)->second >= 1;
//	bool sticks = inv.mInvItems.find(Item)->second >= 2;
//
//	if (iron && sticks)
//	{
//		inv.addItem(items::shovel, 1);
//		inv.removeItem(items::iron, 1);
//		inv.removeItem(items::sticks, 2);
//	}
//}
//
//void crafting::craftIronSword(inventory& inv)
//{
//	bool iron = inv.mInvItems.find(items::iron)->second >= 2;
//	bool sticks = inv.mInvItems.find(items::sticks)->second >= 1;
//
//	if (iron && sticks)
//	{
//		inv.addItem(items::sword, 1);
//		inv.removeItem(items::iron, 2);
//		inv.removeItem(items::sticks, 1);
//	}
//}