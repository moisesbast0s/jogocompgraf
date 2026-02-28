// #include <GL/glut.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "core/game_enums.h"
#include "core/game_state.h"

#include "core/game.h"

#include "level/level.h"

#include "core/camera.h"
#include "input/input.h"
#include "input/keystate.h"

#include "graphics/drawlevel.h"
#include "graphics/skybox.h"
#include "graphics/hud.h"
#include "graphics/menu.h"
#include "graphics/lighting.h"

#include "core/movement.h"
#include "core/player.h"
#include "core/entities.h"

#include "audio/audio_system.h"

#include "graphics/ui_text.h"
#include "utils/assets.h"
#include "core/config.h"

#include "core/window.h"

#include <GL/glew.h>
#include <GL/glut.h>

static HudTextures gHudTex;
static GameContext g;

constexpr int MAX_MAGAZINE = 7;

// --- Assets / Level ---
static GameAssets gAssets;
Level gLevel;
static AudioSystem gAudioSys;

// --- Sistema de mapas/portal ---
static const char* gMapList[] = {
    "maps/map1.txt",
    "maps/map2.txt",
    "maps/map3.txt"
};
static const int gMapCount = 3;
static int gCurrentMap = 0;

// --- Aviso de fase ---
static float gPhaseNotifyTimer = 0.0f;
static int gPhaseNotifyNum = 0;

// --- Tela de conclusão do jogo ---
static bool gGameComplete = false;
static float gGameCompleteTimer = 0.0f;

GameContext &gameContext() { return g; }

AudioSystem &gameAudio() { return gAudioSys; }

Level &gameLevel() { return gLevel; }

GameState gameGetState() { return g.state; }

void gameSetState(GameState s) { g.state = s; }

void gameTogglePause()
{
    if (g.state == GameState::JOGANDO)
        g.state = GameState::PAUSADO;
    else if (g.state == GameState::PAUSADO)
        g.state = GameState::JOGANDO;
}

// --- INIT ---
bool gameInit(const char *mapPath)
{
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glClearColor(0.05f, 0.05f, 0.1f, 1.0f);

    setupSunLightOnce();
    setupIndoorLightOnce();

    if (!loadAssets(gAssets))
        return false;

    g.r.texChao = gAssets.texChao;
    g.r.texParede = gAssets.texParede;
    g.r.texSangue = gAssets.texSangue;
    g.r.texLava = gAssets.texLava;
    g.r.texChaoInterno = gAssets.texChaoInterno;
    g.r.texParedeInterna = gAssets.texParedeInterna;
    g.r.texTeto = gAssets.texTeto;

    g.r.texSkydome = gAssets.texSkydome;
    g.r.texMenuBG = gAssets.texMenuBG;

    gHudTex.texHudFundo = gAssets.texHudFundo;
    gHudTex.texGunHUD = gAssets.texGunHUD;
    gHudTex.texLanternHUD = gAssets.texLanternHUD;
    gHudTex.texLanternIcon = gAssets.texLanternIcon;

    gHudTex.texGunDefault = gAssets.texGunDefault;
    gHudTex.texGunFire1 = gAssets.texGunFire1;
    gHudTex.texGunFire2 = gAssets.texGunFire2;
    gHudTex.texGunReload1 = gAssets.texGunReload1;
    gHudTex.texGunReload2 = gAssets.texGunReload2;

    gHudTex.texDamage = gAssets.texDamage;
    gHudTex.texHealthOverlay = gAssets.texHealthOverlay;

    for (int i = 0; i < 5; i++)
    {
        g.r.texEnemies[i] = gAssets.texEnemies[i];
        g.r.texEnemiesRage[i] = gAssets.texEnemiesRage[i];
        g.r.texEnemiesDamage[i] = gAssets.texEnemiesDamage[i];
    }

    g.r.texEnemiesDead = gAssets.texEnemiesDead;
    g.r.texAguaEsgoto = gAssets.texAguaEsgoto;

    g.r.texHealth = gAssets.texHealth;
    g.r.texAmmo = gAssets.texAmmo;
    g.r.texBattery = gAssets.texBattery;
    g.r.texPistolAmmo = gAssets.texPistolAmmo;

    g.r.progSangue = gAssets.progSangue;
    g.r.progLava = gAssets.progLava;

    if (!loadLevel(gLevel, mapPath, GameConfig::TILE_SIZE))
        return false;

    applySpawn(gLevel, camX, camZ);
    camY = GameConfig::PLAYER_EYE_Y;

    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutPassiveMotionFunc(mouseMotion);
    glutSetCursor(GLUT_CURSOR_NONE);

    // Audio init + ambient + enemy sources
    audioInit(gAudioSys, gLevel);

    g.state = GameState::MENU_INICIAL;
    g.time = 0.0f;
    g.player = PlayerState{};
    g.weapon = WeaponAnim{};

    return true;
}

