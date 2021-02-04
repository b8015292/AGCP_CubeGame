#pragma once

#include "Tool.h"

class Pickaxe : public Tool {
public:
    Pickaxe(Type type) : Tool(type, ItemType::TOOL, 1) {}
};