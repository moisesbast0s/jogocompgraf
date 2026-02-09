#include "core/game.h"
#include "utils/assets.h"
#include "level/level.h"
#include "core/config.h"
#include "graphics/skybox.h"

#include "core/camera.h"
#include "input/input.h"
#include "graphics/drawlevel.h"
#include "core/movement.h"
#include "core/entities.h" 

#include <GL/glut.h>
#include "core/window.h"
#include <cmath>
#include <cstdio>
#include <string>

// --- VARIÁVEIS GLOBAIS ---
GLuint texChao;
GLuint texParede;
GLuint texSangue;
GLuint texLava;
GLuint texChaoInterno;
GLuint texParedeInterna;
GLuint texTeto;
GLuint texSkydome;

// Texturas de Entidades
GLuint texEnemies[5];
GLuint texEnemiesRage[5];
GLuint texEnemiesDamage[5];
GLuint texHealth;
GLuint texAmmo;
GLuint texGunDefault, texGunFire1, texGunFire2;
GLuint texGunReload1, texGunReload2;
GLuint texDamage;
float damageAlpha = 0.0f; // Começa invisível
GLuint texHealthOverlay;
float healthAlpha = 0.0f;

GLuint progSangue;
GLuint progLava;

float tempo = 0.0f;
int playerHealth = 100; // Vida do Jogador
// SISTEMA DE MUNIÇÃO
const int MAX_MAGAZINE = 10; // Capacidade do pente
int currentAmmo = 10;        // Balas na arma
int reserveAmmo = 20;        // Balas no bolso

// Estados da animação
enum WeaponState {
    W_IDLE,    // Parado
    W_FIRE_1,  // Inicio do tiro
    W_FIRE_2,  // Fogo grande
    W_RETURN,   // Volta pro inicio do tiro

    W_RELOAD_1, // Começa a baixar a arma / mexer (gun_reload1)
    W_RELOAD_2, // Ação de trocar o pente (gun_reload2)
    W_RELOAD_3  // Volta a arma pra posição (gun_reload1)
};

WeaponState weaponState = W_IDLE;
float weaponTimer = 0.0f; // Conta o tempo de cada frame

static GameAssets gAssets;
Level gLevel;

// Configurações da IA
const float ENEMY_SPEED = 2.5f;     
const float ENEMY_VIEW_DIST = 15.0f; 
const float ENEMY_ATTACK_DIST = 1.5f;

// --- FUNÇÕES AUXILIARES DE LUZ ---
static void setupIndoorLightOnce()
{
    glEnable(GL_LIGHT1);
    GLfloat lampDiffuse[] = {1.7f, 1.7f, 1.8f, 1.0f};
    GLfloat lampSpecular[] = {0, 0, 0, 1.0f};
    GLfloat lampAmbient[] = {0.98f, 0.99f, 1.41f, 1.0f};
    glLightfv(GL_LIGHT1, GL_DIFFUSE, lampDiffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, lampSpecular);
    glLightfv(GL_LIGHT1, GL_AMBIENT, lampAmbient);
    glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 0.6f);
    glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.06f);
    glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.02f);
    glDisable(GL_LIGHT1);
}

static void setupSunLightOnce()
{
    glEnable(GL_LIGHT0);
    GLfloat sceneAmbient[] = {0.45f, 0.30f, 0.25f, 1.0f};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, sceneAmbient);
    GLfloat sunDiffuse[] = {1.4f, 0.55f, 0.30f, 1.0f};
    GLfloat sunSpecular[] = {0.0f, 0.0f, 0.0f, 1.0f}; 
    glLightfv(GL_LIGHT0, GL_DIFFUSE, sunDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, sunSpecular);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
}

static void setSunDirectionEachFrame()
{
    GLfloat sunDir[] = {0.3f, 1.0f, 0.2f, 0.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, sunDir);
}

void playerTryReload()
{
    // Se a arma já estiver ocupada (atirando OU recarregando), ignora
    if (weaponState != W_IDLE) return;
    if (currentAmmo >= MAX_MAGAZINE) return; // Já está cheia
    if (reserveAmmo <= 0) return; // Não tem balas extras

    std::printf("Recarregando...\n");
    
    // Inicia a sequencia
    weaponState = W_RELOAD_1;
    weaponTimer = 0.2f; // Tempo do primeiro frame (ajuste a gosto)
}

