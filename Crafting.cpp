#include "Crafting.h"
#include "CubeGame.h"

void crafting::craftSticks(inventory& inv)
{
	bool wood = inv.invItems.find(items::wood)->second >= 2;

	if (wood)
	{
		inv.addItem(items::sticks, 6);
		inv.removeItem(items::wood, 2);
	}
}

void crafting::craftIronPick(inventory& inv)
{
	bool iron = inv.invItems.find(items::iron)->second >= 3;
	bool sticks = inv.invItems.find(items::sticks)->second >= 2;

	if (iron && sticks)
	{
		inv.addItem(items::pickaxe, 1);
		inv.removeItem(items::iron, 3);
		inv.removeItem(items::sticks, 2);
	}
}

void crafting::craftIronShovel(inventory& inv)
{
	bool iron = inv.invItems.find(items::iron)->second >= 1;
	bool sticks = inv.invItems.find(items::sticks)->second >= 2;

	if (iron && sticks)
	{
		inv.addItem(items::shovel, 1);
		inv.removeItem(items::iron, 1);
		inv.removeItem(items::sticks, 2);
	}
}

void crafting::craftIronSword(inventory& inv)
{
	bool iron = inv.invItems.find(items::iron)->second >= 2;
	bool sticks = inv.invItems.find(items::sticks)->second >= 1;

	if (iron && sticks)
	{
		inv.addItem(items::sword, 1);
		inv.removeItem(items::iron, 2);
		inv.removeItem(items::sticks, 1);
	}
}