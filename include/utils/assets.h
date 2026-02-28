#pragma once
#include <GL/glew.h>

struct GameAssets
{
    // texturas
    GLuint texMenuBG = 0;
    GLuint texChao = 0;
    GLuint texParede = 0;
    GLuint texSangue = 0;
    GLuint texLava = 0;
    GLuint texAguaEsgoto = 0;   // sewage water tile texture
    GLuint texChaoInterno = 0;
    GLuint texParedeInterna = 0;
    GLuint texTeto = 0;
    GLuint texEnemy = 0;
    GLuint texEnemyRage = 0;    // Viu o player
    GLuint texEnemyDamage = 0;  // Leva dano
    GLuint texHealthOverlay = 0; // Tela de cura
    GLuint texHealth = 0;
    GLuint texAmmo = 0;
    GLuint texBattery = 0;
    GLuint texPistolAmmo = 0;
    GLuint texGunDefault = 0;
    GLuint texGunFire1 = 0;
    GLuint texGunFire2 = 0;
    GLuint texGunReload1 = 0;
    GLuint texGunReload2 = 0;
    GLuint texDamage = 0;
    GLuint texGunHUD = 0;
    GLuint texLanternHUD = 0;           // new icon for lantern (not a real weapon)
    GLuint texLanternIcon = 0;          // small lantern icon for doom bar
    GLuint texHudFundo = 0;

    GLuint texEnemies[5]       = {0, 0, 0, 0, 0};
    GLuint texEnemiesRage[5]   = {0, 0, 0, 0, 0};
    GLuint texEnemiesDamage[5] = {0, 0, 0, 0, 0};
    GLuint texEnemiesDead = 0;               // corpse sprite

    // shaders
    GLuint progSangue = 0;
    GLuint progLava = 0;

    GLuint texSkydome = 0;
};

bool loadAssets(GameAssets &a);