// --- FUNÇÃO DE ATAQUE DO JOGADOR ---
void playerTryAttack()
{
    std::printf("POW! Tentando atacar...\n");

    // Só ataca se a arma estiver parada (evita spam de clique bugando a anim)
    if (weaponState != W_IDLE) return;


    if (currentAmmo <= 0) {
        std::printf("CLICK! Sem municao! Aperte R!\n");
        // Aqui você poderia tocar um som de "Click" seco
        return; 
    }

    // 2. Gasta a bala
    currentAmmo--;
    

    // Começa a animação
    weaponState = W_FIRE_1;
    weaponTimer = 0.08f; // Tempo do primeiro frame

    std::printf("POW! Tiro!\n");


    for (auto &en : gLevel.enemies)
    {
        if (en.state == STATE_DEAD) { 
            continue; 
        }
        float dx = en.x - camX;
        float dz = en.z - camZ;
        float dist = std::sqrt(dx*dx + dz*dz);

        // Se estiver perto
        if (dist < 17.0f)
        {
            // Calcula se estamos olhando para o inimigo
            float radYaw = yaw * 3.14159f / 180.0f;
            float camDirX = std::sin(radYaw);
            float camDirZ = -std::cos(radYaw);

            float toEnemyX = dx / dist;
            float toEnemyZ = dz / dist;

            float dot = camDirX * toEnemyX + camDirZ * toEnemyZ;

            // Se o ângulo for bom (olhando pra frente)
            if (dot > 0.6f)
            {
                en.hp -= 30; // Tira 30 de vida
                en.hurtTimer = 0.5f; //Fica com cara de dor por 0.5s
                std::printf("ACERTOU! HP Inimigo: %.0f\n", en.hp);

                if (en.hp <= 0)
                {
                    en.state = STATE_DEAD;
                    en.respawnTimer = 15.0f;
                
                    std::printf("Inimigo MORREU! Respawn em 1min.\n");
                    Item drop;
                    drop.type = ITEM_AMMO; // Tipo Munição
                    drop.x = en.x;
                    drop.z = en.z;
                    drop.active = true;

                    drop.respawnTimer = 0.0f;
                    
                    gLevel.items.push_back(drop); 
                    
                    std::printf("DROP! Inimigo derrubou municao!\n");
                }
                return; // Acerta um por vez
            }
        }
    }
}

// Função auxiliar para ver se o inimigo pode pisar ali
bool isWalkable(float x, float z)
{
    float tile = gLevel.metrics.tile;
    float offX = gLevel.metrics.offsetX;
    float offZ = gLevel.metrics.offsetZ;

    // Converte posição do mundo para índice do vetor do mapa
    int tx = (int)((x - offX) / tile);
    int tz = (int)((z - offZ) / tile);

    const auto& data = gLevel.map.data();
    
    // Verifica se não caiu fora do mapa
    if (tz < 0 || tz >= (int)data.size()) return false;
    if (tx < 0 || tx >= (int)data[tz].size()) return false;

    char c = data[tz][tx];

    // Se for 1 ou 2, é parede. Todo o resto (0, 3, E, H) é chão.
    if (c == '1' || c == '2') return false;

    return true;
}

