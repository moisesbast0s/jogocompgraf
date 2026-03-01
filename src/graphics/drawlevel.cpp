#include <GL/glew.h>
#include <GL/glut.h>
#include <cmath>
#include "core/game_state.h"
#include "graphics/drawlevel.h"
#include "level/levelmetrics.h"
#include "utils/utils.h"
#include <cstdio>

#include "graphics/FlashlightState.h"
#include "graphics/Fog.h"


#include <GL/glew.h>
#include <GL/glut.h>
#include <cmath>
#include "core/game_state.h"
#include "graphics/drawlevel.h"
#include "level/levelmetrics.h"
#include "utils/utils.h"
#include <cstdio>

#include "graphics/FlashlightState.h"
#include "graphics/Fog.h"

// =====================
// CONFIG / CONSTANTES
// =====================

// Config do grid
static const float SUB = 1.0f;
static const float TILE = 4.0f;      // tamanho do tile no mundo (ajuste)
static const float CEILING_H = 4.0f; // altura do teto
static const float WALL_H = 4.0f;    // altura da parede
static const float EPS_Y = 0.001f;   // evita z-fighting

static const GLfloat kAmbientOutdoor[] = {0.45f, 0.30f, 0.25f, 1.0f}; // quente (seu atual)
static const GLfloat kAmbientIndoor[] = {0.12f, 0.12f, 0.18f, 1.0f};  // frio/azulado

// ======================
// CONFIG ÚNICA DO CULLING (XZ)
// ======================
static float gCullHFovDeg = 170.0f;     // FOV horizontal do culling (cenário + entidades)
static float gCullNearTiles = 2.0f;     // dentro disso não faz culling angular
static float gCullMaxDistTiles = 20.0f; // 0 = sem limite; em tiles

// Retorna TRUE se deve renderizar o objeto no plano XZ (distância + cone de FOV)
static inline bool isVisibleXZ(float objX, float objZ,
                               float camX, float camZ,
                               bool hasFwd, float fwdx, float fwdz)
{
    float vx = objX - camX;
    float vz = objZ - camZ;
    float distSq = vx * vx + vz * vz;

    if (gCullMaxDistTiles > 0.0f)
    {
        float maxDist = gCullMaxDistTiles * TILE;
        float maxDistSq = maxDist * maxDist;
        if (distSq > maxDistSq)
            return false;
    }

    float nearDist = gCullNearTiles * TILE;
    float nearDistSq = nearDist * nearDist;
    if (distSq <= nearDistSq)
        return true;

    if (!hasFwd)
        return true;

    float cosHalf = std::cos(deg2rad(gCullHFovDeg * 0.5f));
    float invDist = 1.0f / std::sqrt(distSq);
    float nvx = vx * invDist;
    float nvz = vz * invDist;

    float dot = clampf(nvx * fwdx + nvz * fwdz, -1.0f, 1.0f);
    return dot >= cosHalf;
}

static void bindTexture0(GLuint tex)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
}

static float hash01(float x)
{
    float s = sinf(x * 12.9898f) * 43758.5453f;
    return s - floorf(s);
}

static float flickerFluorescente(float t)
{
    const float rate = 4.0f;
    float block = floorf(t * rate);
    float r = hash01(block);

    if (r < 0.22f)
    {
        float phase = t * rate - block;

        if (phase > 0.35f && phase < 0.55f)
            return 0.12f;

        if (r < 0.06f && phase > 0.65f && phase < 0.78f)
            return 0.40f;
    }

    return 0.96f + 0.04f * sinf(t * 5.0f);
}

static void setIndoorLampAt(float x, float z, float intensity)
{
    GLfloat pos[] = {x, CEILING_H - 0.05f, z, 1.0f};
    glLightfv(GL_LIGHT1, GL_POSITION, pos);

    GLfloat diff[] = {
        1.20f * intensity,
        1.22f * intensity,
        1.28f * intensity,
        1.0f};
    glLightfv(GL_LIGHT1, GL_DIFFUSE, diff);

    GLfloat amb[] = {
        1.10f * intensity,
        1.10f * intensity,
        1.12f * intensity,
        1.0f};
    glLightfv(GL_LIGHT1, GL_AMBIENT, amb);
}

