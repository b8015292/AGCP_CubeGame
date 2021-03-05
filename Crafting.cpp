#include <algorithm>
#include <vector>

#include "Crafting.h"

crafting::crafting(Inventory &inv) : mInv(&inv), woodTool(25, 5, "Wood"), stoneTool(50, 10, "Stone"), ironTool(100, 15, "Iron")
{
	//Tools
	canCraftWoodSword = false;
	canCraftStoneSword = false;
	canCraftIronSword = false;
	canCraftWoodPick = false;
	canCraftStonePick = false;
	canCraftIronPick = false;
	//canCraftWoodShovel = false;
	//canCraftStoneShovel = false;
	//canCraftIronShovel = false;
	//Misc
	canCraftSticks = false;
	//canCraftTorch = false;
	
	tWood = 0;
	tStone = 0;
	tIron = 0;
	tSticks = 0;
	tCoal = 0;
}

void crafting::resetCrafting()
{
	//Tools
	canCraftWoodSword = false;
	canCraftStoneSword = false;
	canCraftIronSword = false;
	canCraftWoodPick = false;
	canCraftStonePick = false;
	canCraftIronPick = false;
	//canCraftWoodShovel = false;
	//canCraftStoneShovel = false;
	//canCraftIronShovel = false;
	//Misc
	canCraftSticks = false;
	//canCraftTorch = false;

	tWood = 0;
	tStone = 0;
	tIron = 0;
	tSticks = 0;
	tCoal = 0;
}

std::string crafting::GetCraftables()
{
	std::string craftables = "";
	char plus = mGUIElementTextureCharacters.at("+");
	char equals = mGUIElementTextureCharacters.at("=");


	std::for_each(mInv->getInventory().begin(), mInv->getInventory().end(), [&](invItem t) 
		{
			if (t.name == "woodBlock") tWood += t.stackSize;
			else if (t.name == "stoneBlock") tStone += t.stackSize;
			else if (t.name == "iron") tIron += t.stackSize;
			else if (t.name == "stick") tSticks += t.stackSize;
			else if (t.name == "coal") tCoal += t.stackSize;
		});
	
	if (tWood >= 2) {
		canCraftSticks = true;
		craftables += mGUIElementTextureCharacters.at("itm_oak_log") + mGUIElementTextureCharacters.at("+") + mGUIElementTextureCharacters.at("itm_oak_log") + mGUIElementTextureCharacters.at("=") + mGUIElementTextureCharacters.at("itm_stick");
	}

	if (tSticks >= 1)
	{
		if (tWood >= 2) {
			canCraftWoodSword = true;
		}

		if (tStone >= 2) {
			canCraftStoneSword = true;
		}
		
		if (tIron >= 2) {
			canCraftIronSword = true;
		}
		//if (tCoal >= 1) canCraftTorch = true;

		if (tSticks >= 2)
		{
			if (tWood >= 1)
			{
				//canCraftWoodShovel = true;
				if (tWood >= 3) {
					canCraftWoodPick = true;

				}
			}
			if (tStone >= 1)
			{
				//canCraftStoneShovel = true;
				if (tStone >= 3) {
					canCraftStonePick = true;
				}
			}
			if (tIron >= 1)
			{
				//canCraftIronShovel = true;
				if (tIron >= 3) {
					canCraftIronPick = true;
				}
			}
		}
	}

	return craftables;
}

void crafting::craftSticks()
{
	Stick tempStick(mGUIElementTextureCharacters.at("itm_stick"));
	int amount = 2;

	mInv->removeItemCraft("woodBlock", 2);
	mInv->addItem(tempStick, amount);
}

void crafting::craftTorch()
{
	Torch tempTorch(mGUIElementTextureCharacters.at("NULL1"));;
	int amount = 1;

	mInv->removeItemCraft("stick", 1);
	mInv->removeItemCraft("coal", 1);
	mInv->addItem(tempTorch, amount);
}

void crafting::craftPick(ItemMaterial mat)
{
	int amount = 1;

	if (mat == ItemMaterial::WOOD)
	{
		Pickaxe tempPick(woodTool, mGUIElementTextureCharacters.at("itm_pickaxe_wood"));

		mInv->removeItemCraft("stick", 2);
		mInv->removeItemCraft("woodBlock", 3);
		mInv->addItem(tempPick, amount);
	}
	else if (mat == ItemMaterial::STONE)
	{
		Pickaxe tempPick(stoneTool, mGUIElementTextureCharacters.at("itm_pickaxe_stone"));

		mInv->removeItemCraft("stick", 2);
		mInv->removeItemCraft("stoneBlock", 3);
		mInv->addItem(tempPick, amount);
	}
	else if (mat == ItemMaterial::IRON)
	{
		Pickaxe tempPick(ironTool, mGUIElementTextureCharacters.at("itm_pickaxe_iron"));

		mInv->removeItemCraft("stick", 2);
		mInv->removeItemCraft("iron", 3);
		mInv->addItem(tempPick, amount);
	}
}

void crafting::craftShovel(ItemMaterial mat)
{
	int amount = 1;

	if (mat == ItemMaterial::WOOD)
	{
		Shovel tempShovel(woodTool, mGUIElementTextureCharacters.at("NULL1"));

		mInv->removeItemCraft("stick", 2);
		mInv->removeItemCraft("woodBlock", 1);
		mInv->addItem(tempShovel, amount);
	}
	else if (mat == ItemMaterial::STONE)
	{
		Shovel tempShovel(stoneTool, mGUIElementTextureCharacters.at("NULL1"));

		mInv->removeItemCraft("stick", 2);
		mInv->removeItemCraft("stoneBlock", 1);
		mInv->addItem(tempShovel, amount);
	}
	else if (mat == ItemMaterial::IRON)
	{
		Shovel tempShovel(ironTool, mGUIElementTextureCharacters.at("NULL1"));

		mInv->removeItemCraft("stick", 2);
		mInv->removeItemCraft("iron", 1);
		mInv->addItem(tempShovel, amount);
	}
}

void crafting::craftSword(ItemMaterial mat)
{
	int amount = 1;

	if (mat == ItemMaterial::WOOD)
	{
		Sword tempSword(woodTool, mGUIElementTextureCharacters.at("itm_sword_wood"));

		mInv->removeItemCraft("stick", 1);
		mInv->removeItemCraft("woodBlock", 2);
		mInv->addItem(tempSword, amount);
	}
	else if (mat == ItemMaterial::STONE)
	{
		Sword tempSword(stoneTool, mGUIElementTextureCharacters.at("itm_sword_stone"));

		mInv->removeItemCraft("stick", 1);
		mInv->removeItemCraft("stoneBlock", 2);
		mInv->addItem(tempSword, amount);
	}
	else if (mat == ItemMaterial::IRON)
	{
		Sword tempSword(ironTool, mGUIElementTextureCharacters.at("itm_sword_iron"));

		mInv->removeItemCraft("stick", 1);
		mInv->removeItemCraft("iron", 2);
		mInv->addItem(tempSword, amount);
	}
}