// --- LÓGICA DAS ENTIDADES (IA E ITENS) ---
// Definida ANTES do gameUpdate para não dar erro
void updateEntities(float dt)
{
    // 1. Inimigos
    for (auto &en : gLevel.enemies)
    {
        if (en.state == STATE_DEAD) 
        {
            en.respawnTimer -= dt;
            if (en.respawnTimer <= 0.0f)
            {
                // HORA DE REVIVER!
                en.state = STATE_IDLE;
                en.hp = 100; 
                en.x = en.startX; 
                en.z = en.startZ;
                en.hurtTimer = 0.0f;
                std::printf("Inimigo RESPAWNOU!\n");
            }
            continue; 
        }

        // Se o código chegou aqui, o inimigo está VIVO. Segue a lógica:

        if (en.hurtTimer > 0.0f) {
            en.hurtTimer -= dt;
        }

        float dx = camX - en.x;
        float dz = camZ - en.z;
        float dist = std::sqrt(dx*dx + dz*dz);

        switch (en.state)
        {
        case STATE_IDLE:
            if (dist < ENEMY_VIEW_DIST) en.state = STATE_CHASE;
            break;

        case STATE_CHASE:
            if (dist < ENEMY_ATTACK_DIST) 
            {
                en.state = STATE_ATTACK;
                en.attackCooldown = 0.5f; 
            }
            else if (dist > ENEMY_VIEW_DIST * 1.5f) 
            {
                en.state = STATE_IDLE;
            }
            else
            {
                float dirX = dx / dist;
                float dirZ = dz / dist;
                
                float moveStep = ENEMY_SPEED * dt;

                // Tenta mover no eixo X
                float nextX = en.x + dirX * moveStep;
                if (isWalkable(nextX, en.z)) en.x = nextX;

                // Tenta mover no eixo Z
                float nextZ = en.z + dirZ * moveStep;
                if (isWalkable(en.x, nextZ)) en.z = nextZ;
            }
            break;

        case STATE_ATTACK:
            if (dist > ENEMY_ATTACK_DIST)
            {
                en.state = STATE_CHASE;
            }
            else
            {
                en.attackCooldown -= dt;
                if (en.attackCooldown <= 0.0f)
                {
                    playerHealth -= 10;
                    en.attackCooldown = 1.0f; 
                    std::printf("DANO RECEBIDO! Vida: %d\n", playerHealth);
                    damageAlpha = 1.0f; 

                    if (playerHealth <= 0) {
                        std::printf("=== GAME OVER ===\n");
                        playerHealth = 100;
                        loadLevel(gLevel, "maps/map1.txt", 4.0f); 
                        applySpawn(gLevel, camX, camZ);
                        damageAlpha = 0.0f;
                    }
                }
            }
            break;
        }
    }

    // 2. Itens
    for (auto &item : gLevel.items)
    {
        if (!item.active) 
        {
            item.respawnTimer -= dt;
            if (item.respawnTimer <= 0.0f)
            {
                item.active = true;
                std::printf("Item de volta no mapa!\n");
            }
            continue; 
        }

        // Se chegou aqui, o item está ATIVO. Verifica colisão:

        float dx = camX - item.x;
        float dz = camZ - item.z;
        
        // Coleta se chegar perto (1 metro)
        if (dx*dx + dz*dz < 1.0f) 
        {
            item.active = false;
            
            if (item.type == ITEM_HEALTH) {
                item.respawnTimer = 15.0f;
                playerHealth += 50;
                if(playerHealth > 100) playerHealth = 100;
                healthAlpha = 1.0f; // Mostra o overlay de cura
                std::printf("PEGOU CURA! Vida: %d\n", playerHealth);
            }
            else if (item.type == ITEM_AMMO) {
                // Caixa de munição não precisa de respawnTimer aqui se não quiser,
                // ou pode colocar também item.respawnTimer = 60.0f;
                item.respawnTimer = 999999.0f; // Nunca respawna
                reserveAmmo = 20;
                std::printf("PEGOU MUNICAO!\n");
            }
        }
    }
}

// --- GAME INIT ---
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

    // Conecta variáveis globais
    texChao = gAssets.texChao;
    texParede = gAssets.texParede;
    texSangue = gAssets.texSangue;
    texLava = gAssets.texLava;
    texChaoInterno = gAssets.texChaoInterno;
    texParedeInterna = gAssets.texParedeInterna;
    texTeto = gAssets.texTeto;
    
    // --- CONECTA O SKYDOME ---
    texSkydome = gAssets.texSkydome;


    texGunDefault  = gAssets.texGunDefault;
    texGunFire1 = gAssets.texGunFire1;
    texGunFire2 = gAssets.texGunFire2;
    texGunReload1 = gAssets.texGunReload1;
    texGunReload2 = gAssets.texGunReload2;

    texDamage = gAssets.texDamage;
    for(int i=0; i<5; i++) {
        texEnemies[i]       = gAssets.texEnemies[i];
        texEnemiesRage[i]   = gAssets.texEnemiesRage[i];
        texEnemiesDamage[i] = gAssets.texEnemiesDamage[i];
    }
    texHealthOverlay = gAssets.texHealthOverlay;
    texHealth = gAssets.texHealth;
    texAmmo = gAssets.texAmmo;

    progSangue = gAssets.progSangue;
    progLava = gAssets.progLava;

    if (!loadLevel(gLevel, mapPath, GameConfig::TILE_SIZE))
        return false;

    applySpawn(gLevel, camX, camZ);
    camY = GameConfig::PLAYER_EYE_Y;

    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutPassiveMotionFunc(mouseMotion);
    
    // O mouse click é registrado no app.cpp, não aqui, mas a lógica está pronta.
    
    glutSetCursor(GLUT_CURSOR_NONE);

    return true;
}

