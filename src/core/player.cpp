#include "core/player.h"
#include "core/game.h"
#include "core/camera.h"
#include "audio/audio_system.h"
#include <cmath>

constexpr int MAX_MAGAZINE = 12;

void playerTryReload()
{
    auto &g = gameContext();
    auto &audio = gameAudio();

    if (g.weapon.state != WeaponState::W_IDLE)
        return;
    if (g.player.currentAmmo >= MAX_MAGAZINE)
        return;
    if (g.player.reserveAmmo <= 0)
        return;

    g.weapon.state = WeaponState::W_RELOAD_1;
    g.weapon.timer = 0.50f;

    audioPlayReload(audio);
}

void playerTryAttack()
{
    auto &g = gameContext();
    auto &lvl = gameLevel();
    auto &audio = gameAudio();

    if (g.weapon.state != WeaponState::W_IDLE)
        return;
    if (g.player.currentAmmo <= 0)
        return;

    g.player.currentAmmo--;

    audioOnPlayerShot(audio);
    audioPlayShot(audio);

    g.weapon.state = WeaponState::W_FIRE_1;
    g.weapon.timer = 0.08f;

    for (auto &en : lvl.enemies)
    {
        if (en.state == STATE_DEAD)
            continue;

        float dx = en.x - camX;
        float dz = en.z - camZ;
        float dist = std::sqrt(dx * dx + dz * dz);
        if (dist < 17.0f)
        {
            float radYaw = yaw * 3.14159f / 180.0f;
            float camDirX = std::sin(radYaw);
            float camDirZ = -std::cos(radYaw);

            float toEnemyX = dx / dist;
            float toEnemyZ = dz / dist;

            float dot = camDirX * toEnemyX + camDirZ * toEnemyZ;

            if (dot > 0.6f)
            {
                en.hp -= 30;
                en.hurtTimer = 0.5f;

                if (en.hp <= 0)
                {
                    en.state = STATE_DEAD;
                    en.respawnTimer = 15.0f;

                    Item drop;
                    drop.type = ITEM_AMMO;
                    drop.x = en.x;
                    drop.z = en.z;
                    drop.active = true;
                    drop.respawnTimer = 0.0f;

                    lvl.items.push_back(drop);
                }
                return;
            }
        }
    }
}

void updateWeaponAnim(float dt)
{
    auto &g = gameContext();
    auto &audio = gameAudio();

    const float TIME_FRAME_2 = 0.12f;
    const float RELOAD_T2 = 0.85f;
    const float RELOAD_T3 = 0.25f;

    if (g.weapon.state == WeaponState::W_IDLE)
        return;

    g.weapon.timer -= dt;
    if (g.weapon.timer > 0.0f)
        return;

    if (g.weapon.state == WeaponState::W_FIRE_1)
    {
        g.weapon.state = WeaponState::W_FIRE_2;
        g.weapon.timer = TIME_FRAME_2;
    }
    else if (g.weapon.state == WeaponState::W_FIRE_2)
    {
        g.weapon.state = WeaponState::W_PUMP;
        g.weapon.timer = AudioTuning::PUMP_TIME;
        audioPlayPumpClick(audio);
    }
    else if (g.weapon.state == WeaponState::W_RETURN)
    {
        g.weapon.state = WeaponState::W_IDLE;
        g.weapon.timer = 0.0f;
    }
    else if (g.weapon.state == WeaponState::W_PUMP)
    {
        g.weapon.state = WeaponState::W_IDLE;
        g.weapon.timer = 0.0f;
    }
    else if (g.weapon.state == WeaponState::W_RELOAD_1)
    {
        g.weapon.state = WeaponState::W_RELOAD_2;
        g.weapon.timer = RELOAD_T2;
    }
    else if (g.weapon.state == WeaponState::W_RELOAD_2)
    {
        g.weapon.state = WeaponState::W_RELOAD_3;
        g.weapon.timer = RELOAD_T3;
    }
    else if (g.weapon.state == WeaponState::W_RELOAD_3)
    {
        g.weapon.state = WeaponState::W_IDLE;
        g.weapon.timer = 0.0f;

        int needed = MAX_MAGAZINE - g.player.currentAmmo;
        if (needed > g.player.reserveAmmo)
            needed = g.player.reserveAmmo;

        g.player.currentAmmo += needed;
        g.player.reserveAmmo -= needed;
    }
}
