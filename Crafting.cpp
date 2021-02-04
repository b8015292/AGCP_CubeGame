#include <algorithm>
#include <vector>

#include "Crafting.h"
#include "Tool.h"
#include "Misc.h"

crafting::crafting(Inventory &inv) : mInv(&inv)
{
	//Tools
	canCraftWoodSword = false;
	canCraftStoneSword = false;
	canCraftIronSword = false;
	canCraftWoodPick = false;
	canCraftStonePick = false;
	canCraftIronPick = false;
	canCraftWoodShovel = false;
	canCraftStoneShovel = false;
	canCraftIronShovel = false;
	//Misc
	canCraftSticks = false;
	canCraftTorch = false;
	
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
	canCraftWoodShovel = false;
	canCraftStoneShovel = false;
	canCraftIronShovel = false;
	//Misc
	canCraftSticks = false;
	canCraftTorch = false;

	tWood = 0;
	tStone = 0;
	tIron = 0;
	tSticks = 0;
	tCoal = 0;
}

void crafting::checkCraftables()
{
	std::for_each(mInv->getInventory().begin(), mInv->getInventory().end(), [&](invItem t) 
		{
			if (t.name == "woodBlock") tWood += t.stackSize;
			else if (t.name == "stoneBlock") tStone += t.stackSize;
			else if (t.name == "iron") tIron += t.stackSize;
			else if (t.name == "stick") tSticks += t.stackSize;
			else if (t.name == "coal") tCoal += t.stackSize;
		});
	
	if (tWood >= 2) canCraftSticks = true;

	if (tSticks >= 1)
	{
		if (tWood >= 2) canCraftWoodSword = true;
		if (tStone >= 2) canCraftStoneSword = true;
		if (tIron >= 2) canCraftIronSword = true;

		if (tCoal >= 1) canCraftTorch = true;

		if (tSticks >= 2)
		{
			if (tWood >= 1)
			{
				canCraftWoodShovel = true;
				if (tWood >= 3) canCraftWoodPick = true;
			}
			if (tStone >= 1)
			{
				canCraftStoneShovel = true;
				if (tStone >= 3) canCraftStonePick = true;
			}
			if (tIron >= 1)
			{
				canCraftIronShovel = true;
				if (tIron >= 3) canCraftIronPick = true;
			}
		}
	}
}

void crafting::craftSticks()
{
	Stick tempStick;
	int amount = 2;

	mInv->removeItemCraft("woodBlock", 2);
	mInv->addItem(tempStick, amount);
}

void crafting::craftTorch()
{
	Torch tempTorch;
	int amount = 1;

	mInv->removeItemCraft("stick", 1);
	mInv->removeItemCraft("coal", 1);
	mInv->addItem(tempTorch, amount);
}

void crafting::craftPick(Material mat)
{
	int amount = 1;

	if (mat == Material::WOOD)
	{
		Pickaxe tempPick(WOOD);

		mInv->removeItemCraft("stick", 2);
		mInv->removeItemCraft("woodBlock", 3);
		mInv->addItem(tempPick, amount);
	}
	else if (mat == Material::STONE)
	{
		Pickaxe tempPick(STONE);

		mInv->removeItemCraft("stick", 2);
		mInv->removeItemCraft("stoneBlock", 3);
		mInv->addItem(tempPick, amount);
	}
	else if (mat == Material::IRON)
	{
		Pickaxe tempPick(IRON);

		mInv->removeItemCraft("stick", 2);
		mInv->removeItemCraft("iron", 3);
		mInv->addItem(tempPick, amount);
	}
}

void crafting::craftShovel(Material mat)
{
	int amount = 1;

	if (mat == Material::WOOD)
	{
		Shovel tempShovel(WOOD);

		mInv->removeItemCraft("stick", 2);
		mInv->removeItemCraft("woodBlock", 1);
		mInv->addItem(tempShovel, amount);
	}
	else if (mat == Material::STONE)
	{
		Shovel tempShovel(STONE);

		mInv->removeItemCraft("stick", 2);
		mInv->removeItemCraft("stoneBlock", 1);
		mInv->addItem(tempShovel, amount);
	}
	else if (mat == Material::IRON)
	{
		Shovel tempShovel(IRON);

		mInv->removeItemCraft("stick", 2);
		mInv->removeItemCraft("iron", 1);
		mInv->addItem(tempShovel, amount);
	}
}

void crafting::craftSword(Material mat)
{
	int amount = 1;

	if (mat == Material::WOOD)
	{
		Shovel tempSword(WOOD);

		mInv->removeItemCraft("stick", 1);
		mInv->removeItemCraft("woodBlock", 2);
		mInv->addItem(tempSword, amount);
	}
	else if (mat == Material::STONE)
	{
		Shovel tempSword(STONE);

		mInv->removeItemCraft("stick", 1);
		mInv->removeItemCraft("stoneBlock", 2);
		mInv->addItem(tempSword, amount);
	}
	else if (mat == Material::IRON)
	{
		Shovel tempSword(IRON);

		mInv->removeItemCraft("stick", 1);
		mInv->removeItemCraft("iron", 2);
		mInv->addItem(tempSword, amount);
	}
}