static void beginIndoor(float wx, float wz, float time)
{
    glDisable(GL_LIGHT0);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, kAmbientIndoor);

    glEnable(GL_LIGHT1);

    float f = flickerFluorescente(time);
    float intensity = 1.2f * f;

    setIndoorLampAt(wx, wz, intensity);
}

static void endIndoor()
{
    glDisable(GL_LIGHT1);

    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, kAmbientOutdoor);
    glEnable(GL_LIGHT0);
}

static void desenhaQuadTeto(float x, float z, float tile, float tilesUV) {
    float half = tile * 0.5f;
    float uvStep = tilesUV / (tile / SUB);
    glNormal3f(0.0f, -1.0f, 0.0f);
    
    for (float fz = -half; fz < half; fz += SUB) {
        for (float fx = -half; fx < half; fx += SUB) {
            float u = ((fx + half) / tile) * tilesUV;
            float v = ((fz + half) / tile) * tilesUV;
            glBegin(GL_QUADS);
            glTexCoord2f(u, v);                   glVertex3f(x + fx, CEILING_H, z + fz);
            glTexCoord2f(u + uvStep, v);          glVertex3f(x + fx + SUB, CEILING_H, z + fz);
            glTexCoord2f(u + uvStep, v + uvStep); glVertex3f(x + fx + SUB, CEILING_H, z + fz + SUB);
            glTexCoord2f(u, v + uvStep);          glVertex3f(x + fx, CEILING_H, z + fz + SUB);
            glEnd();
        }
    }
}

static void desenhaQuadChao(float x, float z, float tile, float tilesUV) {
    float half = tile * 0.5f;
    float uvStep = tilesUV / (tile / SUB);
    glNormal3f(0.0f, 1.0f, 0.0f);

    for (float fz = -half; fz < half; fz += SUB) {
        for (float fx = -half; fx < half; fx += SUB) {
            float u = ((fx + half) / tile) * tilesUV;
            float v = ((fz + half) / tile) * tilesUV;
            glBegin(GL_QUADS);
            glTexCoord2f(u, v + uvStep);          glVertex3f(x + fx, EPS_Y, z + fz + SUB);
            glTexCoord2f(u + uvStep, v + uvStep); glVertex3f(x + fx + SUB, EPS_Y, z + fz + SUB);
            glTexCoord2f(u + uvStep, v);          glVertex3f(x + fx + SUB, EPS_Y, z + fz);
            glTexCoord2f(u, v);                   glVertex3f(x + fx, EPS_Y, z + fz);
            glEnd();
        }
    }
}

static void desenhaTileChao(float x, float z, GLuint texChaoX, bool temTeto, GLuint texTetoX = 0)
{
    glUseProgram(0);
    glColor3f(1, 1, 1);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texChaoX);

    desenhaQuadChao(x, z, TILE, 2.0f);

    if (temTeto)
    {
        glBindTexture(GL_TEXTURE_2D, texTetoX ? texTetoX : texChaoX);
        desenhaQuadTeto(x, z, TILE, 2.0f);
    }
}

