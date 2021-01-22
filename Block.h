#pragma once

#include "Item.h"

class Block : public Item {
public:
    Block(std::string name, ItemType type, blockType blockType) : mBlockType(blockType), Item(name, type, 64, 1) {}
protected:
    blockType mBlockType;
};