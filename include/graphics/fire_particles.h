#pragma once

#include <vector>
#include <cstdlib>

// Tipo correto usado pelo menu
struct FireParticle {
    float x, y;
    float velY;
    float life;
    float size;
    float r, g, b;
};

struct FireSystem {
    std::vector<FireParticle> particles;
};

// atualiza (spawna + move + remove)
void fireUpdate(FireSystem& fs, int screenW, int screenH);

// desenha (assume que o caller já está em 2D)
void fireRender(const FireSystem& fs);

