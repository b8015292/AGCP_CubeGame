//#include "Crafting.h"
//#include "CubeGame.h"
//
//crafting::crafting()
//{
//	mInvCraftingItems.insert(std::make_pair(Type::Wood, 0));
//	mInvCraftingItems.insert(std::make_pair(Type::Stone, 0));
//	mInvCraftingItems.insert(std::make_pair(Type::Coal, 0));
//	mInvCraftingItems.insert(std::make_pair(Type::Iron, 0));
//	mInvCraftingItems.insert(std::make_pair(Type::Stick, 0));
//
//	canCraftWoodSword = false;
//	canCraftStoneSword = false;
//	canCraftIronSword = false;
//	canCraftWoodPick = false;
//	canCraftStonePick = false;
//	canCraftIronPick = false;
//	canCraftWoodShovel = false;
//	canCraftStoneShovel = false;
//	canCraftIronShovel = false;
//	canCraftSticks = false;
//	
//	tWood = 0;
//	tStone = 0;
//	tIron = 0;
//	tSticks = 0;
//	tCoal = 0;
//}
//
//void crafting::resetCrafting()
//{
//	canCraftWoodSword = false;
//	canCraftStoneSword = false;
//	canCraftIronSword = false;
//	canCraftWoodPick = false;
//	canCraftStonePick = false;
//	canCraftIronPick = false;
//	canCraftWoodShovel = false;
//	canCraftStoneShovel = false;
//	canCraftIronShovel = false;
//	canCraftSticks = false;
//
//	tWood = 0;
//	tStone = 0;
//	tIron = 0;
//	tSticks = 0;
//	tCoal = 0;
//}
//
//void crafting::checkCraftables(inventory& inv)
//{
//	Type tempType;
//
//	std::for_each(inv.getItems().begin(), inv.getItems().end(), [&](std::pair<int, Item*> p)
//		{
//			int numToAdd = p.second->GetAmountInStack();
//			tempType = p.second->GetName();
//			std::map<Type, int>::iterator it = mInvCraftingItems.find(tempType);
//			if (it != mInvCraftingItems.end())
//			{
//				switch (tempType) {
//				case Type::Stick:
//					tSticks += numToAdd;
//					tSticksSpot = p.first;
//					break;
//				case Type::Wood:
//					tWood += numToAdd;
//					tWoodSpot = p.first;
//					break;
//				case Type::Stone:
//					tStone += numToAdd;
//					tStoneSpot = p.first;
//					break;
//				case Type::Iron:
//					tIron += numToAdd;
//					tIronSpot = p.first;
//					break;
//				case Type::Coal:
//					tCoal += numToAdd;
//					tCoalSpot = p.first;
//					break;
//				default:
//					break;
//				}
//			}
//
//		});
//	if (tWood >= 2)
//	{
//		canCraftSticks = true;
//	}
//	if (tSticks >= 1)
//	{
//		if (tSticks >= 2)
//		{
//			if (tWood >= 1)
//			{
//				canCraftWoodShovel = true;
//				if (tWood >= 3)
//				{
//					canCraftWoodPick = true;
//				}
//			}
//			if (tStone >= 1)
//			{
//				canCraftStoneShovel = true;
//				if (tStone >= 3)
//				{
//					canCraftStonePick = true;
//				}
//			}
//			if (tIron >= 1)
//			{
//				canCraftIronShovel = true;
//				if (tIron >= 3)
//				{
//					canCraftIronPick = true;
//				}
//			}
//		}
//		if (tWood >= 2)
//		{
//			canCraftWoodSword = true;
//		}
//		if (tStone >= 2)
//		{
//			canCraftStoneSword = true;
//		}
//		if (tIron >= 2)
//		{
//			canCraftIronSword = true;
//		}
//	}
//}
//
//void crafting::craftSticks(inventory& inv)
//{
//	Item newItem(Type::Stick, ItemType::TOOL, 64, 1);
//	newItem.increaseStack(4);
//	inv.removeItem(tWoodSpot, true, true);
//	inv.addItem(newItem, true);
//}
//
//void crafting::craftPick(inventory& inv, int type)
//{
//	Type newType;
//	int typeSpot;
//	switch (type)
//	{
//	case 1:
//		newType = Type::WoodPick;
//		typeSpot = tWoodSpot;
//		break;
//	case 2:
//		newType = Type::StonePick;
//		typeSpot = tStoneSpot;
//		break;
//	case 3:
//		newType = Type::IronPick;
//		typeSpot = tIronSpot;
//		break;
//	}
//	Item newItem(newType, ItemType::TOOL, 64, 1);
//	newItem.increaseStack(1);
//	inv.removeItem(typeSpot, true, true);
//	inv.removeItem(tSticksSpot, true, true);
//	inv.addItem(newItem, true);
//}
//
//void crafting::craftShovel(inventory& inv, int type)
//{
//	Type newType;
//	int typeSpot;
//	switch (type)
//	{
//	case 1:
//		newType = Type::WoodShovel;
//		typeSpot = tWoodSpot;
//		break;
//	case 2:
//		newType = Type::StoneShovel;
//		typeSpot = tStoneSpot;
//		break;
//	case 3:
//		newType = Type::IronShovel;
//		typeSpot = tIronSpot;
//		break;
//	}
//	Item newItem(newType, ItemType::TOOL, 64, 1);
//	newItem.increaseStack(1);
//	inv.removeItem(typeSpot, true, true);
//	inv.removeItem(tSticksSpot, true, true);
//	inv.addItem(newItem, true);
//}
//
//void crafting::craftSword(inventory& inv, int type)
//{
//	Type newType;
//	int typeSpot;
//	switch (type)
//	{
//	case 1:
//		newType = Type::WoodSword;
//		typeSpot = tWoodSpot;
//		break;
//	case 2:
//		newType = Type::StoneSword;
//		typeSpot = tStoneSpot;
//		break;
//	case 3:
//		newType = Type::IronSword;
//		typeSpot = tIronSpot;
//		break;
//	}
//	Item newItem(newType, ItemType::TOOL, 64, 1);
//	newItem.increaseStack(1);
//	inv.removeItem(typeSpot, false, true);
//	inv.removeItem(tSticksSpot, false, true);
//	inv.addItem(newItem, true);
//}