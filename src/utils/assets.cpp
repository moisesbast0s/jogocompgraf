#include "utils/assets.h"
#include "graphics/texture.h"
#include "graphics/shader.h"
#include <cstdio>

bool loadAssets(GameAssets &a)
{
    a.texMenuBG = carregaTextura("assets/menu_bg.png");

    a.texChao = carregaTextura("assets/181.png");
    a.texParede = carregaTextura("assets/textura-cranio.jpeg");
    a.texSangue = carregaTextura("assets/016.png");
    a.texLava = carregaTextura("assets/179.png");
    a.texAguaEsgoto = carregaTextura("assets/agua_esgoto.png");
    a.texChaoInterno = carregaTextura("assets/100.png");
    a.texParedeInterna = carregaTextura("assets/060.png");
    a.texTeto = carregaTextura("assets/081.png");

    a.progSangue = criaShader("shaders/blood.vert", "shaders/blood.frag");
    a.progLava = criaShader("shaders/lava.vert", "shaders/lava.frag");
    a.progPortal = criaShader("shaders/portal.vert", "shaders/portal.frag");

    // --- INIMIGO 0 ('J') ---
    a.texEnemies[0] = carregaTextura("assets/enemies/enemy.png");
    a.texEnemiesRage[0] = carregaTextura("assets/enemies/enemyRage.png");
    a.texEnemiesDamage[0] = carregaTextura("assets/enemies/enemyRageDamage.png");

    // --- INIMIGO 1 ('T') - BAT ---
    a.texEnemies[1] = carregaTextura("assets/enemies/enemyBat.png");
    a.texEnemiesRage[1] = carregaTextura("assets/enemies/enemyBatRage.png");
    a.texEnemiesDamage[1] = carregaTextura("assets/enemies/enemyBatRageDamage.png");
    a.texEnemiesDeadBat = carregaTextura("assets/enemies/enemyBatDead.png");


    // corpse texture for dead enemies
    a.texEnemiesDead = carregaTextura("assets/enemies/dead.png");
    a.texEnemiesDeadBat = carregaTextura("assets/enemies/enemyBatDead.png");

    // --- BOSS 0 ---
    a.texBosses[0] = carregaTextura("assets/enemies/boss.png");
    a.texBossesRage[0] = carregaTextura("assets/enemies/bossRage.png");
    a.texBossesDamage[0] = carregaTextura("assets/enemies/bossRageDamage.png");
    // --- BOSS 1 ---
    a.texBosses[1] = carregaTextura("assets/enemies/bossBat.png");
    a.texBossesRage[1] = carregaTextura("assets/enemies/bossBatRage.png");
    a.texBossesDamage[1] = carregaTextura("assets/enemies/bossBatRageDamage.png");
    // --- BOSS 2 ---
    a.texBosses[2] = carregaTextura("assets/enemies/boss.png");
    a.texBossesRage[2] = carregaTextura("assets/enemies/bossRage.png");
    a.texBossesDamage[2] = carregaTextura("assets/enemies/bossRageDamage.png");

    // corpse texture for dead bosses
    a.texBossesDead = carregaTextura("assets/enemies/bossDead.png");
    a.texBossesDeadBat = carregaTextura("assets/enemies/bossBatDead.png");

    a.texHealthOverlay = carregaTextura("assets/heal.png");
    a.texGunDefault = carregaTextura("assets/gun_default.png");
    a.texGunFire1 = carregaTextura("assets/gun_fire1.png");
    a.texGunFire2 = carregaTextura("assets/gun_fire2.png");
    a.texGunReload1 = carregaTextura("assets/gun_reload1.png");
    a.texGunReload2 = carregaTextura("assets/gun_reload2.png");
    a.texDamage = carregaTextura("assets/damage.png");

    a.texHealth = carregaTextura("assets/health.png");
    a.texAmmo = carregaTextura("assets/066.png");
    a.texBattery = carregaTextura("assets/BatteryCharge.png");
    a.texPistolAmmo = carregaTextura("assets/pistolAmmo.png");

    a.texSkydome = carregaTextura("assets/Va4wUMQ.png");

    a.texGunHUD = carregaTextura("assets/arma.png");
    a.texLanternHUD = carregaTextura("assets/lantern.png");
    a.texLanternIcon = carregaTextura("assets/icon_latern.png");
    a.texHudFundo = carregaTextura("assets/100.png");

    a.texFog   = carregaTextura("assets/fog2.png");
    a.texSmoke = carregaTextura("assets/smoke_03.png");

    if (!a.texChao || !a.texParede || !a.texSangue || !a.texLava || !a.progSangue ||
        !a.progLava || !a.texHealth || !a.texGunDefault || !a.texGunFire1 ||
        !a.texGunFire2 || !a.texSkydome || !a.texGunReload1 || !a.texGunReload2 ||
        !a.texDamage || !a.texAmmo || !a.texHealthOverlay || !a.texEnemies[0] ||
        !a.texEnemiesRage[0] || !a.texEnemiesDamage[0] || !a.texEnemies[1] ||
        !a.texEnemiesRage[1] || !a.texEnemiesDamage[1] || !a.texEnemies[2] || !a.texFog || !a.texSmoke ||
        !a.texEnemiesRage[2] || !a.texEnemiesDamage[2] || !a.texEnemiesDead || !a.texAguaEsgoto || !a.texGunHUD || !a.texLanternHUD || !a.texHudFundo || !a.texMenuBG ||
        !a.texBattery || !a.texPistolAmmo || !a.texBosses[0] || !a.texBossesRage[0] || !a.texBossesDamage[0] ||
        !a.texBosses[1] || !a.texBossesRage[1] || !a.texBossesDamage[1] ||
        !a.texBosses[2] || !a.texBossesRage[2] || !a.texBossesDamage[2] || !a.texBossesDead)
    {
        std::printf("ERRO: falha ao carregar algum asset (textura/shader).\n");
        return false;
    }
    return true;
}
