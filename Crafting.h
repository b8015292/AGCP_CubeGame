//#pragma once
//
//#include "GameData.h"
//#include "Inventory.h"
//#include "Item.h"
//
//class crafting
//{
//public:
//	crafting();
//	std::vector<Item> getCraftables() { return mCraftables; };
//	void checkCraftables(inventory& inv);
//	void resetCrafting();
//
//	void craftSticks(inventory& inv);
//	void craftPick(inventory& inv, int type);
//	void craftShovel(inventory& inv, int type);
//	void craftSword(inventory& inv, int type);
//private:
//	std::vector<Item> mCraftables;
//	std::map<Type, int> mInvCraftingItems;
//
//	bool canCraftWoodSword, canCraftStoneSword,
//	canCraftIronSword, canCraftWoodPick,
//	canCraftStonePick, canCraftIronPick, 
//	canCraftWoodShovel, canCraftStoneShovel,
//	canCraftIronShovel, canCraftSticks;
//
//	int tWood, tStone, tIron, tSticks, tCoal;
//	int tWoodSpot, tStoneSpot, tIronSpot, tSticksSpot, tCoalSpot;
//};