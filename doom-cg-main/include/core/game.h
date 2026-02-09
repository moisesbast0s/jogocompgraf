#pragma once

#include "level/level.h"

extern Level gLevel;

bool gameInit(const char* mapPath);
void gameUpdate(float dt);
void gameRender();
void playerTryAttack();
void playerTryReload();
