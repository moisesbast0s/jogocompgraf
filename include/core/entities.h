#pragma once

bool isWalkable(float x, float z);
char getTileAtPlayer(float x, float z);
void updateEntities(float dt);

// Configurações da IA
const float ENEMY_SPEED = 2.5f;
const float ENEMY_VIEW_DIST = 15.0f;
const float ENEMY_ATTACK_DIST = 1.5f;

// Inimigo inicial HP (regulares)
const float ENEMY_START_HP = 100.0f;

// Boss configurations
const float BOSS_SPEED = 3.0f;           
const float BOSS_VIEW_DIST = 20.0f;      
const float BOSS_ATTACK_DIST = 2.0f;     
const float BOSS_START_HP = 2000.0f;      
const float BOSS_ATTACK_DMG = 40;        

enum EnemyState
{
    STATE_IDLE,
    STATE_CHASE,
    STATE_ATTACK,
    STATE_DEAD
};

struct Enemy
{
    int type;
    float x, z;       // Posição no mundo
    float hp;         // Vida
    EnemyState state; // Estado atual (IA)
    float startX, startZ;

    float respawnTimer;
    // Animação
    int animFrame;
    float animTimer;

    // NOVO: Tempo de espera entre um ataque e outro
    float attackCooldown;

    // NOVO: Tempo que ele fica com a textura de dano
    float hurtTimer;
};

enum ItemType
{
    ITEM_HEALTH,
    ITEM_AMMO,
    ITEM_AMMO_BOX,
    ITEM_BATTERY,
    ITEM_PISTOL_AMMO
};

struct Item
{
    float x, z;
    ItemType type;
    bool active; // Se false, já foi pego

    float respawnTimer;
};