#pragma once

#include <map>

#include "Inventory.h"
#include "Item.h"

enum class Material {
	WOOD,
	STONE,
	IRON
};

class crafting 
{
public:
	crafting(Inventory& inv);
	crafting(const crafting&) = delete;
	crafting& operator=(const crafting&) = delete;

	void resetCrafting();
	void checkCraftables();

	void craftSticks();
	void craftTorch();
	void craftPick(Material mat);
	void craftShovel(Material mat);
	void craftSword(Material mat);

	//Tools that can be crafted
	bool canCraftWoodSword, canCraftStoneSword,
		canCraftIronSword, canCraftWoodPick,
		canCraftStonePick, canCraftIronPick,
		canCraftWoodShovel, canCraftStoneShovel,
		canCraftIronShovel;
	//Misc that can be crafted
	bool canCraftSticks, canCraftTorch;

private:

	Inventory* mInv;

	//Temporary variables to store amount of items for crafting
	int tWood, tStone, tIron, tSticks, tCoal;

	//int tWoodSpot, tStoneSpot, tIronSpot, tSticksSpot, tCoalSpot;
};