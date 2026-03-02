#include "core/entities.h"
#include "core/game.h"
#include "core/camera.h"
#include "audio/audio_system.h"
#include "graphics/FlashlightState.h"  // for flashlightBattery/flashlightOn
#include <cmath>

bool isWalkable(float x, float z)
{
    auto& lvl = gameLevel();
    float tile = lvl.metrics.tile;
    float offX = lvl.metrics.offsetX;
    float offZ = lvl.metrics.offsetZ;

    int tx = (int)((x - offX) / tile);
    int tz = (int)((z - offZ) / tile);

    const auto& data = lvl.map.data();

    if (tz < 0 || tz >= (int)data.size()) return false;
    if (tx < 0 || tx >= (int)data[tz].size()) return false;

    char c = data[tz][tx];
    if (c == '1' || c == '2') return false;

    return true;
}

// Retorna o tile em que o jogador está
char getTileAtPlayer(float x, float z)
{
    auto& lvl = gameLevel();
    float tile = lvl.metrics.tile;
    float offX = lvl.metrics.offsetX;
    float offZ = lvl.metrics.offsetZ;

    int tx = (int)((x - offX) / tile);
    int tz = (int)((z - offZ) / tile);

    const auto& data = lvl.map.data();

    if (tz < 0 || tz >= (int)data.size()) return '1';
    if (tx < 0 || tx >= (int)data[tz].size()) return '1';

    return data[tz][tx];
}

void updateEntities(float dt)
{
    auto& g = gameContext();
    auto& lvl = gameLevel();
    auto& audio = gameAudio();

    for (auto& en : lvl.enemies)
    {
        if (en.state == STATE_DEAD)
        {
            continue;
        }

        if (en.hurtTimer > 0.0f) en.hurtTimer -= dt;

        float dx = camX - en.x;
        float dz = camZ - en.z;
        float dist = std::sqrt(dx * dx + dz * dz);

        switch (en.state)
        {
        case STATE_IDLE:
        {
            if (en.type >= 5) {
                en.state = STATE_CHASE;
            } else {
                float viewDist = ENEMY_VIEW_DIST;
                if (dist < viewDist) en.state = STATE_CHASE;
            }
            break;
        }

        case STATE_CHASE:
        {
            float atkDist = (en.type >= 5) ? BOSS_ATTACK_DIST : ENEMY_ATTACK_DIST;
            float speed = (en.type >= 5) ? BOSS_SPEED : ENEMY_SPEED;

            if (dist < atkDist)
            {
                en.state = STATE_ATTACK;
                en.attackCooldown = 0.5f;
            }
            else if (en.type < 5)
            {
                float viewDist = ENEMY_VIEW_DIST;
                if (dist > viewDist * 1.5f)
                {
                    en.state = STATE_IDLE;
                }
            }
            if (dist >= atkDist) {
                float dirX = dx / dist;
                float dirZ = dz / dist;

                float moveStep = speed * dt;

                float nextX = en.x + dirX * moveStep;
                if (isWalkable(nextX, en.z)) en.x = nextX;

                float nextZ = en.z + dirZ * moveStep;
                if (isWalkable(en.x, nextZ)) en.z = nextZ;
            }
            break;
        }

        case STATE_ATTACK:
        {
            float atkDist = (en.type >= 5) ? BOSS_ATTACK_DIST : ENEMY_ATTACK_DIST;

            if (dist > atkDist)
            {
                en.state = STATE_CHASE;
            }
            else
            {
                en.attackCooldown -= dt;
                if (en.attackCooldown <= 0.0f)
                {
                    int dmg = (en.type >= 5) ? BOSS_ATTACK_DMG : 10;
                    g.player.health -= dmg;
                    en.attackCooldown = 1.0f;
                    g.player.damageAlpha = 1.0f;
                    audioPlayHurt(audio);
                }
            }
            break;
        }

        default:
            break;
        }
    }

    for (auto& item : lvl.items)
    {
        if (!item.active)
        {
            item.respawnTimer -= dt;
            if (item.respawnTimer <= 0.0f) item.active = true;
            continue;
        }

        float dx = camX - item.x;
        float dz = camZ - item.z;

        if (dx * dx + dz * dz < 1.0f)
        {
            item.active = false;

            if (item.type == ITEM_HEALTH)
            {
                item.respawnTimer = 15.0f;
                g.player.health += 50;
                if (g.player.health > 100) g.player.health = 100;
                g.player.healthAlpha = 1.0f;
            }
            else if (item.type == ITEM_AMMO)
            {
                item.respawnTimer = 999999.0f;
                g.player.reserveAmmo += 7;
                g.player.spareMagazines += 1;
                audioPlayAmmoPickup(audio);
            }
            else if (item.type == ITEM_PISTOL_AMMO)
            {
                item.respawnTimer = 999999.0f;
                g.player.reserveAmmo += 7;
                g.player.spareMagazines += 1;
                audioPlayAmmoPickup(audio);
            }
            else if (item.type == ITEM_BATTERY)
            {
                item.respawnTimer = 999999.0f;
                flashlightBattery += 50.0f;
                if (flashlightBattery > 100.0f) flashlightBattery = 100.0f;
                flashlightOn = true;
                audioPlayBatteryPickup(audio);
            }
        }
    }
}
