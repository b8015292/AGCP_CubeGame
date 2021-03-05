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
	tIronOre = 0;
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
	tIronOre = 0;
	mCraftables.clear();
}

std::string crafting::GetCraftables()
{
	std::string craftables = "";
	char plus = mGUIElementTextureCharacters.at("+");
	char equals = mGUIElementTextureCharacters.at("=");

	resetCrafting();

	for (int i = 0; i < mInv->getInventory().size(); i++) {
		invItem t = mInv->getInventory().at(i);
		if (t.name == "oak_log") tWood += t.stackSize;
		else if (t.name == "stone") tStone += t.stackSize;
		else if (t.name == "iron") tIron += t.stackSize;
		else if (t.name == "stick") tSticks += t.stackSize;
		else if (t.name == "coal_ore") tCoal += t.stackSize;
		else if (t.name == "iron_ore") tIronOre += t.stackSize;
	}

	for (int i = 0; i < mInv->getHotbar().size(); i++) {
		invItem t = mInv->getHotbar().at(i);
		if (t.name == "oak_log") tWood += t.stackSize;
		else if (t.name == "stone") tStone += t.stackSize;
		else if (t.name == "iron") tIron += t.stackSize;
		else if (t.name == "stick") tSticks += t.stackSize;
		else if (t.name == "coal_ore") tCoal += t.stackSize;
		else if (t.name == "iron_ore") tIronOre += t.stackSize;
	}
	
	if (tWood >= 2) {
		canCraftSticks = true;
		mCraftables.push_back("itm_stick");

		craftables.push_back(mGUIElementTextureCharacters.at("itm_oak_log"));
		craftables.push_back(mGUIElementTextureCharacters.at("+"));
		craftables.push_back(mGUIElementTextureCharacters.at("itm_oak_log"));
		craftables.push_back(mGUIElementTextureCharacters.at("="));
		craftables.push_back(mGUIElementTextureCharacters.at("itm_stick"));
	}

	if (tCoal > 0 && tIronOre > 0) {
		canCraftIron = true;
		mCraftables.push_back("itm_iron");
		craftables.push_back(mGUIElementTextureCharacters.at("itm_coal_ore"));
		craftables.push_back(mGUIElementTextureCharacters.at("+"));
		craftables.push_back(mGUIElementTextureCharacters.at("itm_iron_ore"));
		craftables.push_back(mGUIElementTextureCharacters.at("="));
		craftables.push_back(mGUIElementTextureCharacters.at("itm_iron"));
	}

	if (tSticks >= 1)
	{
		if (tIron >= 1) {
			canCraftIronSword = true;
			mCraftables.push_back("itm_sword_iron");
			craftables.push_back(mGUIElementTextureCharacters.at("itm_stick"));
			craftables.push_back(mGUIElementTextureCharacters.at("+"));
			craftables.push_back(mGUIElementTextureCharacters.at("itm_iron"));
			craftables.push_back(mGUIElementTextureCharacters.at("="));
			craftables.push_back(mGUIElementTextureCharacters.at("itm_sword_iron"));
		}
		else if (tStone >= 1) {
			canCraftStoneSword = true;
			mCraftables.push_back("itm_sword_stone");
			craftables.push_back(mGUIElementTextureCharacters.at("itm_stick"));
			craftables.push_back(mGUIElementTextureCharacters.at("+"));
			craftables.push_back(mGUIElementTextureCharacters.at("itm_stone"));
			craftables.push_back(mGUIElementTextureCharacters.at("="));
			craftables.push_back(mGUIElementTextureCharacters.at("itm_sword_stone"));
		} 
		else if (tWood >= 1) {
			canCraftWoodSword = true;
			mCraftables.push_back("itm_sword_wood");
			craftables.push_back(mGUIElementTextureCharacters.at("itm_stick"));
			craftables.push_back(mGUIElementTextureCharacters.at("+"));
			craftables.push_back(mGUIElementTextureCharacters.at("itm_oak_log"));
			craftables.push_back(mGUIElementTextureCharacters.at("="));
			craftables.push_back(mGUIElementTextureCharacters.at("itm_sword_wood"));
		}

		
		

		//if (tCoal >= 1) canCraftTorch = true;

		if (tSticks >= 1)
		{
			if (tIron >= 1)
			{
				////canCraftIronShovel = true;
				//if (tIron >= 3) {
				canCraftIronPick = true;
				mCraftables.push_back("itm_pickaxe_iron");
				craftables.push_back(mGUIElementTextureCharacters.at("itm_stick"));
				craftables.push_back(mGUIElementTextureCharacters.at("+"));
				craftables.push_back(mGUIElementTextureCharacters.at("itm_iron"));
				craftables.push_back(mGUIElementTextureCharacters.at("="));
				craftables.push_back(mGUIElementTextureCharacters.at("itm_pickaxe_iron"));
				//}
			}
			else if (tStone >= 1)
			{
				////canCraftStoneShovel = true;
				//if (tStone >= 3) {
				canCraftStonePick = true;
				mCraftables.push_back("itm_pickaxe_stone");
				craftables.push_back(mGUIElementTextureCharacters.at("itm_stick"));
				craftables.push_back(mGUIElementTextureCharacters.at("+"));
				craftables.push_back(mGUIElementTextureCharacters.at("itm_stone"));
				craftables.push_back(mGUIElementTextureCharacters.at("="));
				craftables.push_back(mGUIElementTextureCharacters.at("itm_pickaxe_stone"));
				//}
			}
			else if (tWood >= 1)
			{
				////canCraftWoodShovel = true;
				//if (tWood >= 3) {
				canCraftWoodPick = true;
				mCraftables.push_back("itm_pickaxe_wood");
				craftables.push_back(mGUIElementTextureCharacters.at("itm_stick"));
				craftables.push_back(mGUIElementTextureCharacters.at("+"));
				craftables.push_back(mGUIElementTextureCharacters.at("itm_oak_log"));
				craftables.push_back(mGUIElementTextureCharacters.at("="));
				craftables.push_back(mGUIElementTextureCharacters.at("itm_pickaxe_wood"));
				//}
			}


		}
	}

	return craftables;
}

