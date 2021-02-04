#pragma once
#include "Item.h"

class Coal : public Item
{
public:
	Coal() : Item("coal", ItemType::MISC, 64, 1, 1) {}
};

class Iron : public Item
{
	Iron() : Item("iron", ItemType::MISC, 64, 1, 1) {}
};

class Stick : public Item
{
public:
	Stick() : Item("stick", ItemType::MISC, 64, 1, 1) {}
};

class Torch : public Item
{
public:
	Torch() : Item("torch", ItemType::MISC, 64, 1, 1) {}
};