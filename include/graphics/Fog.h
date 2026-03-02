#ifndef FOG_H
#define FOG_H

#include <GL/glew.h>
#include <GL/glut.h>
#include <vector>

class MapLoader;
struct RenderAssets;

struct FogParticle {
    float x, y, z;                  // Posição
    float startX, startY, startZ;   // Altura de nascimento (para o loop)
    float size;                     // Tamanho
    float alpha;                    // Transparência base
    float currentAlpha;             // Transparência atual (animada)
    float velX, velY, velZ;         // Velocidade
    float life;                     // Vida (0.0 a 1.0)
    float speed;                    // Quão rápido a vida acaba
    GLuint textureID;               // ID da textura (texFog ou texSmoke)
};

extern std::vector<FogParticle> fogParticles;

void initFogParticles(const MapLoader &map, float fogHeight, float particleSize, int density);
void drawFog(float camX, float camY, float camZ, const RenderAssets &r, float dt);

#endif // FOG_H