#pragma once

#include "Tool.h"

class WoodPickaxe : public Tool {
public:
    WoodPickaxe() : Tool("Wooden Pickaxe", ItemType::ITEM, 1, WOOD) {}      
};