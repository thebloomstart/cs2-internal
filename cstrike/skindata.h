#ifndef SKIN_MANAGER_H
#define SKIN_MANAGER_H

#include <cstdint>
#include <string>
#include <vector>

struct SkinData {
    int weaponID;
    int quality;
    int rarity;
    int skinID;
    int paintSeed;
    float wear;
};

// Функция для добавления скинов
void addSkin();

#endif // SKIN_MANAGER_H