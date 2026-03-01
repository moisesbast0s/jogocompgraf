// core/game_state.h
#pragma once
#include "core/game_enums.h"
#include <GL/glew.h>

struct PlayerState
{
    int health = 100;
    float damageAlpha = 0.0f;
    float healthAlpha = 0.0f;

    int currentAmmo = 7;
    int reserveAmmo = 21;
    int spareMagazines = 3; // Numero de recargas completas disponíveis
};

struct WeaponAnim
{
    WeaponState state = WeaponState::W_IDLE;
    float timer = 0.0f;
};

struct RenderAssets
{
    GLuint texChao = 0, texParede = 0, texSangue = 0, texLava = 0;
    GLuint texChaoInterno = 0, texParedeInterna = 0, texTeto = 0, texSkydome = 0, texMenuBG = 0;
    // GLuint texSkydome=0;

    GLuint texEnemies[5] = {0};
    GLuint texEnemiesRage[5] = {0};
    GLuint texEnemiesDamage[5] = {0};
    GLuint texEnemiesDead = 0; // corpse sprite copied from assets
    GLuint texAguaEsgoto = 0; // sewage water texture

    GLuint texHealth = 0;
    GLuint texAmmo = 0;
    GLuint texBattery = 0;
    GLuint texPistolAmmo = 0;

    GLuint progSangue = 0;
    GLuint progLava = 0;

    GLint texFog = 0;
    GLint texSmoke = 0;
};

struct GameContext
{
    GameState state = GameState::MENU_INICIAL;
    PlayerState player;
    WeaponAnim weapon;
    float time = 0.0f;

    RenderAssets r;
};
