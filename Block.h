#pragma once

#include "Item.h"

class DirtBlock : public Item 
{
public:
	DirtBlock() : Item("dirtBlock", ItemType::BLOCK, 64, 1, 1) {}
};

class StoneBlock : public Item
{
public:
	StoneBlock() : Item("stoneBlock", ItemType::BLOCK, 64, 1, 1) {}
};

class WoodBlock : public Item
{
public:
	WoodBlock() : Item("woodBlock", ItemType::BLOCK, 64, 1, 1) {}
};

class CoalBlock : public Item
{
public:
	CoalBlock() : Item("coalBlock", ItemType::BLOCK, 64, 1, 1) {}
};

class IronBlock : public Item
{
public:
	IronBlock() : Item("IronBlock", ItemType::BLOCK, 64, 1, 1) {}
};