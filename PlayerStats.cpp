#include "PlayerStats.h"

void PlayerStats::increasePlayerHealth(int amount)
{
	if ((mPlayerMaxHealth - mPlayerHealth) <= amount) mPlayerHealth = 100;
	else mPlayerHealth += amount;
}

void PlayerStats::decreasePlayerHealth(int amount)
{
	if (mPlayerHealth > amount) mPlayerHealth -= amount;
	else mPlayerHealth = 0;
}