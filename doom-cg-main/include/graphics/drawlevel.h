#ifndef DRAWLEVEL_H
#define DRAWLEVEL_H

#include "level/maploader.h"
#include "core/entities.h"
#include <vector>

// Note que adicionamos camX, camZ (posição) e dirX, dirZ (para onde você olha)
void drawLevel(const MapLoader& map, float camX, float camZ, float dirX, float dirZ);

void drawEntities(const std::vector<Enemy>& enemies, const std::vector<Item>& items, 
                  float camX, float camZ, float dirX, float dirZ);

#endif