// Reinicia o jogo
void gameReset()
{
    g.player.health = 100;
    g.player.currentAmmo = 7;
    g.player.reserveAmmo = 21;

    g.player.spareMagazines = 3;

    g.player.damageAlpha = 0.0f;
    g.player.healthAlpha = 0.0f;

    g.weapon.state = WeaponState::W_IDLE;
    g.weapon.timer = 0.0f;
    // Respawna o jogador
    applySpawn(gLevel, camX, camZ);
}

// Carrega novo mapa (portal)
bool gameLoadMap(const char *mapPath)
{
    if (!loadLevel(gLevel, mapPath, GameConfig::TILE_SIZE))
        return false;

    applySpawn(gLevel, camX, camZ);
    camY = GameConfig::PLAYER_EYE_Y;

    // Re-init audio para novo mapa
    audioInit(gAudioSys, gLevel);

    return true;
}

void gameUpdate(float dt)
{
    g.time += dt;

    // 1. SE NÃO ESTIVER JOGANDO, NÃO RODA A LÓGICA DO JOGO
    if (g.state != GameState::JOGANDO)
    {
        return;
    }

    atualizaMovimento();

    AudioListener L;
    L.pos = {camX, camY, camZ};
    {
        float ry = yaw * 3.14159f / 180.0f;
        float rp = pitch * 3.14159f / 180.0f;
        L.forward = {cosf(rp) * sinf(ry), sinf(rp), -cosf(rp) * cosf(ry)};
    }
    L.up = {0.0f, 1.0f, 0.0f};
    L.vel = {0.0f, 0.0f, 0.0f};

    bool moving = (keyW || keyA || keyS || keyD);
    audioUpdate(gAudioSys, gLevel, L, dt, moving, g.player.health);

    if (g.player.damageAlpha > 0.0f)
    {
        g.player.damageAlpha -= dt * 0.5f;
        if (g.player.damageAlpha < 0.0f)
            g.player.damageAlpha = 0.0f;
    }
    if (g.player.healthAlpha > 0.0f)
    {
        g.player.healthAlpha -= dt * 0.9f;
        if (g.player.healthAlpha < 0.0f)
            g.player.healthAlpha = 0.0f;
    }

    updateEntities(dt);
    updateWeaponAnim(dt);

    // CHECAGEM DO PORTAL
    char tile = getTileAtPlayer(camX, camZ);
    if (tile == 'P' && !gGameComplete)
    {
        gCurrentMap++;
        if (gCurrentMap >= gMapCount)
        {
            // Último mapa - jogador zerou o jogo!
            gGameComplete = true;
            gGameCompleteTimer = 5.0f;
        }
        else
        {
            gameLoadMap(gMapList[gCurrentMap]);
            gPhaseNotifyNum = gCurrentMap + 1;
            gPhaseNotifyTimer = 3.0f;
        }
    }

    // Atualiza timer do aviso de fase
    if (gPhaseNotifyTimer > 0.0f)
        gPhaseNotifyTimer -= dt;

    // Atualiza timer de conclusão do jogo
    if (gGameComplete)
    {
        gGameCompleteTimer -= dt;
        if (gGameCompleteTimer <= 0.0f)
        {
            gGameComplete = false;
            gGameCompleteTimer = 0.0f;
            gCurrentMap = 0;
            gameLoadMap(gMapList[gCurrentMap]);
            g.state = GameState::MENU_INICIAL;
        }
        return; // Não processa mais nada enquanto mostra tela de conclusão
    }

    // 3. CHECAGEM DE GAME OVER
    if (g.player.health <= 0)
    {
        g.state = GameState::GAME_OVER;
        g.player.damageAlpha = 1.0f;
    }
}

