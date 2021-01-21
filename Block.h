#pragma once

#include "Item.h"

class Block : public Item {
public:
    Block(std::string name, ItemType type, short int maxStackSize, blockType blockType) : mBlockType(blockType), Item(name, type, maxStackSize) {}
protected:
    blockType mBlockType;
};