// --- Desenha parede FACE POR FACE ---
static void desenhaParedePorFace(float x, float z, GLuint texParedeX, int f) {
    float half = TILE * 0.5f;
    glUseProgram(0);
    glColor3f(1, 1, 1);
    glBindTexture(GL_TEXTURE_2D, texParedeX);
    
    float tilesX = 1.0f; float tilesY = 2.0f;
    float stepX = SUB; // O tamanho do recorte (ex: 1.0f)
    float stepY = SUB;

    // Define a normal apenas uma vez por face para otimizar
    float nx = 0, ny = 0, nz = 0;
    if(f==0) nz=1; else if(f==1) nz=-1; else if(f==2) nx=1; else if(f==3) nx=-1;
    glNormal3f(nx, ny, nz);

    for (float h = 0; h < WALL_H; h += stepY) {
        for (float w = -half; w < half; w += stepX) {
            // Cálculo dinâmico de UV para a textura não ficar repetida estranhamente
            float u = ((w + half) / TILE) * tilesX;
            float v = (h / WALL_H) * tilesY;
            float uNext = u + (stepX / TILE) * tilesX;
            float vNext = v + (stepY / WALL_H) * tilesY;

            glBegin(GL_QUADS);
            switch (f) {
                case 0: // Face Z+
                    glTexCoord2f(uNext, vNext);  glVertex3f(x + w, h, z + half);
                    glTexCoord2f(u, vNext);      glVertex3f(x + w + stepX, h, z + half);
                    glTexCoord2f(u, v);          glVertex3f(x + w + stepX, h + stepY, z + half);
                    glTexCoord2f(uNext, v);      glVertex3f(x + w, h + stepY, z + half);
                    break;
                case 1: // Face Z-
                    glTexCoord2f(uNext, vNext);  glVertex3f(x + w + stepX, h, z - half);
                    glTexCoord2f(u, vNext);      glVertex3f(x + w, h, z - half);
                    glTexCoord2f(u, v);          glVertex3f(x + w, h + stepY, z - half);
                    glTexCoord2f(uNext, v);      glVertex3f(x + w + stepX, h + stepY, z - half);
                    break;
                case 2: // Face X+
                    glTexCoord2f(uNext, vNext);  glVertex3f(x + half, h, z - w);
                    glTexCoord2f(u, vNext);      glVertex3f(x + half, h, z - (w + stepX));
                    glTexCoord2f(u, v);          glVertex3f(x + half, h + stepY, z - (w + stepX));
                    glTexCoord2f(uNext, v);      glVertex3f(x + half, h + stepY, z - w);
                    break;
                case 3: // Face X-
                    glTexCoord2f(uNext, vNext);  glVertex3f(x - half, h, z + w);
                    glTexCoord2f(u, vNext);      glVertex3f(x - half, h, z + w + stepX);
                    glTexCoord2f(u, v);          glVertex3f(x - half, h + stepY, z + w + stepX);
                    glTexCoord2f(uNext, v);      glVertex3f(x - half, h + stepY, z + w);
                    break;
            }
            glEnd();
        }
    }
}

// Wrapper para desenhar o cubo todo (parede outdoor)
static void desenhaParedeCuboCompleto(float x, float z, GLuint texParedeX)
{
    desenhaParedePorFace(x, z, texParedeX, 0);
    desenhaParedePorFace(x, z, texParedeX, 1);
    desenhaParedePorFace(x, z, texParedeX, 2);
    desenhaParedePorFace(x, z, texParedeX, 3);

    float half = TILE * 0.5f;
    glBindTexture(GL_TEXTURE_2D, texParedeX);
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(x - half, WALL_H, z + half);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(x + half, WALL_H, z + half);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(x + half, WALL_H, z - half);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(x - half, WALL_H, z - half);
    glEnd();
}

static void desenhaTileLava(float x, float z, const RenderAssets &r, float time)
{
    glUseProgram(r.progLava);

    GLint locTime = glGetUniformLocation(r.progLava, "uTime");
    GLint locStr = glGetUniformLocation(r.progLava, "uStrength");
    GLint locScr = glGetUniformLocation(r.progLava, "uScroll");
    GLint locHeat = glGetUniformLocation(r.progLava, "uHeat");
    GLint locTex = glGetUniformLocation(r.progLava, "uTexture");

    glUniform1f(locTime, time);
    glUniform1f(locStr, 1.0f);
    glUniform2f(locScr, 0.1f, 0.0f);
    glUniform1f(locHeat, 0.6f);

    bindTexture0(r.texLava);
    glUniform1i(locTex, 0);

    glColor3f(1, 1, 1);
    desenhaQuadChao(x, z, TILE, 2.0f);

    glUseProgram(0);
}

static void desenhaTileAgua(float x, float z, const RenderAssets &r)
{
    glUseProgram(0); // fixed pipeline
    glColor3f(1,1,1);
    glBindTexture(GL_TEXTURE_2D, r.texAguaEsgoto);
    desenhaQuadChao(x, z, TILE, 2.0f);
}

static void desenhaTileSangue(float x, float z, const RenderAssets &r, float time)
{
    glUseProgram(r.progSangue);

    GLint locTime = glGetUniformLocation(r.progSangue, "uTime");
    GLint locStr = glGetUniformLocation(r.progSangue, "uStrength");
    GLint locSpd = glGetUniformLocation(r.progSangue, "uSpeed");
    GLint locTex = glGetUniformLocation(r.progSangue, "uTexture");

    glUniform1f(locTime, time);
    glUniform1f(locStr, 1.0f);
    glUniform2f(locSpd, 2.0f, 1.3f);

    bindTexture0(r.texSangue);
    glUniform1i(locTex, 0);

    glColor3f(1, 1, 1);
    desenhaQuadChao(x, z, TILE, 2.0f);

    glUseProgram(0);
}