void updateWeaponAnim(float dt)
{
    // Tempos de cada frame (ajuste para ficar mais rápido ou devagar)
    const float TIME_FRAME_1 = 0.05f; // 80ms (Rápido)
    const float TIME_FRAME_2 = 0.12f; // 120ms (Fogo grande fica um pouco mais)
    const float TIME_RELOAD = 0.25f; // Reload é mais lento que o tiro
    
    if (weaponState == W_IDLE) return; // Se tá parado, não faz nada

    weaponTimer -= dt;

    if (weaponTimer <= 0.0f)
    {
        // Troca de estado quando o tempo acaba
        if (weaponState == W_FIRE_1)
        {
            weaponState = W_FIRE_2; // Vai pro fogo grande
            weaponTimer = TIME_FRAME_2;
        }
        else if (weaponState == W_FIRE_2)
        {
            weaponState = W_RETURN; // Volta pro frame 1
            weaponTimer = TIME_FRAME_1;
        }
        else if (weaponState == W_RETURN)
        {
            weaponState = W_IDLE; // Acabou, volta pro idle
            weaponTimer = 0.0f;
        }
        // --- SEQUÊNCIA DE RELOAD (NOVA) ---
        // Sequência: R1 -> R2 -> R3 (que usa imagem 1) -> IDLE
        else if (weaponState == W_RELOAD_1) {
            weaponState = W_RELOAD_2;
            weaponTimer = TIME_RELOAD; // Fica um tempo trocando o pente
        }
        else if (weaponState == W_RELOAD_2) {
            weaponState = W_RELOAD_3;
            weaponTimer = 0.2f; // Movimento de volta
        }
        else if (weaponState == W_RELOAD_3) {
            weaponState = W_IDLE;
            weaponTimer = 0.0f;

            // MATEMÁTICA DA RECARGA
            int needed = MAX_MAGAZINE - currentAmmo; // Quantas faltam pra encher? (Ex: 10 - 5 = 5)
            
            // Só pegamos o que temos disponível na reserva
            if (needed > reserveAmmo) {
                needed = reserveAmmo; 
            }

            currentAmmo += needed; // Enche a arma
            reserveAmmo -= needed; // Tira da reserva
            std::printf("Arma carregada!\n");
        }
    }
}

// --- GAME UPDATE ---
void gameUpdate(float dt)
{
    tempo += dt;

    atualizaMovimento();

    if (damageAlpha > 0.0f) {
        damageAlpha -= dt * 0.5f; // Demora uns 2 segundos pra sumir totalmente
        if (damageAlpha < 0.0f) damageAlpha = 0.0f;
    }

    if (healthAlpha > 0.0f) {
        healthAlpha -= dt * 0.9f; // Ajuste 0.9f se quiser mais rápido/devagar
        if (healthAlpha < 0.0f) healthAlpha = 0.0f;
    }
    
    // Agora o compilador acha a função porque ela foi escrita lá em cima
    updateEntities(dt);

    updateWeaponAnim(dt);
}