void crafting::Craft(int pos) {
	if (pos < mCraftables.size()) {
		const std::string chosen = mCraftables.at(pos);
		
		if (chosen == "itm_stick") {
			craftSticks();
		}
		else if (chosen == "itm_iron") {
			craftIron();
		}
		else if (chosen == "itm_sword_iron") {
			craftSword(ItemMaterial::IRON);
		}
		else if (chosen == "itm_sword_stone") {
			craftSword(ItemMaterial::STONE);
		}
		else if (chosen == "itm_sword_wood") {
			craftSword(ItemMaterial::WOOD);
		}
		else if (chosen == "itm_pickaxe_iron") {
			craftPick(ItemMaterial::IRON);
		}
		else if (chosen == "itm_pickaxe_stone") {
			craftPick(ItemMaterial::STONE);
		}
		else if (chosen == "itm_pickaxe_wood") {
			craftPick(ItemMaterial::WOOD);
		}


	}
}

void crafting::craftSticks()
{
	Stick tempStick(mGUIElementTextureCharacters.at("itm_stick"));
	int amount = 2;

	mInv->removeItemCraft("oak_log", 2);
	mInv->addItem(tempStick, amount);
}

void crafting::craftIron()
{
	Stick tempStick(mGUIElementTextureCharacters.at("itm_iron"));
	int amount = 1;

	mInv->removeItemCraft("iron_ore", 1);
	mInv->removeItemCraft("coal_ore", 1);
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

		mInv->removeItemCraft("stick", 1);
		mInv->removeItemCraft("oak_log", 1);
		mInv->addItem(tempPick, amount);
	}
	else if (mat == ItemMaterial::STONE)
	{
		Pickaxe tempPick(stoneTool, mGUIElementTextureCharacters.at("itm_pickaxe_stone"));

		mInv->removeItemCraft("stick", 1);
		mInv->removeItemCraft("stone", 1);
		mInv->addItem(tempPick, amount);
	}
	else if (mat == ItemMaterial::IRON)
	{
		Pickaxe tempPick(ironTool, mGUIElementTextureCharacters.at("itm_pickaxe_iron"));

		mInv->removeItemCraft("stick", 1);
		mInv->removeItemCraft("iron", 1);
		mInv->addItem(tempPick, amount);
	}
}

void crafting::craftShovel(ItemMaterial mat)
{
	int amount = 1;

	if (mat == ItemMaterial::WOOD)
	{
		Shovel tempShovel(woodTool, mGUIElementTextureCharacters.at("NULL1"));

		mInv->removeItemCraft("stick", 1);
		mInv->removeItemCraft("oak_log", 1);
		mInv->addItem(tempShovel, amount);
	}
	else if (mat == ItemMaterial::STONE)
	{
		Shovel tempShovel(stoneTool, mGUIElementTextureCharacters.at("NULL1"));

		mInv->removeItemCraft("stick", 1);
		mInv->removeItemCraft("stoneBlock", 1);
		mInv->addItem(tempShovel, amount);
	}
	else if (mat == ItemMaterial::IRON)
	{
		Shovel tempShovel(ironTool, mGUIElementTextureCharacters.at("NULL1"));

		mInv->removeItemCraft("stick", 1);
		mInv->removeItemCraft("oak_log", 1);
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
		mInv->removeItemCraft("oak_log", 1);
		mInv->addItem(tempSword, amount);
	}
	else if (mat == ItemMaterial::STONE)
	{
		Sword tempSword(stoneTool, mGUIElementTextureCharacters.at("itm_sword_stone"));

		mInv->removeItemCraft("stick", 1);
		mInv->removeItemCraft("stone", 1);
		mInv->addItem(tempSword, amount);
	}
	else if (mat == ItemMaterial::IRON)
	{
		Sword tempSword(ironTool, mGUIElementTextureCharacters.at("itm_sword_iron"));

		mInv->removeItemCraft("stick", 1);
		mInv->removeItemCraft("iron", 1);
		mInv->addItem(tempSword, amount);
	}
}