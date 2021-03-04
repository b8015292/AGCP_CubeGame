#pragma once

#include <map>
#include <unordered_map>

#include "Inventory.h"
#include "Item.h"

enum class ItemMaterial {
	WOOD,
	STONE,
	IRON
};

class crafting 
{
public:
	crafting(Inventory& inv, std::unordered_map<std::string, char>& textureCharacters);
	crafting(const crafting&) = delete;
	crafting& operator=(const crafting&) = delete;

	void resetCrafting();
	void checkCraftables();

	void craftSticks();
	void craftTorch();
	void craftPick(ItemMaterial mat);
	void craftShovel(ItemMaterial mat);
	void craftSword(ItemMaterial mat);

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
	std::unordered_map<std::string, char> mGUIElementTextureCharacters;
	//Temporary variables to store amount of items for crafting
	int tWood, tStone, tIron, tSticks, tCoal;

	//int tWoodSpot, tStoneSpot, tIronSpot, tSticksSpot, tCoalSpot;
};