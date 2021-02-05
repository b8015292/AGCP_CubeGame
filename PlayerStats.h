#pragma once

class PlayerStats
{
public:
	PlayerStats() : mPlayerHealth(100), mPlayerMaxHealth(100), mPlayerDamage(1), mDistanceTravelled(0), mEnemiesKilled(0), mFriendliesKilled(0) {}
	PlayerStats(const PlayerStats&) = delete;
	PlayerStats& operator=(const PlayerStats&) = delete;

	int getPlayerHealth() { return mPlayerHealth; }
	int getPlayerDamage() { return mPlayerDamage; }

	void increasePlayerDamage(int itemBonus) { mPlayerDamage += itemBonus; }
	void decreasePlayerDamage(int itemBonus) { mPlayerDamage -= itemBonus; }
	void increasePlayerHealth(int amount);
	void decreasePlayerHealth(int amount);

	int getDistanceTravelled() { return mDistanceTravelled; }
	int getEnemiesKilled() { return mEnemiesKilled; }
	int getFriendliesKilled() { return mFriendliesKilled; }

	void increaseDistanceTravelled() { ++mDistanceTravelled; }
	void increaseFriendliesKilled() { ++mFriendliesKilled; }
	void increaseEnemiesKilled() { ++mEnemiesKilled; }

private:
	//Stats that effect the game
	int mPlayerHealth;
	const int mPlayerMaxHealth;
	int mPlayerDamage;

	//Overall stats
	int mDistanceTravelled;
	int mEnemiesKilled;
	int mFriendliesKilled;
};