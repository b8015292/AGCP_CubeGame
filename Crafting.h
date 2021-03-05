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

//ToolType woodTool(25, 5, "Wood");
//ToolType stoneTool(50, 10, "Stone");
//ToolType ironTool(100, 15, "Iron");

class crafting 
{
public:
	crafting(Inventory& inv);
	crafting(const crafting&) = delete;
	crafting& operator=(const crafting&) = delete;

	void SetTexChars(std::unordered_map<std::string, char>& texChars) { mGUIElementTextureCharacters = texChars; };

	void resetCrafting();
	std::string GetCraftables();

	void craftSticks();
	void craftTorch();
	void craftPick(ItemMaterial mat);
	void craftShovel(ItemMaterial mat);
	void craftSword(ItemMaterial mat);

	//Tools that can be crafted
	bool canCraftWoodSword, canCraftStoneSword,
		canCraftIronSword, canCraftWoodPick,
		canCraftStonePick, canCraftIronPick;// ,
		//canCraftWoodShovel, canCraftStoneShovel,
		//canCraftIronShovel;
	//Misc that can be crafted
	bool canCraftSticks;// , canCraftTorch;

	ToolType woodTool;
	ToolType stoneTool;
	ToolType ironTool;

private:

	Inventory* mInv;
	std::unordered_map<std::string, char> mGUIElementTextureCharacters;
	//Temporary variables to store amount of items for crafting
	int tWood, tStone, tIron, tSticks, tCoal;

	//int tWoodSpot, tStoneSpot, tIronSpot, tSticksSpot, tCoalSpot;
};