// --- Checa vizinhos ---
static char getTileAt(const MapLoader &map, int tx, int tz)
{
    const auto &data = map.data();
    const int H = map.getHeight();

    if (tz < 0 || tz >= H)
        return '0';
    if (tx < 0 || tx >= (int)data[tz].size())
        return '0';

    return data[tz][tx];
}

static void drawFace(float wx, float wz, int face, char neighbor, GLuint texParedeInternaX, float time)
{
    bool outside = (neighbor == '0' || neighbor == 'L' || neighbor == 'B');

    if (outside)
    {
        glDisable(GL_LIGHT1);
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, kAmbientOutdoor);
        glEnable(GL_LIGHT0);

        desenhaParedePorFace(wx, wz, texParedeInternaX, face);
    }
    else if (neighbor != '2')
    {
        beginIndoor(wx, wz, time);
        desenhaParedePorFace(wx, wz, texParedeInternaX, face);
        endIndoor();
    }
}

//flashlight

static void setupFlashlight(float px, float py, float pz, float dx, float dy, float dz) {
    glEnable(GL_LIGHT2);

    // Posição da luz (mesma da câmera/jogador)
    // O 1.0f no final indica que é uma Positional Light (importante para spotlight)
    GLfloat lightPos[] = { px, py, pz, 1.0f };
    glLightfv(GL_LIGHT2, GL_POSITION, lightPos);

    // Direção da luz (para onde o jogador está olhando)
    GLfloat lightDir[] = { dx, dy, dz };
    glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, lightDir);

    // Configurações de intensidade e cor
    GLfloat white[] = { 1.0f, 1.0f, 0.9f, 1.0f }; // levemente amarelada
    glLightfv(GL_LIGHT2, GL_DIFFUSE, white);
    glLightfv(GL_LIGHT2, GL_SPECULAR, white);

    // Configurações do cone de luz
    glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, 25.0f);     // Ângulo do cone (ex: 25 graus)
    glLightf(GL_LIGHT2, GL_SPOT_EXPONENT, 20.0f);   // Suavidade das bordas (0-128)

    // Atenuação (faz a luz perder força com a distância)
    glLightf(GL_LIGHT2, GL_CONSTANT_ATTENUATION, 1.0f);
    glLightf(GL_LIGHT2, GL_LINEAR_ATTENUATION, 0.05f);
    glLightf(GL_LIGHT2, GL_QUADRATIC_ATTENUATION, 0.01f);
}

// 1. ADICIONE A FUNÇÃO AUXILIAR ANTES DA DRAWLEVEL
static void setupFlashlight(float px, float pz, float dx, float dz) {
    glEnable(GL_LIGHTING); 
    glEnable(GL_LIGHT2);

    // Posição e Direção
    GLfloat lightPos[] = { px, 2.0f, pz, 1.0f };
    GLfloat lightDir[] = { dx, -0.1f, dz }; // Leve inclinação para baixo (-0.1f) ajuda a ver o chão
    
    glLightfv(GL_LIGHT2, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, lightDir);

    // INTENSIDADE: Aumentamos os valores para 2.0f ou mais para "estourar" o brilho no centro
    // brilho variável dependendo da % da bateria
    //GLfloat brightWhite[] = { 2.5f, 2.5f, 2.2f, 1.0f }; 
    
    float intensity = flashlightOn ? 1.0f : 0.0f; // se a lanterna estiver desligada, a intensidade é 0 (sem luz)

    GLfloat brightWhite[] = { 2.5f * intensity, 2.5f * intensity, 2.2f * intensity, 1.0f };
    glLightfv(GL_LIGHT2, GL_DIFFUSE, brightWhite);
    glLightfv(GL_LIGHT2, GL_SPECULAR, brightWhite);
    
    GLfloat ambientNone[] = { 0.0f, 0.0f, 0.0f, 1.0f }; // Lanterna não clareia o resto do mundo

    glLightfv(GL_LIGHT2, GL_DIFFUSE, brightWhite);
    glLightfv(GL_LIGHT2, GL_SPECULAR, brightWhite);
    glLightfv(GL_LIGHT2, GL_AMBIENT, ambientNone);

    // CONE: 20 graus deixa o foco mais fechado e claustrofóbico
    glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, 20.0f);     
    glLightf(GL_LIGHT2, GL_SPOT_EXPONENT, 30.0f);   // Borda mais nítida

    // ATENUAÇÃO: Valores menores fazem a luz ir mais longe
    glLightf(GL_LIGHT2, GL_CONSTANT_ATTENUATION, 1.0f);
    glLightf(GL_LIGHT2, GL_LINEAR_ATTENUATION, 0.02f);    // Antes era 0.05
    glLightf(GL_LIGHT2, GL_QUADRATIC_ATTENUATION, 0.005f); // Antes era 0.01
}