// Função auxiliar pra desenhar texto na tela
void drawText(float x, float y, const char *text, float escala) 
{
    glPushMatrix(); // Salva a posição atual
    
    // 1. Move para a posição do texto
    glTranslatef(x, y, 0);

    // 2. Define o tamanho (Escala)
    // O texto Stroke original é gigante (100 pixels+), então usamos 0.x para diminuir
    // 0.2f = Tamanho parecido com 24px
    // 0.5f = Grande
    // 1.0f = Gigante
    glScalef(escala, escala, 1.0f); 

    // 3. Define a grossura da linha (opcional, deixa mais negrito)
    glLineWidth(2.0f);

    for (const char *c = text; *c != '\0'; c++) {
        // Note que mudou de Bitmap para Stroke
        glutStrokeCharacter(GLUT_STROKE_ROMAN, *c); 
    }

    glPopMatrix(); // Restaura a posição
}

void drawAmmoHUD()
{
    // Configura o modo 2D (igual na arma e no sangue)
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, janelaW, 0, janelaH);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    glDisable(GL_TEXTURE_2D); // <--- DESLIGA TEXTURA PARA O TEXTO APARECER

    // Cor do Texto (Amarelo ou Branco)
    glColor3f(1.0f, 0.8f, 0.0f); 

    // Prepara o texto: "10 / 20"
    std::string ammoStr = std::to_string(currentAmmo) + " / " + std::to_string(reserveAmmo);
    
    // Posição: Canto Superior Esquerdo
    // x = 20 pixels da borda esquerda
    // y = altura da janela - 30 pixels (pra não ficar colado no topo)
    drawText(20, janelaH - 50, ammoStr.c_str(), 0.3f);
    // --- LIGA TEXTURA DE VOLTA PARA NÃO ESTRAGAR O RESTO DO JOGO ---
    glEnable(GL_TEXTURE_2D); // <--- RELIGA AQUI

    // Restaura tudo
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}
void drawHealthHUD()
{
    // 1. Configura 2D
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, janelaW, 0, janelaH);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D); // Importante para o texto aparecer

    // 2. Lógica de Cor (Semáforo)
    if (playerHealth > 60) {
        glColor3f(0.2f, 1.0f, 0.2f); // Verde (Saudável)
    } else if (playerHealth > 30) {
        glColor3f(1.0f, 1.0f, 0.0f); // Amarelo (Atenção)
    } else {
        glColor3f(1.0f, 0.0f, 0.0f); // Vermelho (Perigo)
    }

    // 3. Texto
    std::string hpStr = "VIDA: " + std::to_string(playerHealth);

    // 4. Posição (Canto Superior Direito)
    // janelaW = largura total
    // Tiramos uns 250 pixels da direita para o texto caber
    float x = janelaW - 250.0f; 
    float y = janelaH - 50.0f;  // Mesma altura da munição

    // Usando a função drawText escalável que criamos antes (escala 0.3)
    drawText(x, y, hpStr.c_str(), 0.3f);

    // 5. Restaura tudo
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}
void drawWeaponHUD()
{
    // 1. Configura projeção 2D (HUD)
    glMatrixMode(GL_PROJECTION);
    glPushMatrix(); // Salva a projeção 3D
    glLoadIdentity();
    gluOrtho2D(0, janelaW, 0, janelaH); // Coordenadas em pixels da tela

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix(); // Salva a câmera 3D
    glLoadIdentity();

    // 2. Desabilita teste de profundidade e iluminação (pra arma ficar sempre na frente e acesa)
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 3. Escolhe qual textura desenhar baseada no estado
    GLuint currentTex = texGunDefault;
    if (weaponState == W_FIRE_1 || weaponState == W_RETURN) currentTex = texGunFire1;
    else if (weaponState == W_FIRE_2) currentTex = texGunFire2;
    // Lógica do Reload
    else if (weaponState == W_RELOAD_1 || weaponState == W_RELOAD_3) {
        currentTex = texGunReload1; // Começo e Fim usam a imagem 1
    }
    else if (weaponState == W_RELOAD_2) {
        currentTex = texGunReload2; // Meio da ação usa a imagem 2
    }

    glBindTexture(GL_TEXTURE_2D, currentTex);
    glColor3f(1, 1, 1); // Branco puro (sem tintura)

    // 4. Calcula tamanho e posição na tela
    // Vamos fazer a arma ter tamanho proporcional à altura da tela
    float gunH = janelaH * 0.5f; // Arma ocupa 50% da altura
    float gunW = gunH;           // Quadrada (ou ajuste conforme sua imagem)
    
    // Centraliza na base da tela
    float x = (janelaW - gunW) / 2.0f;
    float y = 0.0f; // Colada em baixo

    // Efeito de "coice" (Recoil)
    // Se estiver atirando, a arma desce um pouquinho e treme
    if (weaponState != W_IDLE) {
        y -= 20.0f; 
        x += (rand() % 10 - 5); // Treme 5 pixels pros lados
    }

    // 5. Desenha o quadrado da arma
    glBegin(GL_QUADS);
        // Note que o segundo número (V) foi invertido: de 0.0 virou 1.0 e vice-versa.
        glTexCoord2f(0.0f, 1.0f); glVertex2f(x, y);            // Baixo Esq da tela = Topo da imagem
        glTexCoord2f(1.0f, 1.0f); glVertex2f(x + gunW, y);     // Baixo Dir da tela = Topo da imagem
        glTexCoord2f(1.0f, 0.0f); glVertex2f(x + gunW, y + gunH); // Cima Dir da tela = Base da imagem
        glTexCoord2f(0.0f, 0.0f); glVertex2f(x, y + gunH);     // Cima Esq da tela = Base da imagem
    glEnd();

    // 6. Restaura as configurações anteriores
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

// Desenha o overlay de dano na tela
void drawDamageOverlay()
{
    if (damageAlpha <= 0.0f) return; // Se alpha é 0, não desenha nada

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, janelaW, 0, janelaH);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindTexture(GL_TEXTURE_2D, texDamage);
    
    // Define a cor como Branca, mas com a transparência do damageAlpha
    glColor4f(1.0f, 1.0f, 1.0f, damageAlpha); 

    glBegin(GL_QUADS);
        // Estica a imagem para cobrir a tela inteira (0,0 até W,H)
        // Usando a mesma inversão UV que usamos na arma pra garantir que não fique ponta-cabeça
        glTexCoord2f(0.0f, 1.0f); glVertex2f(0, 0);            
        glTexCoord2f(1.0f, 1.0f); glVertex2f(janelaW, 0);     
        glTexCoord2f(1.0f, 0.0f); glVertex2f(janelaW, janelaH); 
        glTexCoord2f(0.0f, 0.0f); glVertex2f(0, janelaH);     
    glEnd();

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f); // Reseta a cor pra não atrapalhar o resto

    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}


