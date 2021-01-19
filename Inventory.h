#pragma once

#include "Item.h"

class Inventory {
public:
    void te() {
        items = Item[10];
    }
private:
    Item *items;
    short int size;
};