// atualizar a % da bateria
void updateFlashlight(float dt) { // dt = delta time em segundos
    if(flashlightOn) {
        flashlightBattery -= batteryDrainRate * dt;
        if (flashlightBattery <= 0.0f) {
            flashlightBattery = 0.0f;
            flashlightOn = false; //desliga a lanterna quando a bateria acabar
        }
    }
}


void drawLevel(const MapLoader &map, float px, float py, float pz, float dx, float dz, const RenderAssets &r, float time)
{
    // Inicializa as partículas de névoa uma única vez
    static bool fogInitialized = false;
    if (!fogInitialized) {
        // map, altura, tamanho, densidade
        initFogParticles(map, 0.01f, 3.0f, 30);
        fogInitialized = true;
    }

    // calcula delta time
    static float lastTime = 0.0f;
    float currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    float dt = currentTime - lastTime;
    lastTime = currentTime;

    // ATUALIZA BATERIA
    updateFlashlight(dt);

    // 1. CONFIGURAÇÃO DE ESCURIDÃO TOTAL
    glDisable(GL_LIGHT0); // Mata o Sol
    glEnable(GL_LIGHTING);
    
    // Zera a luz ambiente do mundo (nada se vê sem lanterna)
    GLfloat darkness[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, darkness);

    // 2. LIGA A LANTERNA
    setupFlashlight(px, pz, dx, dz);

    const auto &data = map.data();
    const int H = map.getHeight();
    LevelMetrics m = LevelMetrics::fromMap(map, TILE);
    float fwdx, fwdz;
    bool hasFwd = getForwardXZ(dx, dz, fwdx, fwdz);

    for (int z = 0; z < H; z++) {
        for (int x = 0; x < (int)data[z].size(); x++) {
            float wx, wz;
            m.tileCenter(x, z, wx, wz);

            if (!isVisibleXZ(wx, wz, px, pz, hasFwd, fwdx, fwdz)) continue;

            char c = data[z][x];
            bool isEnemy = (c == 'J' || c == 'T' || c == 'C' || c == 'K' ||
                            c == 'G' || c == 'E' || c == 'F' || c == 'I');
            bool isItem = (c == 'B' || c == 'M' || c == 'H' || c == 'A');

            if (isEnemy) {
                desenhaTileChao(wx, wz, r.texChao, true, r.texTeto);
            }
            else if (isItem) {
            
                desenhaTileChao(wx, wz, r.texChao, true, r.texTeto);
            }
            else if (c == '0') {
                desenhaTileChao(wx, wz, r.texChao, true, r.texTeto);
            }
            else if (c == '3') {
                desenhaTileChao(wx, wz, r.texChaoInterno, true, r.texTeto);
            }
            else if (c == '4') {
                desenhaTileAgua(wx, wz, r);
            }
            else if (c == '1') {
                desenhaParedeCuboCompleto(wx, wz, r.texParede);
            }
            else if (c == '2') {
                char vizFrente = getTileAt(map, x, z + 1);
                char vizTras = getTileAt(map, x, z - 1);
                char vizDireita = getTileAt(map, x + 1, z);
                char vizEsq = getTileAt(map, x - 1, z);
                drawFace(wx, wz, 0, vizFrente, r.texParedeInterna, time);
                drawFace(wx, wz, 1, vizTras, r.texParedeInterna, time);
                drawFace(wx, wz, 2, vizDireita, r.texParedeInterna, time);
                drawFace(wx, wz, 3, vizEsq, r.texParedeInterna, time);
            }
            else if (c == 'P') {
                desenhaTileChao(wx, wz, r.texChao, true, r.texTeto);
                glDisable(GL_LIGHTING);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glColor4f(0.0f, 0.6f, 1.0f, 0.5f);
                float half = TILE * 0.5f;
                glBegin(GL_QUADS);
                glVertex3f(wx-half, EPS_Y+0.01f, wz+half); glVertex3f(wx+half, EPS_Y+0.01f, wz+half);
                glVertex3f(wx+half, EPS_Y+0.01f, wz-half); glVertex3f(wx-half, EPS_Y+0.01f, wz-half);
                glEnd();
                glDisable(GL_BLEND);
                glEnable(GL_LIGHTING); 
            }
        }
    }

    // DESENHA A NÉVOA NO FINAL DO CENÁRIO
    // Passamos o dt (delta time) para animar as partículas
    drawFog(px, py, pz, r, dt);
}