// Função auxiliar para desenhar o mundo 3D (Inimigos, Mapa, Céu)
void drawWorld3D()
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // LIGAR O 3D
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);

    // Configuração da Câmera
    float radYaw = yaw * 3.14159265f / 180.0f;
    float radPitch = pitch * 3.14159265f / 180.0f;
    float dirX = cosf(radPitch) * sinf(radYaw);
    float dirY = sinf(radPitch);
    float dirZ = -cosf(radPitch) * cosf(radYaw);
    gluLookAt(camX, camY, camZ, camX + dirX, camY + dirY, camZ + dirZ, 0.0f, 1.0f, 0.0f);

    // Desenha o cenário
    setSunDirectionEachFrame();
    drawSkydome(camX, camY, camZ, g.r);
    drawLevel(gLevel.map, camX, camZ, dirX, dirZ, g.r, g.time);
    drawEntities(gLevel.enemies, gLevel.items, camX, camZ, dirX, dirZ, g.r);
}

// FUNÇÃO PRINCIPAL DE DESENHO (REFATORADA: usa menuRender / pauseMenuRender / hudRenderAll)
void gameRender()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Monta o estado do HUD a partir das variáveis globais do jogo
    HudState hs;
    hs.playerHealth = g.player.health;
    hs.currentAmmo = g.player.currentAmmo;
    hs.reserveAmmo = g.player.reserveAmmo;
    hs.spareMagazines = g.player.spareMagazines;
    hs.damageAlpha = g.player.damageAlpha;
    hs.healthAlpha = g.player.healthAlpha;
    hs.weaponState = g.weapon.state;

    // --- ESTADO: MENU INICIAL ---
    if (g.state == GameState::MENU_INICIAL)
    {
        // menuRender já cuida do fogo (update + render)
        menuRender(janelaW, janelaH, g.time, "", "Pressione ENTER para Jogar", g.r);
    }
    // --- ESTADO: GAME OVER ---
    else if (g.state == GameState::GAME_OVER)
    {
        // Fundo 3D congelado
        drawWorld3D();

        // OVERLAY DO MELT por cima do jogo
        // menuMeltRenderOverlay(janelaW, janelaH, g.time);

        // Tela do game over por cima (com fogo)
        menuRender(janelaW, janelaH, g.time, "GAME OVER", "Pressione ENTER para Reiniciar", g.r);
    }
    // --- ESTADO: PAUSADO ---
    else if (g.state == GameState::PAUSADO)
    {
        // 1) Mundo 3D congelado
        drawWorld3D();

        // 2) HUD normal (arma + barra + mira + overlays)
        hudRenderAll(janelaW, janelaH, gHudTex, hs, true, true, true);

        // 3) Menu escuro por cima
        pauseMenuRender(janelaW, janelaH, g.time);
    }
    // --- ESTADO: JOGANDO ---
    else // JOGANDO
    {
        // 1) Mundo 3D
        drawWorld3D();

        // 2) HUD completo
        hudRenderAll(janelaW, janelaH, gHudTex, hs, true, true, true);

        menuMeltRenderOverlay(janelaW, janelaH, g.time);

        // 3) Tela de conclusão do jogo
        if (gGameComplete)
        {
            glMatrixMode(GL_PROJECTION);
            glPushMatrix();
            glLoadIdentity();
            gluOrtho2D(0, janelaW, 0, janelaH);
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();

            glDisable(GL_DEPTH_TEST);
            glDisable(GL_LIGHTING);
            glDisable(GL_TEXTURE_2D);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            // Fundo escuro cobrindo a tela toda
            float fadeAlpha = (gGameCompleteTimer > 4.0f) ? (5.0f - gGameCompleteTimer) : 1.0f;
            if (gGameCompleteTimer < 1.0f) fadeAlpha = gGameCompleteTimer;
            glColor4f(0.0f, 0.0f, 0.0f, 0.75f * fadeAlpha);
            glBegin(GL_QUADS);
            glVertex2f(0, 0);
            glVertex2f(janelaW, 0);
            glVertex2f(janelaW, janelaH);
            glVertex2f(0, janelaH);
            glEnd();

            // Texto "VOCE ZEROU O JOGO!"
            const char* msg1 = "VOCE ZEROU O JOGO!";
            float scale1 = 0.4f;
            float tw1 = uiStrokeTextWidthScaled(msg1, scale1);
            float cy1 = janelaH * 0.6f;
            glColor4f(0.0f, 1.0f, 0.3f, fadeAlpha);
            uiDrawStrokeText((janelaW - tw1) * 0.5f, cy1, msg1, scale1);

            // Texto "Parabens!" menor embaixo
            const char* msg2 = "Parabens!";
            float scale2 = 0.25f;
            float tw2 = uiStrokeTextWidthScaled(msg2, scale2);
            glColor4f(0.8f, 0.8f, 0.8f, fadeAlpha * 0.8f);
            uiDrawStrokeText((janelaW - tw2) * 0.5f, cy1 - 60, msg2, scale2);

            glDisable(GL_BLEND);
            glEnable(GL_TEXTURE_2D);
            glEnable(GL_LIGHTING);
            glEnable(GL_DEPTH_TEST);

            glMatrixMode(GL_PROJECTION);
            glPopMatrix();
            glMatrixMode(GL_MODELVIEW);
            glPopMatrix();
        }

        // 4) Aviso de troca de fase
        if (gPhaseNotifyTimer > 0.0f)
        {
            glMatrixMode(GL_PROJECTION);
            glPushMatrix();
            glLoadIdentity();
            gluOrtho2D(0, janelaW, 0, janelaH);
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();

            glDisable(GL_DEPTH_TEST);
            glDisable(GL_LIGHTING);
            glDisable(GL_TEXTURE_2D);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            // Fundo escuro atrás do texto
            float alpha = (gPhaseNotifyTimer > 1.0f) ? 0.6f : gPhaseNotifyTimer * 0.6f;
            glColor4f(0.0f, 0.0f, 0.0f, alpha);
            float cy = janelaH * 0.6f;
            glBegin(GL_QUADS);
            glVertex2f(0, cy - 50);
            glVertex2f(janelaW, cy - 50);
            glVertex2f(janelaW, cy + 50);
            glVertex2f(0, cy + 50);
            glEnd();

            // Texto "FASE X"
            char phaseMsg[32];
            std::sprintf(phaseMsg, "FASE %d", gPhaseNotifyNum);
            float textScale = 0.35f;
            float tw = uiStrokeTextWidthScaled(phaseMsg, textScale);
            float textAlpha = (gPhaseNotifyTimer > 1.0f) ? 1.0f : gPhaseNotifyTimer;
            glColor4f(0.0f, 0.8f, 1.0f, textAlpha);
            uiDrawStrokeText((janelaW - tw) * 0.5f, cy - 20, phaseMsg, textScale);

            glDisable(GL_BLEND);
            glEnable(GL_TEXTURE_2D);
            glEnable(GL_LIGHTING);
            glEnable(GL_DEPTH_TEST);

            glMatrixMode(GL_PROJECTION);
            glPopMatrix();
            glMatrixMode(GL_MODELVIEW);
            glPopMatrix();
        }
    }

    glutSwapBuffers();
}