// Desenha o overlay de cura na tela
void drawHealthOverlay()
{
    if (healthAlpha <= 0.0f) return; // Se alpha é 0, não desenha

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, janelaW, 0, janelaH);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // --- USA A TEXTURA E O ALPHA DE CURA ---
    glBindTexture(GL_TEXTURE_2D, texHealthOverlay);
    glColor4f(1.0f, 1.0f, 1.0f, healthAlpha); 
    // ---------------------------------------

    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(0, 0);            
        glTexCoord2f(1.0f, 1.0f); glVertex2f(janelaW, 0);     
        glTexCoord2f(1.0f, 0.0f); glVertex2f(janelaW, janelaH); 
        glTexCoord2f(0.0f, 0.0f); glVertex2f(0, janelaH);     
    glEnd();

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f); // Reseta cor

    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

// --- GAME RENDER ---
void gameRender()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    float radYaw = yaw * 3.14159265f / 180.0f;
    float radPitch = pitch * 3.14159265f / 180.0f;

    float dirX = cosf(radPitch) * sinf(radYaw);
    float dirY = sinf(radPitch);
    float dirZ = -cosf(radPitch) * cosf(radYaw);

    gluLookAt(
        camX, camY, camZ,
        camX + dirX, camY + dirY, camZ + dirZ,
        0.0f, 1.0f, 0.0f);

    setSunDirectionEachFrame();

    // --- DESENHAR O CÉU ANTES DO MAPA ---
    drawSkydome(camX, camY, camZ);

    drawLevel(gLevel.map, camX, camZ, dirX, dirZ);
    
    // Desenha os sprites
    drawEntities(gLevel.enemies, gLevel.items, camX, camZ, dirX, dirZ);
    drawWeaponHUD();
    drawDamageOverlay();
    drawHealthOverlay();
    drawAmmoHUD();
    drawHealthHUD();

    glutSwapBuffers();
}