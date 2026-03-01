#include <GL/glew.h>
#include <GL/glut.h>
#include "graphics/Fog.h"
#include "level/maploader.h"
#include "level/levelmetrics.h"
#include "core/game_state.h"
#include <cmath>
#include <random>
#include <algorithm>

#ifndef TILE
#define TILE 4.0f
#endif

std::vector<FogParticle> fogParticles;
static std::default_random_engine generator;

void initFogParticles(const MapLoader &map, float fogHeight, float particleSize, int density) {
    fogParticles.clear();

    const auto &mapData = map.data();
    const int H = map.getHeight();
    LevelMetrics m = LevelMetrics::fromMap(map, TILE);

    std::uniform_real_distribution<float> pos_dist(-TILE * 0.45f, TILE * 0.45f);
    std::uniform_real_distribution<float> size_dist(particleSize * 0.8f, particleSize * 1.5f);
    std::uniform_real_distribution<float> alpha_dist(0.15f, 0.35f);
    std::uniform_real_distribution<float> life_dist(0.0f, 1.0f);
    std::uniform_real_distribution<float> speed_dist(0.1f, 0.25f);
    std::uniform_real_distribution<float> vel_dist(0.07f, 0.15f);
    std::uniform_real_distribution<float> drift_dist(-1.0f, 1.0f); // deriva lateral [-1, 1]
    std::uniform_int_distribution<int>   chance_dist(0, 100);

    const int PROB_FOG = 60; // 60% Fog / 40% Smoke

    for (int z = 0; z < H; ++z) {
        if (z >= (int)mapData.size()) continue;
        const std::string& line = mapData[z];

        for (int x = 0; x < (int)line.size(); x++) {
            char c = line[x];
            bool isWalkable = (c == '0' || c == '3' || c == 'P' || c == 'J' || c == 'T' ||
                               c == 'M' || c == 'K' || c == 'G' || c == 'H' || c == 'A' ||
                               c == 'E' || c == 'F' || c == 'I' || c == '4');

            if (isWalkable) {
                float wx, wz;
                m.tileCenter(x, z, wx, wz);

                for (int i = 0; i < density; ++i) {
                    FogParticle p;

                    // Posição inicial (origem fixa para o ciclo de drift)
                    p.startX = wx + pos_dist(generator);
                    p.startY = fogHeight;
                    p.startZ = wz + pos_dist(generator);

                    p.x = p.startX;
                    p.y = p.startY;
                    p.z = p.startZ;

                    p.life  = life_dist(generator);
                    p.speed = speed_dist(generator);
                    p.velY  = vel_dist(generator);

                    // Direção lateral aleatória por partícula
                    // velX e velZ formam um vetor 2D no plano XZ; cada componente em [-1, 1]
                    p.velX = drift_dist(generator);
                    p.velZ = drift_dist(generator);

                    if (chance_dist(generator) < PROB_FOG) {
                        p.textureID = 0; // texFog
                        p.size  = size_dist(generator);
                        p.alpha = alpha_dist(generator);
                    } else {
                        p.textureID = 1; // texSmoke
                        p.size  = size_dist(generator) * 1.5f;
                        p.alpha = alpha_dist(generator) * 1.2f;
                    }

                    p.currentAlpha = 0.0f;
                    fogParticles.push_back(p);
                }
            }
        }
    }
}