static void drawSprite(float x, float z, float w, float h, GLuint tex, float camX, float camZ, float yOffset = 0.0f)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.1f);

    glBindTexture(GL_TEXTURE_2D, tex);
    glColor3f(1, 1, 1);

    glPushMatrix();

    glTranslatef(x, yOffset, z);

    float ddx = camX - x;
    float ddz = camZ - z;
    float angle = std::atan2(ddx, ddz) * 180.0f / 3.14159f;

    glRotatef(angle, 0.0f, 1.0f, 0.0f);

    float hw = w * 0.5f;

    glBegin(GL_QUADS);
    glNormal3f(0, 0, 1);

    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(-hw, 0.0f, 0.0f);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(hw, 0.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(hw, h, 0.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(-hw, h, 0.0f);
    glEnd();

    glPopMatrix();

    glDisable(GL_ALPHA_TEST);
    glDisable(GL_BLEND);
}

// Desenha inimigos e itens
void drawEntities(const std::vector<Enemy> &enemies, const std::vector<Item> &items,
                  float camX, float camZ, float dx, float dz, const RenderAssets &r)
{
    glDisable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.1f);

    float fwdx, fwdz;
    bool hasFwd = getForwardXZ(dx, dz, fwdx, fwdz);

    // --- ITENS ---
    for (const auto &item : items)
    {
        if (!item.active)
            continue;

        if (!isVisibleXZ(item.x, item.z, camX, camZ, hasFwd, fwdx, fwdz))
            continue;

        if (item.type == ITEM_HEALTH)
            drawSprite(item.x, item.z, 0.7f, 0.7f, r.texHealth, camX, camZ);
        else if (item.type == ITEM_AMMO)
            drawSprite(item.x, item.z, 0.7f, 0.7f, r.texAmmo, camX, camZ);
        else if (item.type == ITEM_BATTERY)
            drawSprite(item.x, item.z, 0.7f, 0.7f, r.texBattery, camX, camZ);
        else if (item.type == ITEM_PISTOL_AMMO)
            drawSprite(item.x, item.z, 0.7f, 0.7f, r.texPistolAmmo, camX, camZ);
    }

    // --- INIMIGOS ---
    for (const auto &en : enemies)
    {
        if (en.state == STATE_DEAD)
        {
            if (r.texEnemiesDead != 0 &&
                isVisibleXZ(en.x, en.z, camX, camZ, hasFwd, fwdx, fwdz))
            {

                drawSprite(en.x, en.z, 2.5f, 2.5f, r.texEnemiesDead, camX, camZ, -0.7f);
            }
            continue;
        }

        if (!isVisibleXZ(en.x, en.z, camX, camZ, hasFwd, fwdx, fwdz))
            continue;

        int t = (en.type < 0 || en.type > 4) ? 0 : en.type;

        GLuint currentTex;
        if (en.hurtTimer > 0.0f)
            currentTex = r.texEnemiesDamage[t];
        else if (en.state == STATE_CHASE || en.state == STATE_ATTACK)
            currentTex = r.texEnemiesRage[t];
        else
            currentTex = r.texEnemies[t];

        drawSprite(en.x, en.z, 2.5f, 2.5f, currentTex, camX, camZ, -0.2f);
    }

    glEnable(GL_LIGHTING);
    glDisable(GL_ALPHA_TEST);
}