void drawFog(float camX, float camY, float camZ, const RenderAssets &r, float dt) {
    if (fogParticles.empty()) return;

    // Altura máxima que a partícula sobe acima de startY ao longo do ciclo completo.
    const float FOG_RISE_HEIGHT = 0.3f;

    // Raio máximo de deriva lateral (em unidades de mundo).
    // Cada partícula usa seu próprio velX/velZ normalizado contra esse raio,
    // portanto a deriva real varia aleatoriamente entre partículas.
    const float FOG_DRIFT_RADIUS = 1.7f;

    glPushAttrib(GL_LIGHTING_BIT | GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_TEXTURE_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glEnable(GL_TEXTURE_2D);
    glUseProgram(0);
    glEnable(GL_LIGHTING);

    GLfloat fog_amb[]  = {0.25f, 0.25f, 0.25f, 1.0f};
    GLfloat fog_diff[] = {0.9f,  0.9f,  0.9f,  1.0f};
    GLfloat fog_emi[]  = {0.1f,  0.1f,  0.1f,  1.0f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,  fog_amb);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,  fog_diff);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, fog_emi);

    float modelview[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, modelview);

    float rightX = modelview[0], rightY = modelview[4], rightZ = modelview[8];
    float upX    = modelview[1], upY    = modelview[5], upZ    = modelview[9];
    float normX  = -modelview[2], normY = -modelview[6], normZ = -modelview[10];

    GLuint lastBoundTex = 0xFFFFFFFF;

    for (auto &p : fogParticles) {
        // Avança ciclo de vida [0, 1)
        p.life += p.speed * dt;
        if (p.life >= 1.0f) {
            p.life = 0.0f;
        }

        // Posição vertical: sobe linearmente de startY até startY + FOG_RISE_HEIGHT
        p.y = p.startY + (p.life * FOG_RISE_HEIGHT);

        // Posição lateral: deriva a partir da origem em direção aleatória (velX, velZ).
        // O deslocamento aumenta conforme life avança, imitando o arrasto do vento.
        // Ao resetar life=0 a partícula volta suavemente à origem no próximo ciclo.
        p.x = p.startX + p.velX * (p.life * FOG_DRIFT_RADIUS);
        p.z = p.startZ + p.velZ * (p.life * FOG_DRIFT_RADIUS);

        // Envelope de alpha:
        // - sin(life*π) cria fade-in/fade-out simétrico ao longo do ciclo
        // - (1 - life)^0.6 penaliza o topo, mantendo o fog denso embaixo e rarefeito em cima
        float envelope   = sinf(p.life * 3.14159f);
        float heightFade = powf(1.0f - p.life, 0.6f);
        p.currentAlpha   = p.alpha * envelope * heightFade;

        if (p.currentAlpha < 0.005f) continue;

        // Culling por distância
        float dx = p.x - camX;
        float dy = p.y - camY;
        float dz = p.z - camZ;
        if (dx*dx + dy*dy + dz*dz > 1600.0f) continue;

        GLuint currentTex = (p.textureID == 0) ? r.texFog : r.texSmoke;
        if (currentTex == 0) continue;

        if (currentTex != lastBoundTex) {
            glBindTexture(GL_TEXTURE_2D, currentTex);
            lastBoundTex = currentTex;
        }

        if (p.textureID == 1) {
            GLfloat smoke_emi[] = {0.2f, 0.2f, 0.2f, 1.0f};
            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, smoke_emi);
        } else {
            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, fog_emi);
        }

        glColor4f(1.0f, 1.0f, 1.0f, p.currentAlpha);
        glPushMatrix();

        float h = p.size * 0.5f;
        glBegin(GL_QUADS);
            glNormal3f(normX, normY, normZ);

            glTexCoord2f(0, 0);
            glVertex3f(p.x + (-rightX - upX) * h, p.y + (-rightY - upY) * h, p.z + (-rightZ - upZ) * h);

            glTexCoord2f(1, 0);
            glVertex3f(p.x + ( rightX - upX) * h, p.y + ( rightY - upY) * h, p.z + ( rightZ - upZ) * h);

            glTexCoord2f(1, 1);
            glVertex3f(p.x + ( rightX + upX) * h, p.y + ( rightY + upY) * h, p.z + ( rightZ + upZ) * h);

            glTexCoord2f(0, 1);
            glVertex3f(p.x + (-rightX + upX) * h, p.y + (-rightY + upY) * h, p.z + (-rightZ + upZ) * h);
        glEnd();

        glPopMatrix();
    }

    glPopAttrib();
}